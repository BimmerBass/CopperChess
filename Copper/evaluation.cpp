#include "defs.h"
#include "psqt.h"

/*
INCLUDES THE FUNCTIONS:
	- staticEval(S_BOARD* board)
	- getMaterial(const S_BOARD* pos, bool side)
*/

const int passedPawnValue[8] = { 0, 5, 10, 25, 35, 60, 100, 140 };

const int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };

// Alpha and beta are added because lazy evaluation will be added in the future.
using namespace psqt;
int eval::staticEval(const S_BOARD* pos, int depth, int alpha, int beta) {
	int value = 0;
	int phase = 0;

	int mgScore = 0;
	int mgWeight = 0;

	int egScore = 0;
	int egWeight = 0;

	int wPawnCnt = countBits(pos->position[WP]);
	int bPawnCnt = countBits(pos->position[BP]);


	/*
	We'll first probe the evaluation hash table to see if this position has already been evaluated
	*/
	if (pos->evaluationCache->probeCache(pos, value)) {
		return value;
	}

	for (int sq = 0; sq < 64; sq++) {
		if (pos->pieceList[sq] == NO_PIECE) {
			continue;
		}

		else if (pos->pieceList[sq] == WP){
			mgScore += pawnValMg + PawnTableMg[sq];
			egScore += pawnValEg + PawnTableEg[sq];

			continue;
		}

		else if (pos->pieceList[sq] == WN) {
			phase += 1;

			mgScore += knightValMg + KnightTableMg[sq];
			egScore += knightValEg + KnightTableEg[sq];

			continue;
		}
		
		else if (pos->pieceList[sq] == WB) {
			phase += 1;

			mgScore += bishopValMg + BishopTableMg[sq];
			egScore += bishopValEg + BishopTableEg[sq];

			continue;
		}
		
		else if (pos->pieceList[sq] == WR) {
			phase += 2;

			mgScore += rookValMg + RookTableMg[sq];
			egScore += rookValEg + RookTableEg[sq];

			continue;
		}
		
		else if (pos->pieceList[sq] == WQ) {
			phase += 4;

			mgScore += queenValMg + QueenTableMg[sq];
			egScore += queenValEg + QueenTableEg[sq];

			continue;
		}
		
		else if (pos->pieceList[sq] == WK) {
			mgScore += kingValMg + KingTableMg[sq];
			egScore += kingValEg + KingTableEg[sq];

			continue;
		}


		else if (pos->pieceList[sq] == BP) {
			mgScore -= (pawnValMg + PawnTableMg[Mirror64[sq]]);
			egScore -= (pawnValEg + PawnTableEg[Mirror64[sq]]);

			continue;
		}
		
		else if (pos->pieceList[sq] == BN) {
			phase += 1;

			mgScore -= (knightValMg + KnightTableMg[Mirror64[sq]]);
			egScore -= (knightValEg + KnightTableEg[Mirror64[sq]]);

			continue;
		}
		
		else if (pos->pieceList[sq] == BB) {
			phase += 1;

			mgScore -= (bishopValMg + BishopTableMg[Mirror64[sq]]);
			egScore -= (bishopValEg + BishopTableEg[Mirror64[sq]]);

			continue;
		}
		
		else if (pos->pieceList[sq] == BR) {
			phase += 2;

			mgScore -= (rookValMg + RookTableMg[Mirror64[sq]]);
			egScore -= (rookValEg + RookTableEg[Mirror64[sq]]);

			continue;
		}
		
		else if (pos->pieceList[sq] == BQ) {
			phase += 4;

			mgScore -= (queenValMg + QueenTableMg[Mirror64[sq]]);
			egScore -= (queenValEg + QueenTableEg[Mirror64[sq]]);

			continue;
		}
		
		else if (pos->pieceList[sq] == BK) {
			mgScore -= (kingValMg + KingTableMg[Mirror64[sq]]);
			egScore -= (kingValEg + KingTableEg[Mirror64[sq]]);

			continue;
		}

	}

	// The phase increases with material meaning that the mgWeight should increase and the egWeight should decrease as the phase increases.
	if (phase > 24) { phase = 24; }
	mgWeight = phase;
	egWeight = 24 - phase;

	value = ((mgScore * mgWeight) + (egScore * egWeight)) / 24;

	if (pos->whitesMove == WHITE) {
		value += 18;
	}
	else {
		value -= 18;
	}

	// Add the evaluation to the evaluation has table for future lookup
	pos->evaluationCache->storeEvaluation(pos, value);

	return value;
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