#include "defs.h"
#include "genetics.h"



/*int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};*/

int main() {
	initAll(SetMask, ClearMask);


	/*std::vector<texel::Parameter> init_g;

	init_g.push_back(texel::Parameter(&outpost_bonus_mg, 0, 70));
	init_g.push_back(texel::Parameter(&outpost_bonus_eg, 0, 70));

	init_g.push_back(texel::Parameter(&safe_outpost_bonus_mg, 0, 100));
	init_g.push_back(texel::Parameter(&safe_outpost_bonus_eg, 0, 100));

	init_g.push_back(texel::Parameter(&knight_outpost_bonus_mg, 1, 5));
	init_g.push_back(texel::Parameter(&knight_outpost_bonus_eg, 1, 5));

	init_g.push_back(texel::Parameter(&N_pawn_defence_mg, 5, 15));
	init_g.push_back(texel::Parameter(&B_pawn_defence_mg, 1, 10));

	init_g.push_back(texel::Parameter(&PawnOn_bCol_mg, 0, 10));
	init_g.push_back(texel::Parameter(&PawnOn_bCol_eg, 0, 10));

	init_g.push_back(texel::Parameter(&bishop_kingring_mg, 10, 60));

	init_g.push_back(texel::Parameter(&enemy_pawns_on_diag_eg, 0, 15));

	init_g.push_back(texel::Parameter(&doubled_rooks_mg, 25, 100));

	init_g.push_back(texel::Parameter(&rook_on_queen_mg, 0, 40));
	init_g.push_back(texel::Parameter(&rook_on_queen_eg, 0, 40));

	init_g.push_back(texel::Parameter(&rook_behind_passer_eg, 20, 150));

	init_g.push_back(texel::Parameter(&rook_kingring_mg, 5, 40));
	init_g.push_back(texel::Parameter(&rook_kingring_eg, 5, 40));

	init_g.push_back(texel::Parameter(&open_rook_mg, 20, 90));
	init_g.push_back(texel::Parameter(&open_rook_eg, 10, 90));

	init_g.push_back(texel::Parameter(&semi_rook_mg, 5, 45));
	init_g.push_back(texel::Parameter(&semi_rook_eg, 5, 45));

	init_g.push_back(texel::Parameter(&queen_behind_passer_eg, 0, 40));
	init_g.push_back(texel::Parameter(&queen_kingDist_bonus_eg, 1, 15));


	for (int i = 0; i < init_g.size(); i++) {
		int new_val = random_num(init_g[i].min_val, init_g[i].max_val);

		*init_g[i].variable = std::max(std::min(new_val, init_g[i].max_val), init_g[i].min_val);
	}

	texel::tune(init_g, "C:\\Users\\abild\\Desktop\\tuner_positions\\quiet-labeled.epd", 200);*/
	
	//int population_count = 12;
	//int generation_count = 200;
	//
	//GeneticTuning tuner(init_g, population_count, generation_count);
	//
	//tuner.run_ga();

	
	
	Uci_Loop();


	cleanPolyBook();
	return 0;
}


/*


	init_g.push_back(Parameter(&outpost_bonus_mg, 40));
	init_g.push_back(Parameter(&outpost_bonus_eg, 40));

	init_g.push_back(Parameter(&safe_outpost_bonus_mg, 100));
	init_g.push_back(Parameter(&safe_outpost_bonus_eg, 100));

	init_g.push_back(Parameter(&knight_outpost_bonus_mg, 10));
	init_g.push_back(Parameter(&knight_outpost_bonus_eg, 10));

	init_g.push_back(Parameter(&N_pawn_defence_mg, 50));
	init_g.push_back(Parameter(&B_pawn_defence_mg, 50));

	init_g.push_back(Parameter(&PawnOn_bCol_mg, 20));
	init_g.push_back(Parameter(&PawnOn_bCol_eg, 20));

	init_g.push_back(Parameter(&bishop_kingring_mg, 100));

	init_g.push_back(Parameter(&enemy_pawns_on_diag_eg, 20));

	init_g.push_back(Parameter(&doubled_rooks_mg, 150));

	init_g.push_back(Parameter(&rook_on_queen_mg, 40));
	init_g.push_back(Parameter(&rook_on_queen_eg, 40));

	init_g.push_back(Parameter(&rook_behind_passer_eg, 150));

	init_g.push_back(Parameter(&rook_kingring_mg, 50));
	init_g.push_back(Parameter(&rook_kingring_eg, 50));

	init_g.push_back(Parameter(&open_rook_mg, 70));
	init_g.push_back(Parameter(&open_rook_eg, 70));

	init_g.push_back(Parameter(&semi_rook_mg, 60));
	init_g.push_back(Parameter(&semi_rook_eg, 60));

	init_g.push_back(Parameter(&queen_behind_passer_eg, 70));
	init_g.push_back(Parameter(&queen_kingDist_bonus_eg, 10));

*/