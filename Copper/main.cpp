#include "defs.h"



/*int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};*/

int main() {
	initAll(SetMask, ClearMask);

	//texel::tuning_positions* EPDS = texel::load_file("C:\\Users\\abild\\Desktop\\tuner_positions\\\quiet-labeled.epd");

	//double k = texel::find_k(EPDS);
	/*
	std::vector<int*> init_g;

	init_g.push_back(&doubled_penalty_mg);
	//init_g.push_back(&doubled_penalty_eg);
	//init_g.push_back(&passedPawnValue[0]);
	//init_g.push_back(&passedPawnValue[1]);
	//init_g.push_back(&passedPawnValue[2]);
	//init_g.push_back(&passedPawnValue[3]);
	//init_g.push_back(&passedPawnValue[4]);
	//init_g.push_back(&passedPawnValue[5]);
	//init_g.push_back(&passedPawnValue[6]);
	//init_g.push_back(&passedPawnValue[7]);
	init_g.push_back(&DoubledIsolatedMg);
	//init_g.push_back(&DoubledIsolatedEg);
	init_g.push_back(&isolatedMg);
	//init_g.push_back(&isolatedEg);
	//init_g.push_back(&supported_mg);
	init_g.push_back(&supported_eg);


	texel::tune(init_g, "C:\\Users\\abild\\Desktop\\tuner_positions\\\quiet-labeled.epd", 200);*/

	//std::vector<Parameter> init_g;
	//
	//init_g.push_back(Parameter(&doubled_penalty_mg, 20));
	//init_g.push_back(Parameter(&DoubledIsolatedMg, 20));
	//init_g.push_back(Parameter(&isolatedMg, 20));
	//init_g.push_back(Parameter(&supported_eg, 30));
	//
	//int population_count = 15;
	//int generation_count = 100;
	//
	//GeneticTuning tuner(init_g, population_count, generation_count);
	//
	//tuner.run_ga();

	Uci_Loop();


	cleanPolyBook();
	return 0;
}
