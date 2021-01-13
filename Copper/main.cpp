#include "defs.h"
#include "genetics.h"



/*int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};*/

int main() {
	initAll(SetMask, ClearMask);


	std::vector<texel::Parameter> init_g;





	for (int i = 0; i < init_g.size(); i++) {
		int new_val = random_num(init_g[i].min_val, init_g[i].max_val);

		*init_g[i].variable = std::max(std::min(new_val, init_g[i].max_val), init_g[i].min_val);
	}

	texel::tune(init_g, "C:\\Users\\abild\\Desktop\\tuner_positions\\quiet-labeled.epd", 200);
	
	//int population_count = 12;
	//int generation_count = 200;
	//
	//GeneticTuning tuner(init_g, population_count, generation_count);
	//
	//tuner.run_ga();

	
	
	//Uci_Loop();


	cleanPolyBook();
	return 0;
}
