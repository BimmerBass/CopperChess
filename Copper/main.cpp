#include "defs.h"
#include "genetics.h"
#include "evaluation.h"


int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};

int main() {
	initAll(SetMask, ClearMask);


	/*std::vector<Parameter> tuning_parameters;
	
	tuning_parameters.push_back(Parameter(&eval::pawnValMg, 200));
	tuning_parameters.push_back(Parameter(&eval::knightValMg, 300));
	tuning_parameters.push_back(Parameter(&eval::bishopValMg, 300));
	tuning_parameters.push_back(Parameter(&eval::rookValMg, 450));
	tuning_parameters.push_back(Parameter(&eval::queenValMg, 1050));

	tuning_parameters.push_back(Parameter(&eval::pawnValEg, 200));
	tuning_parameters.push_back(Parameter(&eval::knightValEg, 300));
	tuning_parameters.push_back(Parameter(&eval::bishopValEg, 300));
	tuning_parameters.push_back(Parameter(&eval::rookValEg, 450));
	tuning_parameters.push_back(Parameter(&eval::queenValEg, 1050));


	/*tuning_parameters.push_back(Parameter(&passedPawnValue[0], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[1], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[2], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[3], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[4], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[5], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[6], 200));
	tuning_parameters.push_back(Parameter(&passedPawnValue[7], 200));


	tuning_parameters.push_back(Parameter(&outpost_bonus, 150));
	tuning_parameters.push_back(Parameter(&safe_outpost_bonus, 200));
	
	int population_count = 8;
	int generation_count = 100;

	GeneticTuning tuner(tuning_parameters, population_count, generation_count);

	tuner.run_ga();
	*/


	Uci_Loop();


	cleanPolyBook();
	return 0;
}