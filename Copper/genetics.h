#pragma once
#include <random>
#include <thread>

#include "defs.h"


#define WAC_THREADS 2

constexpr double CROSSOVER_RATE = 0.75;
constexpr double MUTATION_RATE = 0.005;


static std::default_random_engine generator;
static std::bernoulli_distribution distribution(0.5);

// Bernoulli +-1 distribution with p = 50%
inline double randemacher() {
	return (distribution(generator)) ? 1.0 : -1.0;
}

inline void seed_random() {
	std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
}

// This function simply returns a random number in the range [start, end]
inline int random_num(int start, int end) {
	seed_random();
	int range = (end - start) + 1;
	return (start + (std::rand() % range));
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


struct Chromosome {
	std::vector<int> genes;

	int fitness = 0;
	double selection_probability = 0.0;
};


void bubble_sort(std::vector<Chromosome*> &list);


class SearchWac{
public:
	SearchWac(std::string searchStr, const char* fen, std::string good_moves[5]);

	bool passed_position();

	void search_position();

private:
	bool passed = false;

	std::string search_parameter = "";
	std::string best_moves[5] = {};
	const char* position_fen;
};


class Generation {
public:
	Generation(std::vector<Chromosome> pop, std::vector<Parameter*> parameters, int pop_cnt);

	// Fitness testing
	void get_generation_fitness();

	void generate_new_population();

	// Selection
	std::vector<Chromosome*> select_parents(std::vector<Chromosome*> pop);

	// Genetic operators
	std::vector<Chromosome> crossover(Chromosome parent1, Chromosome parent2);
	void mutate(Chromosome* offspring);

	std::vector<int> return_fitness();
	std::vector<Chromosome> return_individuals();

private:
	// The vector holding all individuals in the generation.
	std::vector<Chromosome> population;
	std::vector<Parameter*> tuning_parameters;

	int population_count;
	int generation_fitness;
};


class GeneticTuning {
public:
	GeneticTuning(std::vector<Parameter> params, int popcnt, int gencnt);

	void run_ga();

private:
	std::vector<int*> tuning_parameters; // Pointers to the evaluation / search parameters to be optimized.
	std::vector<Parameter> parameter_settings; // If the parameters have max deviations set, we need to remember this.

	int population_count;

	int generations;

	Generation* current_gen = nullptr;

};