#include "texel.h"



texel::tuning_positions* texel::load_file(std::string filepath) {
	tuning_positions* positions = new tuning_positions();

	std::ifstream epd_file(filepath);

	std::string epd;

	double result = 0.0;
	std::string pos;

	int count = 0;

	// Now loop through all epd's
	while (std::getline(epd_file, epd)) {
		texel_pos this_position;

		// Copy the FEN in the line.
		pos = "";

		auto fen_end = epd.find_first_of("-");

		if (epd[fen_end + 2] == '-') {
			fen_end = fen_end + 2;
		}

		pos = epd.substr(0, fen_end + 1);


		// Now resolve the game result
		try {
			auto result_start = epd.find_first_of('"');

			std::string res = epd.substr(result_start + 1, epd.length() - result_start - 3);

			if (res == "1/2-1/2") {
				result = 0.5;
			}
			else if (res == "1-0") {
				result = 1.0;
			}
			else if (res == "0-1") {
				result = 0.0;
			}
			else {
				throw 1;
			}
			this_position.game_result = result;
		}
		catch (int e) {
			std::cerr << "Exception occurred. Exception nr. " << e << ": No valid game result was loaded from the .epd file." << std::endl;
		}


		this_position.fen = pos;


		positions->positions.push_back(this_position);
		count++;

	}

	epd_file.close();
	std::cout << "Loaded " << count << " positions from: " << filepath << std::endl;

	return positions;
}


double texel::eval_error(tuning_positions* EPDS, double k) {
	//S_BOARD* pos = new S_BOARD;
	
	double error = 0.0;

	//std::chrono::time_point<std::chrono::high_resolution_clock> m_Starttime = std::chrono::high_resolution_clock::now();

	// Loop through all the EPD positions
#pragma omp parallel shared(error)
	{
		S_BOARD* pos = new S_BOARD;
#pragma omp for schedule(static, EPDS->positions.size() / partitions) reduction(+:error)
		for (int p = 0; p < EPDS->positions.size(); p++) {

			double game_result = EPDS->positions[p].game_result;

			// Parse the FEN
			char* fen = EPDS->positions[p].getFen();

			BoardRep::parseFen(fen, *pos, false);

			delete[] fen;

			// Get the evaluation and make it relative to white
			int evaluation = eval::staticEval(pos, 0, 0);
			evaluation *= (pos->whitesMove == WHITE) ? 1 : -1;


			// Calculate the squared error of this single evaluation, and add it the the error
			error += pow((game_result - sigmoid(double(evaluation), k)), 2);
		}
		delete pos;
	}
	// Take the average error:
	error /= double(EPDS->positions.size());

	return error;
}


double texel::find_k(tuning_positions* EPDS) {

	std::cout << "Computing best value for K" << std::endl;
	std::cout << "Initial value: " << k_initial << std::endl;

	double k_best = k_initial;
	double error_best = eval_error(EPDS, k_best);

	std::cout << "Initial error: " << error_best << std::endl;

	for (int i = 0; i < k_precision; i++) {
		std::cout << "Iteration " << (i + 1) << ": " << std::endl;

		double unit = pow(10.0, -i);  // We'll start by searching from the initial k in +-10 and then narrow down the range afterwards
		double range = 10.0 * unit;

		double k_max = k_best + range;

		for (double k = std::max(k_best - range, 0.0); k <= k_max; k += unit) {

			double error = eval_error(EPDS, k);

			std::cout << "Computed error of K = " << k << " to be E = " << error << std::endl;

			if (error < error_best) {
				k_best = k;
				error_best = error;
			}
		}

		std::cout << "[+] Best k-value of iteration: " << (i+1) << ": K = " << k_best  << " with error E = " << error_best << std::endl;

	}

	std::cout << "Optimal value of K = " << k_best << std::endl;

	k_initial = k_best;

	return k_best;
}


double texel::changed_eval_error(std::vector<int*> params, std::vector<int> new_values, tuning_positions* EPDS, double k) {

	// Firstly, change all the parameters to the desired values:
	for (int i = 0; i < params.size(); i++) {
		*params[i] = new_values[i];
	}

	// Now return the evaluation error.
	return eval_error(EPDS, k);
}


