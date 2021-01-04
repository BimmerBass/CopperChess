#include "genetics.h"
#include "wac_positions.h"


bool operator < (const Chromosome& c1, const Chromosome& c2) {
	return c1.fitness < c2.fitness;
}
bool operator > (const Chromosome& c1, const Chromosome& c2) {
	return c1.fitness > c2.fitness;
}
bool operator == (const Chromosome& c1, const Chromosome& c2) {
	return ((c1.genes == c2.genes) && (c1.fitness == c2.fitness) && (c1.selection_probability == c2.selection_probability));
}


int attempted_mutations = 0;
int mutations_happened = 0;

int new_chromosomes = 0;
int reproductions = 0;
int elitism_chosen = 0;


GeneticTuning::GeneticTuning(std::vector<Parameter> params, int popcnt, int gencnt) {
	
	population_count = popcnt;
	generations = gencnt;

	try {
		for (int p = 0; p < params.size(); p++) {
			if (params[p].variable == nullptr) {
				throw 1;
			}

			tuning_parameters.push_back(params[p].variable);
			parameter_settings.push_back(params[p]);

		}
	}
	catch(int e) {
		std::cout << "Error initializing GA. Exception nr. " << e << ": " << 
			((e == 1) ? "A null-pointer was passed as tuning parameter." : "") << std::endl;
	}

}


Generation::Generation(std::vector<Chromosome> pop, std::vector<Parameter*> parameters, int pop_cnt) {
	population_count = pop_cnt;
	generation_fitness = 0;

	// Populate the vector of parameters, that we're tuning.
	for (int p = 0; p < parameters.size(); p++) {
		tuning_parameters.push_back(parameters[p]);
	}


	// If the pop(-ulation) vector is empty, this is the first generation, and we'll have to generate our own parameters.
	for (int c = 0; c < pop_cnt; c++) {
		Chromosome new_chromosome;

		for (int p = 0; p < parameters.size(); p++) {

			if (parameters[p]->max_delta <= 0) { // If there has not been set a max allowable deviation of the parameter, just generate a random value.
				seed_random(); // Seed the random number generator with the time since epoch in nanoseconds.
				new_chromosome.genes.push_back(randemacher() * std::rand());
			}
			else { // Generate a random number from value - max_delta to variable + max_delta
				int new_value = random_num(parameters[p]->old_value - parameters[p]->max_delta,
					parameters[p]->old_value + parameters[p]->max_delta);
				new_chromosome.genes.push_back(new_value);
			}
		}

		population.push_back(new_chromosome);

	}
}

/*
The SearchWac class is an attempt to make get_generation_fitness() multithreaded.
*/
SearchWac::SearchWac(std::string searchStr, const char* fen, std::string good_moves[5]) {
	search_parameter = searchStr;

	
	for (int i = 0; i < 5; i++) {
		best_moves[i] = good_moves[i];
	}

	position_fen = fen;

	/*catch (std::bad_alloc& ba) {
		std::cout << "Memory allocation error in SearchWac constructor: " << ba.what() << std::endl;
	}*/
}

bool SearchWac::passed_position() {
	return passed;
}


void SearchWac::search_position() {
	S_BOARD* pos = new S_BOARD();
	BoardRep::parseFen(position_fen, *pos);

	S_SEARCHINFO info;
	passed = false; // By default.

	// Now we'll parse the go-command using the UCI interface, but firstly we need to convert the std::string to a char* (not const)
	char* goLine = new char[search_parameter.size() + 1];
	std::copy(search_parameter.begin(), search_parameter.end(), goLine);
	goLine[search_parameter.size()] = '\0';

	ParseGo(goLine, &info, pos);

	// goLine is declared on heap, so we'll need to delete it manually.
	delete[] goLine;


	// Now we can search the posiiton:
	Search::searchPosition(pos, &info);

	// Now we can check if the move found by Copper matches any of the ones in the WAC-test.
	int bestMove = NOMOVE;

	try {
		bestMove = pos->pvArray[0];
		
		if (bestMove == NOMOVE) {
			throw 1;
		}
	}
	catch (int e) {
		std::cout << "Exception occured in SearcWac::search_position(): Copper didn't return any move." << std::endl;
	}

	for (int m = 0; m < 5; m++) {
		// Check if the move matches any of the good moves:
		if (printMove(bestMove) == best_moves[m]) { // Copper found a good move
			passed = true;
			break;
		}
	}
	delete pos;
}


/*
Helper function for running functions inside objects with multithreading
*/
void doRun(SearchWac* instance) {
	instance->search_position();
}


/*
The fitness function is simply the amount of WAC positions that the engine gets right with each chromosome's values.
*/

