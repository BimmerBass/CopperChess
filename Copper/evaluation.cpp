#include "defs.h"
#include "psqt.h"

/*
INCLUDES THE FUNCTIONS:
	- staticEval(S_BOARD* board)
	- getMaterial(const S_BOARD* pos, bool side)
*/
using namespace psqt;


int addPsqtVal(int sq, int pce, bool eg) {
	if (eg == true) {
		switch (pce) {
		case WP: return PawnTableEg[sq];
		case WN: return KnightTableEg[sq];
		case WB: return BishopTableEg[sq];
		case WR: return RookTableEg[sq];
		case WQ: return QueenTableEg[sq];
		case WK: return KingTableEg[sq];

		case BP: return -PawnTableEg[Mirror64[sq]];
		case BN: return -KnightTableEg[Mirror64[sq]];
		case BB: return -BishopTableEg[Mirror64[sq]];
		case BR: return -RookTableEg[Mirror64[sq]];
		case BQ: return -QueenTableEg[Mirror64[sq]];
		case BK: return -KingTableEg[Mirror64[sq]];
		}
	}

	else {
		switch (pce) {
		case WP: return PawnTableMg[sq];
		case WN: return KnightTableMg[sq];
		case WB: return BishopTableMg[sq];
		case WR: return RookTableMg[sq];
		case WQ: return QueenTableMg[sq];
		case WK: return KingTableMg[sq];

		case BP: return -PawnTableMg[Mirror64[sq]];
		case BN: return -KnightTableMg[Mirror64[sq]];
		case BB: return -BishopTableMg[Mirror64[sq]];
		case BR: return -RookTableMg[Mirror64[sq]];
		case BQ: return -QueenTableMg[Mirror64[sq]];
		case BK: return -KingTableMg[Mirror64[sq]];
		}
	}
}

const int passedPawnValue[8] = { 0, 5, 10, 25, 35, 60, 100, 140 };

const int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };

// Alpha and beta are added because lazy evaluation will be added in the future.
int eval::staticEval(const S_BOARD* pos, int depth, int alpha, int beta) {
	int v_main = 0;
	int v_mg = 0;
	int v_eg = 0;

	// Check to see if we get a hit from the evaluation cache.
	if (pos->evaluationCache->probeCache(pos, v_main) == true) {
		return v_main;
	}

	// Calculate the game-phase, the middlegame weight and the endgame weight
	int p = phase(pos);
	if (p > 24) { p = 24; }
	int weight_mg = p;
	int weight_eg = 24 - p;

	// Add material differences
	v_mg += material_mg(pos);
	v_eg += material_eg(pos);


	// Add psqt values
	v_mg += psqt_mg(pos);
	v_eg += psqt_eg(pos);

	v_main = ((v_mg * weight_mg) + (v_eg * weight_eg)) / 24;

	v_main += (pos->whitesMove == WHITE) ? 18 : -18;

	// Store the evaluation in the evaluation cache
	pos->evaluationCache->storeEvaluation(pos, v_main);

	return v_main;
}

int eval::material_mg(const S_BOARD* pos) {
	int v = 0;

	v += pawnValMg * (countBits(pos->position[WP]) - countBits(pos->position[BP]));
	v += knightValMg * (countBits(pos->position[WN]) - countBits(pos->position[BN]));
	v += bishopValMg * (countBits(pos->position[WB]) - countBits(pos->position[BB]));
	v += rookValMg * (countBits(pos->position[WR]) - countBits(pos->position[BR]));
	v += queenValMg * (countBits(pos->position[WQ]) - countBits(pos->position[BQ]));
	v += kingValMg * (countBits(pos->position[WK]) - countBits(pos->position[BK]));

	return v;
}

int eval::material_eg(const S_BOARD* pos) {
	int v = 0;

	v += pawnValEg * (countBits(pos->position[WP]) - countBits(pos->position[BP]));
	v += knightValEg * (countBits(pos->position[WN]) - countBits(pos->position[BN]));
	v += bishopValEg * (countBits(pos->position[WB]) - countBits(pos->position[BB]));
	v += rookValEg * (countBits(pos->position[WR]) - countBits(pos->position[BR]));
	v += queenValEg * (countBits(pos->position[WQ]) - countBits(pos->position[BQ]));
	v += kingValEg * (countBits(pos->position[WK]) - countBits(pos->position[BK]));

	return v;
}

void eval::material_both(const S_BOARD* pos, int& v_mg, int& v_eg) {
	int pawnDiff = countBits(pos->position[WP]) - countBits(pos->position[BP]);
	int knightDiff = countBits(pos->position[WN]) - countBits(pos->position[BN]);
	int bishopDiff = countBits(pos->position[WB]) - countBits(pos->position[BB]);
	int rookDiff = countBits(pos->position[WR]) - countBits(pos->position[BR]);
	int queenDiff = countBits(pos->position[WQ]) - countBits(pos->position[BQ]);
	int kingDiff = countBits(pos->position[WK]) - countBits(pos->position[BK]);

	v_eg += pawnValEg * pawnDiff + 
		knightValEg * knightDiff + 
		bishopValEg * bishopDiff + 
		rookValEg * rookDiff + 
		queenValEg * queenDiff + 
		kingValEg * kingDiff;
	v_mg += pawnValMg * pawnDiff +
		knightValMg * knightDiff +
		bishopValMg * bishopDiff +
		rookValMg * rookDiff +
		queenValMg * queenDiff +
		kingValMg * kingDiff;
}

int eval::psqt_mg(const S_BOARD* pos) {
	int v = 0;

	for (int pce = 0; pce < 12; pce++) {
		BitBoard pceBoard = pos->position[pce];

		while (pceBoard != 0) {
			v += addPsqtVal(PopBit(&pceBoard), pce, false);
		}
	}
	return v;
}

int eval::psqt_eg(const S_BOARD* pos) {
	int v = 0;

	for (int pce = 0; pce < 12; pce++) {
		BitBoard pceBoard = pos->position[pce];

		while (pceBoard != 0) {
			v += addPsqtVal(PopBit(&pceBoard), pce, true);
		}
	}
	return v;
}

int eval::phase(const S_BOARD* pos) {
	int p = 0;

	p += 1 * countBits(pos->position[WN] | pos->position[BN]);
	p += 1 * countBits(pos->position[WB] | pos->position[BB]);
	p += 2 * countBits(pos->position[WR] | pos->position[BR]);
	p += 4 * countBits(pos->position[WQ] | pos->position[BQ]);

	return p;
}


int eval::getMaterial(const S_BOARD* pos, bool side) {
	int material = 0;
	if (side == WHITE) {
		for (int pce = 0; pce < 5; pce++) { // We don't want to include the king here.
			material += (pieceValMg[pce]) * countBits(pos->position[pce]);
		}
	}
	else {
		for (int pce = 6; pce < 11; pce++) {
			material += ((pieceValMg[pce]) * countBits(pos->position[pce]));
		}
	}
	return material;
}
