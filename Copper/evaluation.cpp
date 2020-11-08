#include "evaluation.h"


// This is the middlegame evaluation
int eval::mg_evaluate(const S_BOARD* pos) {
	int v = 0;

	v += material_mg(pos);
	v += psqt_mg(pos);
	
	return v;
}

// This is the endgame evaluation
int eval::eg_evaluate(const S_BOARD* pos) {
	int v = 0;

	v += material_eg(pos);
	v += psqt_eg(pos);

	return v;
}

int eval::imbalance(const S_BOARD* pos) {
	int v = 0;

	// Give bonus for bishop pair
	v += (countBits(pos->position[WB]) >= 2) ? bishop_pair : 0;
	v -= (countBits(pos->position[BB]) >= 2) ? bishop_pair : 0;

	// Penalty for knight pair
	v -= (countBits(pos->position[WN]) >= 2) ? p_knight_pair : 0;
	v += (countBits(pos->position[BN]) >= 2) ? p_knight_pair : 0;

	// Penalty for rook pair
	v -= (countBits(pos->position[WR]) >= 2) ? p_rook_pair : 0;
	v += (countBits(pos->position[BR]) >= 2) ? p_rook_pair : 0;

	// Bonus to rooks depending on amount of pawns removed
	int wPwnCnt = countBits(pos->position[WP]);
	int bPwnCnt = countBits(pos->position[BP]);
	v += (pos->position[WR] != 0) ? rook_pawn_bonus * (8 - wPwnCnt) : 0;
	v -= (pos->position[BR] != 0) ? rook_pawn_bonus * (8 - bPwnCnt) : 0;

	v -= (pos->position[WN] != 0) ? knight_pawn_penalty * (8 - wPwnCnt) : 0;
	v += (pos->position[BN] != 0) ? knight_pawn_penalty * (8 - bPwnCnt) : 0;

	return v;
}

// Compute the material difference in the middlegame
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

// Compute the material difference in the endgame
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

// Add the piece-square tables for the middlegame
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

// Add the piece-square tables for the endgame
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

// The phase is a way of finding the amount of non-pawn-material on the board. It is used for a tapered eval, such that the endgame
//		evaluation becomes more dominant as material is removed from the board.
int eval::phase(const S_BOARD* pos) {
	int p = 0;

	p += 1 * countBits(pos->position[WN] | pos->position[BN]);
	p += 1 * countBits(pos->position[WB] | pos->position[BB]);
	p += 2 * countBits(pos->position[WR] | pos->position[BR]);
	p += 4 * countBits(pos->position[WQ] | pos->position[BQ]);

	return p;
}

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