void Generation::get_generation_fitness() {
	S_BOARD* pos = new S_BOARD;

	int passed;
	generation_fitness = 0;

	for (int c = 0; c < population_count; c++) {
		passed = 0;
		BoardRep::clearBoard(pos);
		/*
		Before testing the engine with the values given from the current chromosome, we'll have to insert them.
		*/
		for (int v = 0; v < population[c].genes.size(); v++) {
			*(tuning_parameters[v]->variable) = population[c].genes[v];
		}

		/*
		Now we can run through all the WAC positions with Copper.
		*/

		int total_positions = 0;

		std::vector<SearchWac> searches_runnning;
		std::vector<std::thread> threads;

		while (total_positions < 50) {
			searches_runnning.clear();
			threads.clear();

			int this_position = total_positions;

			// Initialise SearchWac objects.
			for (int n = 0; n < WAC_THREADS; n++) {
				searches_runnning.push_back(SearchWac("go depth 1", WAC_positions[this_position + n], WAC_moves[this_position + n]));
			}

			this_position += WAC_THREADS;

			for (int t = 0; t < searches_runnning.size(); t++) {
				std::thread worker(doRun, &searches_runnning[t]);
				threads.push_back(std::move(worker));
			}

			// Wait for the searches to finish and check the search results.
			for (int t = 0; t < threads.size(); t++) {
				threads[t].join();
			}

			for (int s = 0; s < searches_runnning.size(); s++) {
				passed += (searches_runnning[s].passed_position()) ? 1 : 0;
			}

			total_positions = this_position;
		}


		/*
		// Usually we'll use all positions but for testing i only use the first fifty
		for (int b = 0; b < 50; b++) {

			int p = random_num(0, WAC_LENGTH - 1);

			// Parse the WAC-fen string
			BoardRep::parseFen(WAC_positions[p], *pos);

			// Initialize a search-info structure and parse a "go"-line. This is just done using the UCI-interface.
			S_SEARCHINFO info;

			char goLine[] = "go depth 1";

			ParseGo(goLine, &info, pos);


			// After parsing the go-command, search the position.
			Search::searchPosition(pos, &info);


			// Now check if the best move suggested by Copper matches any of the ones in the WAC test.
			int bestMove;
			try {
				bestMove = pos->pvArray[0];

				if (bestMove == NOMOVE) {
					throw 1;
				}
			}
			catch (int e) {
				std::cout << "Exception occured during chromosome testing: " << "Copper didn't return a best move." << std::endl;
			}

			for (int m = 0; m < 5; m++) {
				// The move matches one of the ones suggested by WAC.
				if (printMove(bestMove) == WAC_moves[p][m]) {
					passed++;
					break;
				}
			}
		}*/
		
		// After running the WAC test, set the fitness of this chromosome and increment the total fitness by the same value.
		population[c].fitness = passed * passed;
		generation_fitness += passed * passed;
	}

	// After having gone through all the chromosomes and having gotten their fitness, calculate their selection probability. Given by prob = fitness/total_fitness
	
	// If the totalt fitness is zero, they should all have the same probability.
	if (generation_fitness == 0) {
		for (int c = 0; c < population_count; c++) {
			population[c].selection_probability = double(1) / double(population_count);
		}
	}
	else {

		for (int c = 0; c < population_count; c++) {
			population[c].selection_probability = double(population[c].fitness) / double(generation_fitness);
		}
	}

	// Delete the S_BOARD object
	delete pos;
}


/*
Crossover function: A random number between 1 and the chromosome length minus 1 is chosen and the resulting genomes of the parents are swapped to create two
	different but still similar offspring
*/

std::vector<Chromosome> Generation::crossover(Chromosome parent1, Chromosome parent2) {

	try {
		if (parent1.genes.size() != parent2.genes.size()) {
			throw 1;
		}
	}
	catch (int e) {
		std::cout << "Exception occured in crossover function: The gene sequences of the parents aren't the same length." << std::endl;
	}


	// The random gene number.
	int gene_num = random_num(1, parent1.genes.size() - 1);


	std::vector<Chromosome> offspring;

	Chromosome offspring1;
	Chromosome offspring2;

	// Everything with index before the gene_num should be the same
	for (int g = 0; g < gene_num; g++) {
		offspring1.genes.push_back(parent1.genes[g]);
		offspring2.genes.push_back(parent2.genes[g]);
	}

	// Everything after - including the element at - the gene_num should be switched
	for (int g = gene_num; g < parent1.genes.size(); g++) {
		offspring1.genes.push_back(parent2.genes[g]);
		offspring2.genes.push_back(parent1.genes[g]);
	}

	offspring.push_back(offspring1);
	offspring.push_back(offspring2);

	return offspring;
}


/*
A mutation can happen when making a new generation. When a mutation happens one gene gets changed to a random value (inside it's set bounds if any).
*/