void texel::tune(std::vector<texel::Parameter> initial_guess, std::string epd_file, int runs) {
	// Set up the datapoints of the error and iteration number:
	ErrorData data;
	data.clear();

	std::string date_time = getDateTime();

	// Copy all arguments
	std::vector<texel::Parameter> parameters = initial_guess;
	std::vector<int*> param_ptrs;

	for (int i = 0; i < parameters.size(); i++) {
		param_ptrs.push_back(parameters[i].variable);
	}

	// Copy the initial values of the variables and make a vector in which we can change them
	std::vector<int> initial_values;
	std::vector<int> theta;

	for (int i = 0; i < parameters.size(); i++) {
		initial_values.push_back(*parameters[i].variable);
		theta.push_back(*parameters[i].variable);
	}

	std::string filepath = epd_file;
	int iterations = runs;

	// Now load the file containing all positions:
	tuning_positions* EPDS = load_file(filepath);

	// Find the optimal k
	double k = find_k(EPDS);
	

	// Now we'll loop through all the iterations:
	std::cout << "Starting Texel SPSA tuning session with " << parameters.size() << " parameters in " << iterations << " iterations" << std::endl;

	std::vector<int> theta_plus;
	std::vector<int> theta_minus;
	std::vector<double> delta;

	
	//
	//	Calculate SPSA parameters
	//

	double BIG_A = 0.1 * double(iterations);
	//double a = double(A_END) * pow(BIG_A + double(iterations), alpha);
	
	double c = double(C_END) * pow(double(iterations), gamma);
	double a_end = R_END * pow(c, 2.0);
	double a = a_end * pow(BIG_A + double(iterations), alpha);


	std::vector<int> step_size;
	step_size.push_back(0);

	std::vector<double> momentum;
	std::vector<double> velocity;


	std::vector<double> beta_n;
	std::vector<double> gamma_n;

	double g_hat = 0;

	for (int i = 0; i < theta.size(); i++) {

		momentum.push_back(0.0);
		velocity.push_back(0.0);
	}


	for (int n = 0; n < iterations; n++) {

		double error = changed_eval_error(param_ptrs, theta, EPDS, k);	

		data.push_back(DataPoint(error, n + 1, abs(g_hat), abs(step_size[n]), theta));


		// Calculate an and cn.
		double an = a / (pow(BIG_A + double(n) + 1.0, alpha));
		double cn = c / (pow(double(n) + 1.0, gamma));

		double bn = beta_0 / pow(double(n) + 1.0, lambda);
		double gn = gamma_0 / pow(double(n) + 1.0, lambda);

		beta_n.push_back(bn);
		gamma_n.push_back(gn);

		std::cout << "Iteration nr. " << (n + 1) << ": an = " << an << ", cn = " << cn << std::endl;
		std::cout << "Beta_n = " << bn << ", gamma_n = " << gn << std::endl;

		// Clear theta_plus, theta_minus and populate delta.
		theta_plus.clear();
		theta_minus.clear();
		delta.clear();

		for (int i = 0; i < parameters.size(); i++) {
			delta.push_back(randemacher());

			int tPlus_i = theta[i] + std::round(cn * delta[i]);
			int tMinus_i = theta[i] - std::round(cn * delta[i]);

			theta_plus.push_back(tPlus_i);
			theta_minus.push_back(tMinus_i);
		}


		// Now that we've done this, we can measure the error of theta_plus and theta_minus respectively:
		double tPlus_error = double(EPDS->positions.size()) * changed_eval_error(param_ptrs, theta_plus, EPDS, k);
		double tMinus_error = double(EPDS->positions.size()) * changed_eval_error(param_ptrs, theta_minus, EPDS, k);

		std::cout << "Theta plus error: " << tPlus_error << ", Theta minus error: " << tMinus_error << std::endl;
		
		//
		//Now we can calculate ghat for all parameters:
		//
		for (int i = 0; i < parameters.size(); i++) {
			g_hat = ((tPlus_error - tMinus_error)) / (2.0 * double(cn) * double(delta[i]));

			//momentum[i] = bn * momentum[i] + (1.0 - bn) * g_hat;
			//velocity[i] = gn * momentum[i] + (1.0 - gn) * pow(g_hat, 2);

			momentum[i] = bn * momentum[i] + (1.0 - bn) * g_hat;
			velocity[i] = gn * momentum[i] + (1.0 - gn) * pow(g_hat, 2);



			double m_hat = bn * momentum[i];
			double v_hat = gn * velocity[i];

			double beta_sum = 0.0;
			double gamma_sum = 0.0;

			for (int r = 0; r < n; r++) {

				double bs = (1.0 - beta_n[n - 1 - r]);
				double gs = (1.0 - gamma_n[n - 1 - r]);

				for (int j = 0; j < r; j++) {
					bs *= beta_n[n - j];
					gs *= gamma_n[n - j];
				}

				beta_sum += bs;
				gamma_sum += gs;

			}

			m_hat /= beta_sum;
			v_hat /= gamma_sum;


			if (n == 0) {
				m_hat = 0;
				v_hat = 1;
			}

			double step = std::round(double((an * m_hat) / (sqrt(abs(v_hat)) + epsilon)));

			step_size.push_back(step);

			theta[i] = theta[i] - step;
			theta[i] = std::max(std::min(theta[i], parameters[i].max_val), parameters[i].min_val);
		}

		// Display the new values to the user:
		std::cout << "Updated values:" << std::endl;

		for (int i = 0; i < theta.size(); i++) {
			std::cout << "[" << (i + 1) << "]: " << theta[i] << " (Original value: " << initial_values[i] << ")" << std::endl;
		}
		std::cout << "\n\n";

	}


	//
	//After all iterations, display the results.
	//

	std::cout << "Texel tuning with SPSA results for " << parameters.size() << " variables, after " << iterations << " iterations" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	for (int i = 0; i < theta.size(); i++) {
		std::cout << "[" << (i + 1) << "]: " << theta[i] << " (Original value: " << initial_values[i] << ")" << std::endl;
	}
	std::cout << "\n\n";
	
	std::string display = "";
	std::cout << "[*] Should the error by iteration points be written to a .csv file and/or be displayed on the console? ";
	std::cin >> display; 
	std::cout << "\n";

	if (display == "print" || display == "both") {
		std::cout << "\n";

		std::cout <<
			"	Iteration	|	Error	|";

		for (int i = 0; i < theta.size(); i++) {
			printf("	Var %d	|", (i + 1));
		}				 
		std::cout << "\n";

		std::cout << "---------------+-----------";
		for (int i = 0; i < theta.size(); i++) {
			std::cout << "----------+";
		}
		std::cout << "\n";


		for (int i = 0; i < data.size(); i++) {
			printf("	%d	|	%f	|", data[i].iteration, data[i].error_val);

			for (int v = 0; v < data[i].variables.size(); v++) {
				printf("	%d	|", data[i].variables[v]);
			}
			printf("\n");
		}
		std::cout << "---------------+-----------\n\n";
	}
	
	if (display == "file" || display == "both") {
		std::string filename = "texel-";
		filename += date_time;
		filename += ".csv";

		
		std::ofstream outFile(filename);

		outFile << "Iteration; ";
		outFile << data[0].iteration;
		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].iteration;
		}
		outFile << "\n";

		outFile << "Evaluation error;";
		outFile << data[0].error_val;
		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].error_val;
		}
		outFile << "\n";

		outFile << "Gradient;";
		outFile << data[0].gradient;
		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].gradient;
		}
		outFile << "\n";

		outFile << "Step size;";
		outFile << data[0].step;
		for (int i = 1; i < data.size(); i++) {
			outFile << ";" << data[i].step;
		}
		outFile << "\n";

		for (int n = 0; n < data[0].variables.size(); n++) {
			outFile << "Variable " << (n + 1) << ";";
			outFile << data[0].variables[n];

			for (int i = 1; i < data.size(); i++) {
				outFile << ";" << data[i].variables[n];
			}
			outFile << "\n";
		}
	}

	delete EPDS;
}