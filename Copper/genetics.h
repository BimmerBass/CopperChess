#pragma once
#include <random>
#include <thread>


#include "texel.h"


#define WAC_THREADS 2
#define MAX_DEL 7500

constexpr double CROSSOVER_RATE = 0.75;
constexpr double MUTATION_RATE = 0.05;
//constexpr double ALPHA = 0.2;


inline void seed_random() {
	std::srand(std::chrono::duration_cast<std::chrono::nanoseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
}

// This function simply returns a random number in the range [start, end]
inline int random_num(int start, int end) {
	seed_random();
	int range = (end - start) + 1;
	return (start + (std::rand() % range));
}


/*
This is the mutation range which increases by the value of the variable and decreases with the fitness of the cromosome.
This is done to make the change larger for at larger value, because it probably wouldn't make a difference with a little change,
and a smaller value with high fitness because we don't want to explore the search space with a good chromosome.
*/
inline int mutation_range(int value, int fitness) {
	int range = int(0.084 * double(value) + 5 - pow(1.000157, double(fitness)));

	return (range > 0) ? range : 10;
}


struct Parameter {
	Parameter(int* var, int max_d = 0) {
		variable = var;
		old_value = *var;
		if (max_d == 0) {
			max_delta = MAX_DEL;
		}
		else {
			max_delta = max_d;
		}
	}

	int* variable;

	int old_value;
	int max_delta;
};


struct FittestData {
	FittestData(double fitness_value, int iteration_number, std::vector<int> theta) {
		fitness = fitness_value;
		error = 1.0 / fitness;

		iteration = iteration_number;

		for (int i = 0; i < theta.size(); i++) {
			variables.push_back(theta[i]);
		}
	}

	double fitness = 0.0;
	double error = 0.0;

	int iteration = 0;

	std::vector<int> variables;
};

typedef std::vector<FittestData> GenAdvancement;


struct Chromosome {
	std::vector<int> genes;

	double fitness = 0;
	double selection_probability = 0.0;
};


void bubble_sort(std::vector<Chromosome*> &list);


class SearchWac{
public:
	SearchWac(std::string searchStr, const char* fen, std::string good_moves[5]);

	~SearchWac();

	bool passed_position();

	void search_position();

private:
	bool passed = false;

	S_BOARD* pos = NULL;

	std::string search_parameter = "";
	std::string best_moves[5] = {};
	const char* position_fen;
};


class Generation {
public:
	Generation(std::vector<Chromosome> pop, std::vector<Parameter*> parameters, int pop_cnt);

	// Fitness testing
	void get_generation_fitness();

	// This way of finding the fitness function uses the inverse error of the texel tuning method as a fitness value
	void get_generation_error(texel::tuning_positions* EPDS, double k);
	
	void generate_new_population();

	// Selection
	std::vector<Chromosome*> select_parents(std::vector<Chromosome*> pop);

	// Genetic operators
	Chromosome crossover(Chromosome parent1, Chromosome parent2);
	void mutate(Chromosome* offspring);

	Chromosome get_fittest_individual();
	std::vector<int> return_fitness();
	std::vector<Chromosome> return_individuals();

private:
	// The vector holding all individuals in the generation.
	std::vector<Chromosome> population;
	std::vector<Parameter*> tuning_parameters;

	int population_count;
	double generation_fitness;
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