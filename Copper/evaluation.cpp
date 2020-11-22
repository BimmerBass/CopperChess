#include "evaluation.h"


// This is the middlegame evaluation
int eval::mg_evaluate(const S_BOARD* pos, int alpha, int beta) {
	int v = 0;

	v += material_mg(pos);
	v += psqt_mg(pos);

	/*
	LAZY EVALUATION:
		- If our score after counting up material and psqt is already higher than beta by a certain margin or lower than alpha by the same
		  margin (a knight in the middlegame and a rook in the endgame), we are probably already doing good enough to just return the evaluation.
	*/

	if (LAZY_EVAL && (v - LAZYNESS_MG > beta || v + LAZYNESS_MG < alpha)){
		return v;
	}

	v += pawns_mg(pos);
	v += pieces_mg(pos);
	
	return v;
}

// This is the endgame evaluation
int eval::eg_evaluate(const S_BOARD* pos, int alpha, int beta) {
	int v = 0;

	if (material_draw(pos) == true) {
		return 0;
	}

	v += material_eg(pos);
	v += psqt_eg(pos);

	/*
	LAZY EVALUATION:
		- If our score after counting up material and psqt is already higher than beta by a certain margin or lower than alpha by the same
		  margin (a knight in the middlegame and a rook in the endgame), we are probably already doing good enough to just return the evaluation.
	*/

	if (LAZY_EVAL && (v - LAZYNESS_EG > beta || v + LAZYNESS_EG < alpha)) {
		return v;
	}

	v += pawns_eg(pos);
	v += pieces_eg(pos);

	return v * (scale_factor(pos, v) / 64);
}

inline bool opposite_bishops(const S_BOARD* pos, int* bc_w = nullptr, int* bc_b = nullptr) {
	if (bc_w == nullptr || bc_b == nullptr) {
		bc_w = new int(countBits(pos->position[WB]));
		bc_b = new int(countBits(pos->position[BB]));
	}

	if (*bc_w != 1) { return false; }
	if (*bc_b != 1) { return false; }

	if ((pos->position[WB] & DARK_SQUARES) != 0) { // If the white bishop is dark-squared
		if ((pos->position[BB] & DARK_SQUARES) == 0) { // The black bishop is light-squared
			return true;
		}
	}
	else { // The white bishop is light-squared
		if ((pos->position[BB] & DARK_SQUARES) != 0) { // The black bishop is dark_squared
			return true;
		}
	}

	return false;
}

int eval::scale_factor(const S_BOARD* pos, int eg_eval) {
	int sf = 64;

	S_SIDE stronger = (eg_eval > 0) ? WHITE : BLACK;

	int bc_w = countBits(pos->position[WB]);
	int bc_b = countBits(pos->position[BB]);
	
	int passedPawnsCnt = 0;
	int psq = NO_SQ;

	int npm = knightValMg * countBits(pos->position[WN]) + bishopValMg * bc_w +
		rookValMg * countBits(pos->position[WR]) + queenValMg * countBits(pos->position[WQ])
		+ knightValMg * countBits(pos->position[BN]) + bishopValMg * bc_b +
		rookValMg * countBits(pos->position[BR]) + queenValMg * countBits(pos->position[BQ]);

	if (opposite_bishops(pos, &bc_w, &bc_b) && npm == 2 * bishopValMg) {

		for (int f = 0; f < 8; f++) {
			if (countBits(pos->position[WP] & FileMasks8[f]) == 1) {
				psq = bitScanForward(pos->position[WP] & FileMasks8[f]);
				if ((whitePassedPawnMasks[psq] & pos->position[BP]) == 0) {
					passedPawnsCnt++;
				}
			}
			if (countBits(pos->position[BP] & FileMasks8[f]) == 1) {
				psq = bitScanForward(pos->position[BP] & FileMasks8[f]);
				if ((blackPassedPawnMasks[psq] & pos->position[WP]) == 0) {
					passedPawnsCnt++;
				}
			}
		}

		sf = 16 + 4 * passedPawnsCnt;
	}
	else {
		sf = std::min(40 + ((opposite_bishops(pos, &bc_w, &bc_b)) ? 2 : 7) * ((stronger == WHITE) ? countBits(pos->position[WP]) : countBits(pos->position[BP])), sf);
	}

	return sf;
}


