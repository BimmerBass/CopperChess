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
			//for (int p = 0; p < 5000; p++) {

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


void texel::tune(std::vector<int*> initial_guess, std::string epd_file, int runs) {
	// Copy all arguments
	std::vector<int*> parameters = initial_guess;

	// Copy the initial values of the variables and make a vector in which we can change them
	std::vector<int> initial_values;
	std::vector<int> theta;

	for (int i = 0; i < parameters.size(); i++) {
		initial_values.push_back(*parameters[i]);
		theta.push_back(*parameters[i]);
	}

	std::string filepath = epd_file;
	int iterations = runs;

	// Now load the file containing all positions:
	tuning_positions* EPDS = load_file(filepath);

	// Find the optimal k
	//double k = 0.0;
	double k = 1.987;
	//k = find_k(EPDS);

	// Now we'll loop through all the iterations:
	std::cout << "Starting Texel SPSA tuning session with " << parameters.size() << " parameters in " << iterations << " iterations" << std::endl;

	std::vector<int> theta_plus;
	std::vector<int> theta_minus;
	std::vector<int> delta;
	
	
	/*
		Calculate SPSA parameters
	*/

	double BIG_A = 0.1 * double(iterations);
	/*double a = 3 * double(A_END) * pow(BIG_A + double(iterations), alpha);
	double c = 3 * double(C_END) * pow(double(iterations), gamma);
	*/
	double c = 3 * double(C_END) * pow(double(iterations), gamma);
	double a_end = double(R_END) * pow(double(C_END), 2.0);

	double a = a_end * pow((BIG_A + double(iterations)), alpha);

	for (int n = 0; n < iterations; n++) {
		// Calculate an and cn.
		int an = int(a / (pow(BIG_A + double(n) + 1.0, alpha)));
		int cn = int(c / (pow(double(n) + 1.0, gamma)));

		std::cout << "Iteration nr. " << (n + 1) << ": an = " << an << ", cn = " << cn << std::endl;

		// Clear theta_plus, theta_minus and populate delta.
		theta_plus.clear();
		theta_minus.clear();
		delta.clear();


		for (int i = 0; i < parameters.size(); i++) {
			delta.push_back(randemacher());

			theta_plus.push_back(theta[i] + cn * delta[i]);
			theta_minus.push_back(theta[i] - cn * delta[i]);
		}


		// Now that we've done this, we can measure the error of theta_plus and theta_minus respectively:
		//double tPlus_error = double(EPDS->positions.size()) * changed_eval_error(parameters, theta_plus, EPDS, k);
		//double tMinus_error = double(EPDS->positions.size()) * changed_eval_error(parameters, theta_minus, EPDS, k);
		double tPlus_error = 1000.0 * changed_eval_error(parameters, theta_plus, EPDS, k);
		double tMinus_error = 1000.0 * changed_eval_error(parameters, theta_minus, EPDS, k);


		std::cout << "Theta plus error: " << tPlus_error << ", Theta minus error: " << tMinus_error << std::endl;
		
		/*
		Now we can calculate ghat for all parameters:
		*/
		double g_hat = 0;
		for (int i = 0; i < parameters.size(); i++) {
			g_hat = ((tPlus_error - tMinus_error)) / (2.0 * double(cn) * double(delta[i]));

			// Adjust the variable in theta using gradient descent:
			theta[i] = theta[i] - int(double(an) * g_hat);
		}

		// Display the new values to the user:
		std::cout << "Updated values:" << std::endl;

		for (int i = 0; i < theta.size(); i++) {
			std::cout << "[" << (i + 1) << "]: " << theta[i] << " (Original value: " << initial_values[i] << ")" << std::endl;
		}
		std::cout << "\n\n";

	}


	/*
	After all iterations, display the results.
	*/

	std::cout << "Texel tuning with SPSA results for " << parameters.size() << " variables, after " << iterations << " iterations" << std::endl;
	std::cout << "-------------------------------------------------------------------------" << std::endl;

	for (int i = 0; i < theta.size(); i++) {
		std::cout << "[" << (i + 1) << "]: " << theta[i] << " (Original value: " << initial_values[i] << ")" << std::endl;
	}
	std::cout << "\n\n";

}