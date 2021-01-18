#pragma once
#include "defs.h"
#include "evaluation.h"
#include "code_analysis.h"

#include <fstream>
#include <random>
#include <algorithm>
#include <iomanip>
#include <sstream>



inline void seed_random() {
	std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
}

// This function simply returns a random number in the range [start, end]
inline int random_num(int start, int end) {
	seed_random();
	int range = (end - start) + 1;
	return (start + (std::rand() % range));
}




static int k_precision = 5; // The number of iterations used to find the optimal k
static double k_initial = 1.0;

const int partitions = 20;

/*
SPSA parameters
*/
const double C_END = 5.0;
const double R_END = 0.002;

const double alpha = 0.602;
const double gamma = 0.101;


//const double beta_0 = 0.999;
//const double gamma_0 = 0.999;

const double beta_0 = 0.9;
const double gamma_0 = 0.9;

const double lambda = 0.4;

const double epsilon = 1.0 * pow(10.0, -8.0);



static std::default_random_engine generator;
static std::bernoulli_distribution distribution(0.5);

// Bernoulli +-1 distribution with p = 50%
inline double randemacher() {
	return (distribution(generator)) ? 1 : -1;
}


// Function to get the date and time to write to the filename
inline std::string getDateTime()
{
	auto time = std::time(nullptr);
	std::stringstream ss;

#if (defined(_WIN32) || defined(_WIN64))
	tm ltm;
	localtime_s(&ltm, &time);
	ss << std::put_time(&ltm, "%F_%T"); // ISO 8601 without timezone information.
#else
	ss << std::put_time(std::localtime(&time), "%F_%T");
#endif
	auto s = ss.str();
	std::replace(s.begin(), s.end(), ':', '-');
	return s;
}


inline double sigmoid(double q, double k) {
	return (double)(1.0 / (1 + pow(10, -(k * q / 400.0))));
}


struct DataPoint {
	DataPoint(double error, int iteration_number, double g_hat, double BIG_G, std::vector<int> theta) {
		error_val = error;
		iteration = iteration_number;

		gradient = g_hat;
		step = BIG_G;
		
		for (int i = 0; i < theta.size(); i++) {
			variables.push_back(theta[i]);
		}
	}

	double error_val;
	int iteration;

	double gradient;
	double step;
	
	std::vector<int> variables;
};

typedef std::vector<DataPoint> ErrorData;

namespace texel {


	struct Parameter {
		Parameter(int* var, int min = -INF, int max = INF) {
			variable = var;

			min_val = (min == -INF) ? (*var - INF) : min;
			max_val = (max == INF) ? (*var + INF) : max;
		}

		int* variable;

		int min_val;
		int max_val;
	};


	struct texel_pos {
		std::string fen = "";

		char* getFen() {
			char* f = new char[fen.length() + 1];
#if (defined(_WIN32) || defined(_WIN64))
			strcpy_s(f, fen.length() + 1, fen.c_str());
#else
			strcpy(fen, pos.c_str());
#endif
			return f;
		};

		double game_result = 0.0;
	};

	struct tuning_positions {
		std::vector<texel_pos> positions;
	};

	tuning_positions* load_file(std::string filepath);


	double find_k(tuning_positions* EPDS);

	double eval_error(tuning_positions* EPDS, double k);

	double changed_eval_error(std::vector<int*> params, std::vector<int> new_values, tuning_positions* EPDS, double k);

	void tune(std::vector<texel::Parameter> initial_guess, std::string epd_file, int runs = 0);
}