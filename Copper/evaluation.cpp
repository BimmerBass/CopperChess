#include "evaluation.h"


// This is the middlegame evaluation
int eval::mg_evaluate(const S_BOARD* pos) {
	int v = 0;

	v += material_mg(pos);
	v += psqt_mg(pos);
	v += pawns_mg(pos);
	//v += pieces_mg(pos);
	
	return v;
}

// This is the endgame evaluation
int eval::eg_evaluate(const S_BOARD* pos) {
	int v = 0;

	v += material_eg(pos);
	v += psqt_eg(pos);
	v += pawns_eg(pos);

	return v;
}
/*
int pawns_mg(const S_BOARD* pos);
int pawns_eg(const S_BOARD* pos);*/
int eval::pawns_mg(const S_BOARD* pos) {
	int v = 0;

	int index = NO_SQ;

	BitBoard whitePawns = pos->position[WP];
	BitBoard blackPawns = pos->position[BP];

	// Penalty for doubled pawns
	v -= doubled_penalty * (doubledCnt(whitePawns) - doubledCnt(blackPawns));

	while (whitePawns != 0) {
		index = PopBit(&whitePawns);

		// Passed pawn bonus
		v += ((whitePassedPawnMasks[index] & pos->position[BP]) == 0) ? passedPawnValue[index / 8] : 0;

		// Doubled and isolated
		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[WP]) >= 2) ? true : false;
		bool isolated = (((pos->position[WP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v -= 11; }
		else if (isolated) { v -= 5; }

		// Penalty if the pawn is blocked by a friendly piece
		/*if ((((pos->WHITE_PIECES ^ pos->position[WP]) >> (index + 8)) & 1) == 1) {
			v += mg_value(psqt::blockedPawnTable[index + 8]);
		}*/

		// Bonus for being supported by another pawn
		int supportPawns = defending_pawns(pos, index, WHITE);
		v += 5 * countBits(supportPawns);
	}

	while (blackPawns != 0) {
		index = PopBit(&blackPawns);

		v -= ((blackPassedPawnMasks[index] & pos->position[WP]) == 0) ? passedPawnValue[mirrorRankNum[index / 8]] : 0;

		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[BP]) >= 2) ? true : false;
		bool isolated = (((pos->position[BP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v += 11; }
		else if (isolated) { v += 5; }

		// Penalty if the pawn is blocked by a friendly piece
		/*if ((((pos->BLACK_PIECES ^ pos->position[BP]) >> (index - 8)) & 1) == 1) {
			v -= mg_value(psqt::blockedPawnTable[psqt::Mirror64[index - 8]]);
		}*/

		// Bonus for being supported
		int supportPawns = defending_pawns(pos, index, BLACK);
		v -= 5 * countBits(supportPawns);
	}

	return v;
}


int eval::pawns_eg(const S_BOARD* pos) {
	int v = 0;

	int index = NO_SQ;

	BitBoard whitePawns = pos->position[WP];
	BitBoard blackPawns = pos->position[BP];

	// Penalty for doubled pawns
	v -= 56 * (doubledCnt(whitePawns) - doubledCnt(blackPawns));

	while (whitePawns != 0) {
		index = PopBit(&whitePawns);

		// Passed pawn bonus
		v += ((whitePassedPawnMasks[index] & pos->position[BP]) == 0) ? passedPawnValue[index / 8] : 0;

		// Doubled and isolated
		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[WP]) >= 2) ? true : false;
		bool isolated = (((pos->position[WP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v -= 56; }
		else if (isolated) { v -= 15; }

		// Penalty if the pawn is blocked by a friendly piece
		/*if ((((pos->WHITE_PIECES ^ pos->position[WP]) >> (index + 8)) & 1) == 1) {
			v += eg_value(psqt::blockedPawnTable[index + 8]);
		}*/

		// Bonus for being supported by another pawn
		int supportPawns = defending_pawns(pos, index, WHITE);
		v += 10 * countBits(supportPawns);
	}

	while (blackPawns != 0) {
		index = PopBit(&blackPawns);

		v -= ((blackPassedPawnMasks[index] & pos->position[WP]) == 0) ? passedPawnValue[mirrorRankNum[index / 8]] : 0;

		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[BP]) >= 2) ? true : false;
		bool isolated = (((pos->position[BP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v += 56; }
		else if (isolated) { v += 15; }

		// Penalty if the pawn is blocked by a friendly piece
		/*if ((((pos->BLACK_PIECES ^ pos->position[BP]) >> (index - 8)) & 1) == 1) {
			v -= eg_value(psqt::blockedPawnTable[psqt::Mirror64[index - 8]]);
		}*/

		// Bonus for being supported
		int supportPawns = defending_pawns(pos, index, BLACK);
		v -= 10 * countBits(supportPawns);
	}

	return v;
}

int eval::outpost(const S_BOARD* pos, int sq, S_SIDE side) {
	int v = 0;
	if (side == WHITE) {
		if (sq >= 56 && sq <= 63) { return 0; } // The 8'th rank is never a good outpost

		if (((SETBIT((uint64_t)0, sq + 9) | SETBIT((uint64_t)0, sq + 7)) & pos->position[BP]) != 0) { // If the square is attacked, it cant be an outpost
			return 0;
		}
		else {
			v = ((whiteOutpostMasks[sq] & pos->position[BP]) == 0) ? safe_outpost_bonus : outpost_bonus;

			// Return double value if the outpost is a knight, as they're usually more valuable on outposts than bishops.
			return (pos->pieceList[sq] == WN) ? 2 * v : v;
		}

	}
	else {
		if (sq >= 0 && sq <= 7) { return 0; }

		if (((SETBIT((uint64_t)0, sq - 9) | SETBIT((uint64_t)0, sq - 7)) & pos->position[WP]) != 0) {
			return 0;
		}
		else {
			v = ((blackOutpostMasks[sq] & pos->position[WP]) == 0) ? safe_outpost_bonus : outpost_bonus;

			// Return double value if the outpost is a knight.
			return (pos->pieceList[sq] == BN) ? 2 * v : v;
		}
	}
	return 0;
}

/*
	int pieces_mg(const S_BOARD* pos);
	int pieces_eg(const S_BOARD* pos);
*/


int eval::pieces_mg(const S_BOARD* pos) {
	int v = 0;

	/*
	WHITE PIECES
	*/

	BitBoard knightBrd = pos->position[WN];
	BitBoard bishopBrd = pos->position[WB];
	BitBoard rookBrd = pos->position[WR];
	BitBoard queenBrd = pos->position[WQ];
	int sq = NO_SQ;

	while (knightBrd != 0) {
		sq = PopBit(&knightBrd);

		// Add value if on an outpost
		v += outpost(pos, sq, WHITE);

		// Add value if defended by pawns.
		v += 10 * defending_pawns(pos, sq, WHITE);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Add value for being on an outpost
		v += outpost(pos, sq, WHITE);

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v += 5 * defending_pawns(pos, sq, WHITE);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v -= 3 * countBits(pos->position[WP] & ~DARK_SQUARES);
		}
		else {
			v -= 3 * countBits(pos->position[WP] & DARK_SQUARES);
		}
		
		// Bonus for being on same diagonal or anti-diagonal as enemy king
		v += (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& ((uint64_t)1 << pos->kingPos[1])) != 0) ? 46 : 0;
	}


	/*
	BLACK PIECES
	*/
	knightBrd = pos->position[BN];
	bishopBrd = pos->position[BB];
	rookBrd = pos->position[BR];
	queenBrd = pos->position[BQ];

	while (knightBrd != 0) {
		sq = PopBit(&knightBrd);

		// Add value for being on an outpost
		v -= outpost(pos, sq, BLACK);

		// Add value depending on amount of defending pawns of the square
		v -= 10 * defending_pawns(pos, sq, BLACK);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Outpost bonus
		v -= outpost(pos, sq, BLACK);

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v -= 5 * defending_pawns(pos, sq, BLACK);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v += 3 * countBits(pos->position[BP] & ~DARK_SQUARES);
		}
		else {
			v += 3 * countBits(pos->position[BP] & DARK_SQUARES);
		}

		// Bonus for being on same diagonal or anti-diagonal as enemy king
		v -= (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& ((uint64_t)1 << pos->kingPos[0])) != 0) ? 46 : 0;
	}


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

	v += countBits(pos->position[WR]) * rook_pawn_bonus * (8 - wPwnCnt);
	v -= countBits(pos->position[BR]) * rook_pawn_bonus * (8 - bPwnCnt);

	v -= countBits(pos->position[WN]) * knight_pawn_penalty * (8 - wPwnCnt);
	v += countBits(pos->position[BN]) * knight_pawn_penalty * (8 - bPwnCnt);

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
			int sq = PopBit(&pceBoard);
			v += addPsqtVal(sq, pce, false);
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

int defending_pawns(const S_BOARD* pos, int sq, S_SIDE side) {
	int cnt = 0;
	if (side == WHITE) {
		if (!(sq >= 0 && sq <= 7)) {
			if ((SETBIT((uint64_t)0, sq) & FileMasks8[FILE_A]) == 0) { // Not on a-file
				cnt += ((pos->position[WP] & SETBIT((uint64_t)0, sq - 9)) != 0) ? 1 : 0;
			}
			if ((SETBIT((uint64_t)0, sq) & FileMasks8[FILE_H]) == 0) {
				cnt += ((pos->position[WP] & SETBIT((uint64_t)0, sq - 7)) != 0) ? 1 : 0;
			}
		}
	}
	else {
		if (!(sq >= 56 && sq <= 63)) {
			if ((SETBIT((uint64_t)0, sq) & FileMasks8[FILE_A]) == 0) {
				cnt += ((pos->position[BP] & SETBIT((uint64_t)0, sq + 7)) != 0) ? 1 : 0;
			}

			if ((SETBIT((uint64_t)0, sq) & FileMasks8[FILE_H]) == 0) {
				cnt += ((pos->position[BP] & SETBIT((uint64_t)0, sq + 9)) != 0) ? 1 : 0;
			}
		}
	}
	return cnt;
}