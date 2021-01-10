#include "defs.h"
#include "texel.h"

#include <bitset>


/*int* params[20] = { &eval::pawnValMg, &eval::knightValMg, &eval::bishopValMg, &eval::rookValMg, &eval::queenValMg,
	&eval::pawnValEg, &eval::knightValEg, &eval::bishopValEg, &eval::rookValEg, &eval::queenValEg,
	&passedPawnValue[0], &passedPawnValue[1], &passedPawnValue[2], &passedPawnValue[3], &passedPawnValue[4],
	&passedPawnValue[5], &passedPawnValue[6], &passedPawnValue[7], &outpost_bonus, &safe_outpost_bonus
};*/

int main() {
	initAll(SetMask, ClearMask);

	//texel::tuning_positions* EPDS = texel::load_file("C:\\Users\\abild\\Desktop\\tuner_positions\\\quiet-labeled.epd");

	//double k = texel::find_k(EPDS);

	/*std::vector<int*> init_g;

	init_g.push_back(&eval::knightValMg);
	init_g.push_back(&eval::bishopValMg);
	init_g.push_back(&eval::rookValMg);
	init_g.push_back(&eval::queenValMg);

	init_g.push_back(&eval::knightValEg);
	init_g.push_back(&eval::bishopValEg);
	init_g.push_back(&eval::rookValEg);
	init_g.push_back(&eval::queenValEg);

	texel::tune(init_g, "C:\\Users\\abild\\Desktop\\tuner_positions\\\quiet-labeled.epd", 200);*/

	Uci_Loop();


	cleanPolyBook();
	return 0;
}