int eval::pawns_mg(const S_BOARD* pos) {
	int v = 0;

	int index = NO_SQ;

	BitBoard whitePawns = pos->position[WP];
	BitBoard blackPawns = pos->position[BP];

	// Penalty for doubled pawns
	v -= doubled_penalty * (doubledCnt(whitePawns) - doubledCnt(blackPawns));


	// Give penalty for pieces blocking the E- and D- pawns
	if ((pos->position[WP] & ((FileMasks8[FILE_E] | FileMasks8[FILE_D]) & RankMasks8[RANK_2])) != 0) { // If there are pawns on e2 or d2
		v -= 7 * countBits((pos->WHITE_PIECES ^ pos->position[WP])
			& ((FileMasks8[FILE_E] | FileMasks8[FILE_D]) & (RankMasks8[RANK_3] | RankMasks8[RANK_4])));
	}

	if ((pos->position[BP] & ((FileMasks8[FILE_E] | FileMasks8[FILE_D]) & RankMasks8[RANK_7])) != 0) { // If there are pawns on e7 or d7
		v += 7 * countBits((pos->BLACK_PIECES ^ pos->position[BP])
			& ((FileMasks8[FILE_E] | FileMasks8[FILE_D]) & (RankMasks8[RANK_6] | RankMasks8[RANK_5])));
	}

	while (whitePawns != 0) {
		index = PopBit(&whitePawns);

		// Passed pawn bonus
		v += ((whitePassedPawnMasks[index] & pos->position[BP]) == 0) ? passedPawnValue[index / 8] : 0;

		// Doubled and isolated
		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[WP]) >= 2) ? true : false;
		bool isolated = (((pos->position[WP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v -= 11; }
		else if (isolated) { v -= 5; }

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


int eval::pieces_mg(const S_BOARD* pos) {
	int v = 0;

	BitBoard w_kingRing = kingRing(pos, WHITE);
	BitBoard b_kingRing = kingRing(pos, BLACK);

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

		// Add value if on an outpost. Only if the square is on the fifth rank or above.
		if (sq / 8 >= RANK_5) {
			v += outpost(pos, sq, WHITE);
		}

		// Add value if defended by pawns.
		v += 10 * defending_pawns(pos, sq, WHITE);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Add value for being on an outpost. Only for rank 5 or above.
		if (sq / 8 >= RANK_5) {
			v += outpost(pos, sq, WHITE);
		}

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v += 5 * defending_pawns(pos, sq, WHITE);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v -= 3 * countBits(pos->position[WP] & ~DARK_SQUARES);
		}
		else {
			v -= 3 * countBits(pos->position[WP] & DARK_SQUARES);
		}
		
		// Bonus for being on same diagonal or anti-diagonal as enemy king ring
		v += (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& b_kingRing) != 0) ? 25 : 0;
	}

	// Bonus for having doubled rooks
	for (int f = 0; f < 8; f++) {
		if (countBits(rookBrd & FileMasks8[f]) >= 2) {
			v += 30;
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Bonus for being on the same file as the enemy queen.
		v += ((FileMasks8[sq % 8] & pos->position[BQ]) != 0) ? 5 : 0;

		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = 45;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) == 0 && (pos->position[BP] & FileMasks8[sq % 8]) != 0) { // Only half-open file. No white pawns only
			fileBonus = 20;
		}
		else {
			fileBonus = 0;
		}
		v += fileBonus;

		// Bonus for being on the enemy king ring.
		v += (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & b_kingRing) != 0) ? 15 : 0;

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

		// Add value for being on an outpost. Only on fourth rank and below.
		if (sq / 8 <= RANK_4) {
			v -= outpost(pos, sq, BLACK);
		}
		// Add value depending on amount of defending pawns of the square
		v -= 10 * defending_pawns(pos, sq, BLACK);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Outpost bonus. Only if on rank four or below.
		if (sq / 8 <= RANK_4) {
			v -= outpost(pos, sq, BLACK);
		}

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v -= 5 * defending_pawns(pos, sq, BLACK);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v += 3 * countBits(pos->position[BP] & ~DARK_SQUARES);
		}
		else {
			v += 3 * countBits(pos->position[BP] & DARK_SQUARES);
		}

		// Bonus for being on same diagonal or anti-diagonal as enemy king ring
		v -= (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& w_kingRing) != 0) ? 25 : 0;
	}

	// Bonus for having doubled rooks
	for (int f = 0; f < 8; f++) {
		if (countBits(rookBrd & FileMasks8[f]) >= 2) {
			v -= 30;
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Bonus for same file as enemy queen.
		v -= ((FileMasks8[sq % 8] & pos->position[WQ]) != 0) ? 5 : 0;

		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // No pawns => fully open file.
			fileBonus = 45;
		}
		if ((pos->position[BP] & FileMasks8[sq % 8]) == 0 && (pos->position[WP] & FileMasks8[sq % 8]) != 0) { // There are only white pawns => semi-open
			fileBonus = 20;
		}

		v -= fileBonus;

		// Bonus for being on the enemy king ring.
		v -= (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & w_kingRing) != 0) ? 15 : 0;
	}


	return v;
}