void Generation::mutate(Chromosome* offspring) {

	// Generate a random number representing the gene that is to be changed
	//int gene_num = random_num(0, offspring->genes.size() - 1);

	// Before changing the gene, check the parameter that it represents to see if it is bounded to a certain value range.
	/*if (tuning_parameters[gene_num]->max_delta != 0) {
		offspring->genes[gene_num] = random_num(tuning_parameters[gene_num]->old_value - tuning_parameters[gene_num]->max_delta,
			tuning_parameters[gene_num]->old_value + tuning_parameters[gene_num]->max_delta);
	}
	else {
		seed_random();
		offspring->genes[gene_num] = randemacher() * std::rand();
	}*/

	for (int i = 0; i < offspring->genes.size(); i++) {
		double prob = double(random_num(0, 1000)) / 1000.0;

		attempted_mutations++;

		if (prob < MUTATION_RATE) { // We can make a random mutation on this gene.

			mutations_happened++;

			if (tuning_parameters[i]->max_delta != 0) {
				offspring->genes[i] = random_num(tuning_parameters[i]->old_value - tuning_parameters[i]->max_delta,
					tuning_parameters[i]->old_value + tuning_parameters[i]->max_delta);
			}
			else {
				seed_random();
				offspring->genes[i] = randemacher() * std::rand();
			}


		}
	}

}



/*
This is probably a pretty inefficient bubbleSort implementation, but it will work for this purpose. It sorts the list such that the highest numbers are first.
*/

void bubble_sort(std::vector<Chromosome*> &list) {

	for (int i = 0; i < list.size(); i++) {
		for (int j = 0; j < (list.size() - i - 1); j++) {
			if ((*list[j]) < (*list[j + 1])) {
				Chromosome* temp = list[j];
				list[j] = list[j + 1];
				list[j + 1] = temp;
			}
		}
	}
}


/*
The selection just selects two parents from the population based on their probabilities of selection to breed.
*/

std::vector<Chromosome*> Generation::select_parents(std::vector<Chromosome*> pop) {
	std::vector<Chromosome*> sorted_list;

	for (int i = 0; i < pop.size(); i++) {
		sorted_list.push_back(pop[i]);
	}

	// Now we sort the list based on bubble_sort
	bubble_sort(sorted_list);
	std::vector<Chromosome*> parents;

	select_parent:

	// Generate a random number between zero and one.
	double prob = double(random_num(0, 1000)) / 1000.0;

	// Now loop through the sorted list and add up the selection probabilities, and the first one whose accumulated probability exceeds prob
	//	will be chosen for reproduction

	double accumulated_probability = 0.0;

	for (int i = 0; i < sorted_list.size(); i++) {
		accumulated_probability += sorted_list[i]->selection_probability;

		if (accumulated_probability > prob) {
			parents.push_back(sorted_list[i]); // Add the first parent to the list
			sorted_list.erase(sorted_list.begin() + i); // Remove it from the sorted_list
			break;
		}
	}


	// If there is less than two parents in the output vector, go back.
	if (parents.size() < 2) {
		goto select_parent;
	}

	try {
		if (parents.size() != 2) {
			throw 1;
		}
	}
	catch (int e) {
		std::cout << "Size of std::vector<Chromosome> parents is not equal to two." << std::endl;
	}

	// Now that there are two parents, we can return.
	return parents;
}


void Generation::generate_new_population() {

	std::vector<Chromosome> new_generation;

	std::vector<Chromosome*> old_generation;
	for (int i = 0; i < population_count; i++) {
		old_generation.push_back(&population[i]);
	}

	bubble_sort(old_generation);

	
	while (new_generation.size() < population_count) {
		new_chromosomes++;
		double prob = (double)random_num(0, 1000) / 1000.0;

		

		// 85% of the time, we want to do crossover between two parents.
		if (prob < CROSSOVER_RATE) {
			reproductions++;

			// Find two suitable parents for reproduction.
			std::vector<Chromosome*> parents = select_parents(old_generation);

			// Create two distinct offspring from the parents.
			std::vector<Chromosome> offspring = crossover(*parents[0], *parents[1]);

			// Now divide the probability of choosing parent1 and parent2 by two, such that they probably wont be chosen to reproduce again.
			parents[0]->selection_probability /= 2;
			parents[1]->selection_probability /= 2;

			new_generation.push_back(offspring[0]);
			
			// Only if we haven't met the population size limit, we'll add the other offspring.
			if (new_generation.size() < population_count) {
				new_generation.push_back(offspring[1]);
			}
		}
		else { // If we aren't doing crossover, we'll just take one of the three best chromosomes.
			elitism_chosen++;

			int elite_num = random_num(0, 2);

			new_generation.push_back(*old_generation[elite_num]);
		}
	}

	// After having made the new generation we'll perform mutations on it and add replace the old generation with it.

	try {
		if (new_generation.size() != population_count) {
			throw 1;
		}
	}
	catch (int e) {
		std::cout << "New generation size does not match the given population count" << std::endl;
	}

	for (int i = 0; i < new_generation.size(); i++) {
		mutate(&new_generation[i]);
		new_generation[i].fitness = 0;
		new_generation[i].selection_probability = 0.0;
		population[i] = new_generation[i];
	}
}


