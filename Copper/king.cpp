#include "evaluation.h"


using namespace psqt;

template <>
int eval::pawn_shield<WHITE>(const S_BOARD* pos, int kingSq, bool mg) {
	int v = 0;

	int kFile = kingSq % 8;

	int sq = 0;

	uint64_t king_defenders = 0;

	if (kFile == 0) {
		// A, B, C
		king_defenders = (pos->position[WP] & (FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]));
		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[sq] : king_defence_eg[sq];
		}

	}
	else if (kFile >= 0 && kFile < 3) {
		// A, B, C, D
		king_defenders = (pos->position[WP] & (FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C] | FileMasks8[FILE_D]));
		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[sq] : king_defence_eg[sq];
		}
	}
	else if (kFile >= 3 && kFile < 5) {
		// C, D, E, F
		king_defenders = (pos->position[WP] & (FileMasks8[FILE_C] | FileMasks8[FILE_D] | FileMasks8[FILE_E] | FileMasks8[FILE_F]));
		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[sq] : king_defence_eg[sq];
		}
	}
	else if (kFile >= 5 && kFile < 7) {
		// E, F, G, H
		king_defenders = (pos->position[WP] & (FileMasks8[FILE_E] | FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]));
		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[sq] : king_defence_eg[sq];
		}
	}
	else {
		// F, G, H
		king_defenders = (pos->position[WP] & (FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]));
		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[sq] : king_defence_eg[sq];
		}
	}

	return v;
}

template <>
int eval::pawn_shield<BLACK>(const S_BOARD* pos, int kingSq, bool mg) {
	int v = 0;

	int kFile = kingSq % 8;

	int sq = 0;

	uint64_t king_defenders = 0;

	if (kFile == 0) {
		// A, B, C --> H, G, F
		king_defenders = (pos->position[BP] & (FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]));

		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[Mirror64[sq]] : king_defence_eg[Mirror64[sq]];
		}
	}
	else if (kFile >= 0 && kFile < 3) {
		// A, B, C, D --> H, G, F, E
		king_defenders = (pos->position[BP] & (FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C] | FileMasks8[FILE_D]));

		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[Mirror64[sq]] : king_defence_eg[Mirror64[sq]];
		}
	}
	else if (kFile >= 3 && kFile < 5) {
		// C, D, E, F --> F, E, D, C
		king_defenders = (pos->position[BP] & (FileMasks8[FILE_C] | FileMasks8[FILE_D] | FileMasks8[FILE_E] | FileMasks8[FILE_F]));

		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[Mirror64[sq]] : king_defence_eg[Mirror64[sq]];
		}
	}
	else if (kFile >= 5 && kFile < 7) {
		// E, F, G, H --> A, B, C, D
		king_defenders = (pos->position[BP] & (FileMasks8[FILE_E] | FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]));

		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[Mirror64[sq]] : king_defence_eg[Mirror64[sq]];
		}
	}
	else {
		// F, G, H --> C, B, A
		king_defenders = (pos->position[BP] & (FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]));

		while (king_defenders != 0) {
			sq = PopBit(&king_defenders);

			v += (mg) ? king_defence_mg[Mirror64[sq]] : king_defence_eg[Mirror64[sq]];
		}
	}

	return v;
}