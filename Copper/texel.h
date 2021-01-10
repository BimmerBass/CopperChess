#pragma once
#include "defs.h"
#include "evaluation.h"
#include "code_analysis.h"

#include <fstream>
#include <random>




static int k_precision = 5; // The number of iterations used to find the optimal k
static double k_initial = 1.0;

const int partitions = 20;

/*
SPSA parameters
*/
const int A_END = 8;
const int C_END = 4;
const double alpha = 0.602;
const double gamma = 0.101;


static std::default_random_engine generator;
static std::bernoulli_distribution distribution(0.5);

// Bernoulli +-1 distribution with p = 50%
inline int randemacher() {
	return (distribution(generator)) ? 1 : -1;
}


struct Parameter {
	Parameter(int* var, int max_d = 0) {
		variable = var;
		old_value = *var;

		max_delta = max_d;
	}

	int* variable;

	int old_value;
	int max_delta;
};


inline double sigmoid(double q, double k) {
	return (double)(1.0 / (1 + pow(10, -(k * q / 400))));
}


namespace texel {

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

	void tune(std::vector<int*> initial_guess, std::string epd_file, int runs = 0);
}