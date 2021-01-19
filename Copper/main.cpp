#include "defs.h"
#include "genetics.h"



/*int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};*/

int main() {
	initAll(SetMask, ClearMask);


	//std::vector<texel::Parameter> init_g;
	//
	//init_g.push_back(texel::Parameter(&knight_mob_mg));
	//init_g.push_back(texel::Parameter(&knight_mob_eg));
	//
	//init_g.push_back(texel::Parameter(&bishop_mob_mg));
	//init_g.push_back(texel::Parameter(&bishop_mob_eg));
	//
	//init_g.push_back(texel::Parameter(&rook_mob_mg));
	//init_g.push_back(texel::Parameter(&rook_mob_eg));
	//
	//init_g.push_back(texel::Parameter(&queen_mob_mg));
	//init_g.push_back(texel::Parameter(&queen_mob_eg));
	//
	//

	//for (int i = 0; i < 100; i++) {
	//	init_g.push_back(texel::Parameter(&safety_mg[i]));
	//}
	//for (int i = 0; i < 100; i++) {
	//	init_g.push_back(texel::Parameter(&safety_eg[i]));
	//}
	
	

	//texel::tune(init_g, "C:\\Users\\abild\\Desktop\\tuner_positions\\quiet-labeled.epd", 200);
	
	
	Uci_Loop();



	cleanPolyBook();
	return 0;
}