std::vector<int> Generation::return_fitness() {
	std::vector<int> fitness_vector;

	for (int i = 0; i < population_count; i++) {
		fitness_vector.push_back(population[i].fitness);
	}
	return fitness_vector;
}

std::vector<Chromosome> Generation::return_individuals() {
	std::vector<Chromosome> ind_vector;

	for (int i = 0; i < population_count; i++) {
		ind_vector.push_back(population[i]);
	}
	return ind_vector;
}


void GeneticTuning::run_ga() {
	attempted_mutations = 0;
	mutations_happened = 0;

	new_chromosomes = 0;
	reproductions = 0;
	elitism_chosen = 0;

	std::vector<Parameter*> parameter_settings_ptrs;
	std::vector<Chromosome> initPop;
	for (int i = 0; i < parameter_settings.size(); i++) {
		parameter_settings_ptrs.push_back(&parameter_settings[i]);
	}

	// The constructor of Generation will automatically generate random chromosomes.
	current_gen = new Generation(initPop, parameter_settings_ptrs, population_count);
	std::vector<Chromosome> population;


	// Now we'll loop through the generations
	for (int i = 0; i < generations; i++) {
		std::cout << "Running generation nr. " << i + 1 << "\n";
		std::cout << "Current generation chromosomes: " << "\n\n";

		population.clear();
		population = current_gen->return_individuals();

		for (int i = 0; i < population.size(); i++) {
			std::cout << "Chromosome nr. " << i + 1 << ": ";

			for (int j = 0; j < population[i].genes.size(); j++) {
				if (j != population[i].genes.size() - 1) {
					std::cout << population[i].genes[j] << ", ";
				}
				else {
					std::cout << population[i].genes[j];
				}
			}
			std::cout << "\n";
		}


		std::chrono::time_point<std::chrono::high_resolution_clock> start_time = std::chrono::high_resolution_clock::now();

		// Get the generation fitness:
		current_gen->get_generation_fitness();


		std::chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::seconds>(start_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::seconds>(end_time).time_since_epoch().count();

		std::cout << "Duration of finding fitness function: " << (end - start) << " seconds." << std::endl;

		// Get the acquired fitness values.
		std::vector <int> fitness = current_gen->return_fitness();

		std::cout << "Gen. " << i + 1 << " fitness values: ";
		for (int i = 0; i < fitness.size(); i++) {
			std::cout << fitness[i] << ", ";
		}
		std::cout << "\n";

		// Just add the new chromosomes since they hold the fitness values, and we'll need those to display the top 50% after running the GA.
		population.clear();
		population = current_gen->return_individuals();


		// After having gotten the fitness values, generate the new generation.
		current_gen->generate_new_population();

		std::cout << "\nActual Mutation rate: " << (double(mutations_happened) / double(attempted_mutations)) * 100 << "%" << std::endl;
		std::cout << "Actual crossover rate: " << (double(reproductions) / double(new_chromosomes)) * 100 << "%" << std::endl;
		std::cout << "Actual elitism rate: " << (double(elitism_chosen) / double(new_chromosomes)) * 100 << "%" << std::endl;

		std::cout << "\n\n";
	}

	// When we're done, print out the top 50% of the population:
	std::cout << "Genetic tuning done. The top 50% of the last tested population are: " << std::endl;
	std::cout << "(Same order as the input, and from best to worst)\n\n";

	std::vector<Chromosome*> pop_ptr;

	for (int i = 0; i < population.size(); i++) {
		pop_ptr.push_back(&population[i]);
	}

	bubble_sort(pop_ptr);

	if (pop_ptr.size() > 2) {
		for (int i = 0; i < (pop_ptr.size() / 2); i++) {
			std::cout << "Chromosome nr. " << i + 1 << ". Fitness: " << pop_ptr[i]->fitness << ": ";

			for (int j = 0; j < pop_ptr[i]->genes.size(); j++) {
				std::cout << pop_ptr[i]->genes[j] << ", ";
			}
			std::cout << "\n";
		}
	}
	else {
		for (int i = 0; i < pop_ptr.size(); i++) {
			std::cout << "Chromosome nr. " << i + 1 << ": ";

			for (int j = 0; j < pop_ptr[i]->genes.size(); j++) {
				std::cout << pop_ptr[i]->genes[j] << ", ";
			}
			std::cout << "\n";
		}
	}
}