int eval::pieces_eg(const S_BOARD* pos) {
	int v = 0;

	BitBoard passedPawns = 0;
	int psq = NO_SQ;

	BitBoard w_kingRing = kingRing(pos, WHITE);
	BitBoard b_kingRing = kingRing(pos, BLACK);

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
		
		// Add value for being on an outpost. This is smaller than in the middlegame.
		if (sq / 8 >= RANK_5) {
			v += outpost(pos, sq, WHITE) / 2;
		}
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Give bonus if there are pawns on both sides of the board
		v += (((FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]) & (pos->position[WP] | pos->position[BP])) != 0 &&
			((FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]) & (pos->position[WP] | pos->position[BP])) != 0) ? 40 : 0;

		// Give penalty proportional to the amount of pawns on this bishops colour
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) != 0) {
			v -= 7 * countBits(DARK_SQUARES & pos->position[WP]);
		}
		else {
			v -= 7 * countBits(~DARK_SQUARES & pos->position[WP]);
		}

		// Give bonus proportional to amount of enemy pawns on the bishops diagonals.
		v += 5 * countBits((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)]) & pos->position[BP]);
	}

	for (int f = 0; f < 8; f++) {
		if (countBits(pos->position[WP] & FileMasks8[f]) == 1) { // If there is 1 pawn on this file.
			psq = bitScanForward(pos->position[WP] & FileMasks8[f]);
			if ((whitePassedPawnMasks[psq] & pos->position[BP]) == 0 && (whiteRookSupport[psq] & pos->BLACK_PIECES) == 0) {
				// It is a passed pawn, and there are no enemy pieces behind it.
				passedPawns |= (uint64_t)1 << psq;
				continue;
			}
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Give bonus if we are defending a passed pawn.
		if ((FileMasks8[sq % 8] & passedPawns) != 0) {
			v += 50;
		}

		// Give bonus for eyeing the black king-ring
		v += (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & b_kingRing) != 0) ? 7 : 0;

		// Bonus for being on the same file as the black queen
		v += ((FileMasks8[sq % 8] & pos->position[BQ]) != 0) ? 11 : 0;


		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = 29;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) == 0 && (pos->position[BP] & FileMasks8[sq % 8]) != 0) { // Only half-open file. No white pawns only
			fileBonus = 7;
		}
		else {
			fileBonus = 0;
		}
		v += fileBonus;
	}

	while (queenBrd != 0) {
		sq = PopBit(&queenBrd);

		// Bonus for being behind passed pawns.
		if ((FileMasks8[sq % 8] & passedPawns) != 0) {
			v += 20;
		}

		// Give bonus inversely proportional to the manhattan distance to the black king
		v += 2 * ManhattanDistance[sq][pos->kingPos[1]];
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

		// Bonus for being on an outpost.
		if (sq / 8 <= RANK_4) {
			v -= outpost(pos, sq, BLACK) / 2;
		}
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Give bonus if there are pawns on both sides of the board
		v -= (((FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]) & (pos->position[WP] | pos->position[BP])) != 0 &&
			((FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]) & (pos->position[WP] | pos->position[BP])) != 0) ? 40 : 0;

		// Give penalty proportional to the amount of pawns on this bishops colour
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) != 0) {
			v += 7 * countBits(DARK_SQUARES & pos->position[BP]);
		}
		else {
			v += 7 * countBits(~DARK_SQUARES & pos->position[BP]);
		}

		// Give bonus proportional to amount of enemy pawns on the bishops diagonals.
		v -= 5 * countBits((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)]) & pos->position[WP]);
	}

	passedPawns = 0;
	for (int f = 0; f < 8; f++) {
		if (countBits(pos->position[BP] & FileMasks8[f]) == 1) {
			psq = bitScanForward(pos->position[BP] & FileMasks8[f]);
			if ((blackPassedPawnMasks[psq] & pos->position[WP]) == 0 && (blackRookSupport[psq] & pos->WHITE_PIECES) == 0){
				passedPawns |= (uint64_t)1 << psq;
				continue;
			}
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Give bonus if we are defending a passed pawn.
		if ((FileMasks8[sq % 8] & passedPawns) != 0) {
			v -= 50;
		}

		// Give bonus for eyeing the white king-ring
		v -= (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & w_kingRing) != 0) ? 7 : 0;

		// Bonus for being on the same file as the white queen
		v -= ((FileMasks8[sq % 8] & pos->position[WQ]) != 0) ? 11 : 0;


		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = 29;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) != 0 && (pos->position[BP] & FileMasks8[sq % 8]) == 0) { // Only half-open file. No white pawns only
			fileBonus = 7;
		}
		else {
			fileBonus = 0;
		}
		v -= fileBonus;
	}

	while (queenBrd != 0) {
		sq = PopBit(&queenBrd);

		if ((FileMasks8[sq % 8] & passedPawns) != 0) {
			v -= 20;
		}

		// Give bonus inversely proportional to the manhattan distance to the black king
		v -= 2 * ManhattanDistance[sq][pos->kingPos[0]];
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

	// Penalty to knights depending on amount of pawns removed
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

// Will return true for positions where a checkmate cannot be forced in any way.
bool eval::material_draw(const S_BOARD* pos) {
	if (pos->position[WR] == 0 && pos->position[BR] == 0 && pos->position[WQ] == 0 && pos->position[BQ] == 0) {
		if (pos->position[BB] == 0 && pos->position[WB] == 0) {
			if (countBits(pos->position[WN]) < 3 && countBits(pos->position[BN]) < 3) { return true; }
		}

		else if (pos->position[WN] == 0 && pos->position[BN] == 0) {
			if (abs(countBits(pos->position[WB]) - countBits(pos->position[BB])) < 2) { return true; }
		}

		else if ((countBits(pos->position[WN]) < 3 && pos->position[WB] == 0) || (countBits(pos->position[WB]) == 1 && pos->position[WN] == 0)) {
			if ((countBits(pos->position[BN]) < 3 && pos->position[BB] == 0) || (countBits(pos->position[BB]) == 1 && pos->position[BN] == 0)) { 
				return true; 
			}
		}
	}

	else if (pos->position[WQ] == 0 && pos->position[BQ] == 0) {
		if (countBits(pos->position[WR]) == 1 && countBits(pos->position[BR]) == 1) {
			if (countBits(pos->position[WN] | pos->position[WB]) < 2 && countBits(pos->position[BN] | pos->position[BB]) < 2) { return true; }
		}
		else if (countBits(pos->position[WR]) == 1 && pos->position[BR] == 0) {
			if ((pos->position[WN] | pos->position[WB]) == 0 && 
				(countBits(pos->position[BN] | pos->position[BB]) == 1 || countBits(pos->position[BN] | pos->position[BB]) == 2)) {
				return true;
			}
		}
		else if (countBits(pos->position[BR]) == 1 && pos->position[WR] == 0) {
			if ((pos->position[BN] | pos->position[BB]) == 0 &&
				(countBits(pos->position[WN] | pos->position[WB]) == 1 || countBits(pos->position[WN] | pos->position[WB]) == 2)) {
				return true;
			}
		}
	}
	return false;
}