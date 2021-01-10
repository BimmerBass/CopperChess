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
#pragma omp for schedule(static, 50000) reduction(+:error)
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