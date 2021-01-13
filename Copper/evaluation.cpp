#include "evaluation.h"



int knight_mob_mg = 3;
int knight_mob_eg = 5;

int bishop_mob_mg = 4;
int bishop_mob_eg = 6;

int rook_mob_mg = 2;
int rook_mob_eg = 9;

int queen_mob_mg = 1;
int queen_mob_eg = 10;


/*
int psqt::PawnTableMg[64] = {
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
		10	,	10	,	10	,	-20	,	-20	,	0	,	10	,	10	,
		5	,	0	,	0	,	-5	,	-5	,	0	,	0	,	5	,
		-3	,	-5	,	-5	,	20	,	20	,	5	,	0	,	0	,
		-6	,	-10	,	-5	,	7	,	7	,	0	,	0	,	0	,
		0	,	-5	,	5	,	2	,	2	,	0	,	0	,	0	,
		10	,	10	,	15	,	20	,	20	,	15	,	10	,	10	,
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};
*/

int psqt::PawnTableMg[64] = {
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
		10	,	10	,	0	,	-20	,	-20	,	0	,	10	,	10	,
		5	,	0	,	0	,	-5	,	-5	,	0	,	0	,	5	,
		0	,	0	,	5	,	20	,	20	,	5	,	0	,	0	,
		0	,	0	,	0	,	7	,	7	,	0	,	0	,	0	,
		0	,	0	,	0	,	2	,	2	,	0	,	0	,	0	,
		10	,	10	,	15	,	20	,	20	,	15	,	10	,	10	,
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
};


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
	v += mobility_mg(pos);
	
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
	v += mobility_eg(pos);

	return v * (scale_factor(pos, v) / 64);
}







int pawnless_flank(const S_BOARD* pos, bool side) {
	int king_file = NO_FILE;

	if (side == WHITE) {
		king_file = pos->kingPos[0] % 8;
	}
	else {
		king_file = pos->kingPos[1] % 8;
	}

	int pawn_count_file[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	// We'll count the total number of pawns on each file.
	for (int f = 0; f < 8; f++) {
		pawn_count_file[f] = countBits((pos->position[WP] | pos->position[BP]) & FileMasks8[f]);
	}


	int sum; // The amount of pawns on the flank, that the king occupies.

	if (king_file == 0) { sum = pawn_count_file[FILE_A] + pawn_count_file[FILE_B] + pawn_count_file[FILE_C]; }
	else if (king_file >= 0 && king_file < 3) { sum = pawn_count_file[FILE_A]
		+ pawn_count_file[FILE_B] + pawn_count_file[FILE_C] + pawn_count_file[FILE_D];
	}
	else if (king_file >= 3 && king_file < 5) { sum = pawn_count_file[FILE_C] + 
		pawn_count_file[FILE_D] + pawn_count_file[FILE_E] + pawn_count_file[FILE_F]; 
	}
	else if (king_file >= 5 && king_file < 7) { sum = pawn_count_file[FILE_E] + pawn_count_file[FILE_F] + 
		pawn_count_file[FILE_G] + pawn_count_file[FILE_H]; 
	}
	else {
		sum = pawn_count_file[FILE_F] + pawn_count_file[FILE_G] + pawn_count_file[FILE_H];
	}


	return (sum == 0) ? 1 : 0;
}


int eval::king_mg(const S_BOARD* pos) {
	int v = 0;

	// If white has castled
	if (pos->has_castled[0]) {
		v += castling_bonus;

		// In the middlegame, the king receives a penalty for being on a pawnless flank as it is quite weak there.
		v -= 30 * pawnless_flank(pos, WHITE);
	}

	// If black has castled
	if (pos->has_castled[1]) {
		v -= castling_bonus;

		// In the middlegame, the king receives a penalty for being on a pawnless flank as it is quite weak there.
		v += 30 * pawnless_flank(pos, BLACK);
	}

	return v;
}

int eval::king_eg(const S_BOARD* pos) {
	int v = 0;

	// In the endgame, we don't want our king on a pawnless flank as it is needed to help promote.
	v -= 18 * pawnless_flank(pos, WHITE);
	v += 18 * pawnless_flank(pos, BLACK);


	return v;
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

	// Probe the middlegame pawn-hash table.
#ifndef TUNE
	if (pos->pawn_table_mg->probe_pawn_hash(pos, &v) == true) {
		return v;
	}
#endif

	int index = NO_SQ;

	BitBoard whitePawns = pos->position[WP];
	BitBoard blackPawns = pos->position[BP];

	// Penalty for doubled pawns
	v -= doubled_penalty_mg * (doubledCnt(whitePawns) - doubledCnt(blackPawns));


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

		if (doubled && isolated) { v -= DoubledIsolatedMg; }
		else if (isolated) { v -= isolatedMg; }

		// Bonus for being supported by another pawn
		int supportPawns = defending_pawns(pos, index, WHITE);
		v += supported_mg * countBits(supportPawns);
	}

	while (blackPawns != 0) {
		index = PopBit(&blackPawns);

		v -= ((blackPassedPawnMasks[index] & pos->position[WP]) == 0) ? passedPawnValue[mirrorRankNum[index / 8]] : 0;

		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[BP]) >= 2) ? true : false;
		bool isolated = (((pos->position[BP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v += DoubledIsolatedMg; }
		else if (isolated) { v += isolatedMg; }

		// Bonus for being supported
		int supportPawns = defending_pawns(pos, index, BLACK);
		v -= supported_mg * countBits(supportPawns);
	}

	// Store the pawn structure evaluation in the pawn hash table
#ifndef TUNE
	pos->pawn_table_mg->store_pawn_eval(pos, &v);
#endif

	return v;
}


int eval::pawns_eg(const S_BOARD* pos) {
	int v = 0;

	// Probe the endgame pawn hash table
#ifndef TUNE
	if (pos->pawn_table_eg->probe_pawn_hash(pos, &v) == true) {
		return v;
	}
#endif

	int index = NO_SQ;

	BitBoard whitePawns = pos->position[WP];
	BitBoard blackPawns = pos->position[BP];

	// Penalty for doubled pawns
	v -= doubled_penalty_eg * (doubledCnt(whitePawns) - doubledCnt(blackPawns));

	while (whitePawns != 0) {
		index = PopBit(&whitePawns);

		// Passed pawn bonus
		v += ((whitePassedPawnMasks[index] & pos->position[BP]) == 0) ? passedPawnValue[index / 8] : 0;

		// Doubled and isolated
		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[WP]) >= 2) ? true : false;
		bool isolated = (((pos->position[WP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v -= DoubledIsolatedEg; }
		else if (isolated) { v -= isolatedEg; }

		// Bonus for being supported by another pawn
		int supportPawns = defending_pawns(pos, index, WHITE);
		v += supported_eg * countBits(supportPawns);
	}

	while (blackPawns != 0) {
		index = PopBit(&blackPawns);

		v -= ((blackPassedPawnMasks[index] & pos->position[WP]) == 0) ? passedPawnValue[mirrorRankNum[index / 8]] : 0;

		bool doubled = (countBits(FileMasks8[index % 8] & pos->position[BP]) >= 2) ? true : false;
		bool isolated = (((pos->position[BP] ^ ((uint64_t)1 << index)) & isolatedPawnMasks[index % 8]) == 0) ? true : false;

		if (doubled && isolated) { v += DoubledIsolatedEg; }
		else if (isolated) { v += isolatedEg; }

		// Bonus for being supported
		int supportPawns = defending_pawns(pos, index, BLACK);
		v -= supported_eg * countBits(supportPawns);
	}

	// Store the pawn-evaluation in the endgame pawn-hash-table
#ifndef TUNE
	pos->pawn_table_eg->store_pawn_eval(pos, &v);
#endif

	return v;
}






int eval::outpost(const S_BOARD* pos, int sq, S_SIDE side, bool middlegame) {
	int v = 0;
	if (middlegame) {
		if (side == WHITE) {
			if (sq >= 56 && sq <= 63) { return 0; } // The 8'th rank is never a good outpost

			if (((((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_A]) << 7) | ((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_H]) << 9))
				& pos->position[BP]) != 0) { // If the square is attacked, it cant be an outpost
				return 0;
			}
			else {
				// Warning of buffer overflow here, but not a problem. sq is ensured to be between 0 and 63 when outpost() is called.
				v = ((whiteOutpostMasks[sq] & pos->position[BP]) == 0) ? safe_outpost_bonus_mg : outpost_bonus_mg;

				// Return bigger value if the outpost is a knight, as they're usually more valuable on outposts than bishops.
				return v;
			}

		}
		else {
			if (sq >= 0 && sq <= 7) { return 0; }

			if (((((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_A]) >> 9) | ((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_H]) >> 7))
				& pos->position[WP]) != 0) {
				return 0;
			}
			else {
				v = ((blackOutpostMasks[sq] & pos->position[WP]) == 0) ? safe_outpost_bonus_mg : outpost_bonus_mg;

				return v;
			}
		}
	}

	// If we're in the endgame, we'll have to use the eg values.
	else {
		if (side == WHITE) {
			if (sq >= 56 && sq <= 63) { return 0; } // The 8'th rank is never a good outpost

			if (((((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_A]) << 7) | ((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_H]) << 9))
				& pos->position[BP]) != 0) { // If the square is attacked, it cant be an outpost
				return 0;
			}
			else {
				// Warning of buffer overflow here, but not a problem. sq is ensured to be between 0 and 63 when outpost() is called.
				v = ((whiteOutpostMasks[sq] & pos->position[BP]) == 0) ? safe_outpost_bonus_eg : outpost_bonus_eg;

				return v;
			}

		}
		else {
			if (sq >= 0 && sq <= 7) { return 0; }

			if (((((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_A]) >> 9) | ((SETBIT((uint64_t)0, sq) & ~FileMasks8[FILE_H]) >> 7))
				& pos->position[WP]) != 0) {
				return 0;
			}
			else {
				v = ((blackOutpostMasks[sq] & pos->position[WP]) == 0) ? safe_outpost_bonus_eg : outpost_bonus_eg;

				return v;
			}
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
			v += knight_outpost_bonus_mg * outpost(pos, sq, WHITE, true);
		}

		// Add value if defended by pawns.
		v += N_pawn_defence_mg * defending_pawns(pos, sq, WHITE);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Add value for being on an outpost. Only for rank 5 or above.
		if (sq / 8 >= RANK_5) {
			assert(pos->pieceList[sq] == WB);
			v += outpost(pos, sq, WHITE, true);
		}

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v += B_pawn_defence_mg * defending_pawns(pos, sq, WHITE);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v -= PawnOn_bCol_mg * countBits(pos->position[WP] & ~DARK_SQUARES);
		}
		else {
			v -= PawnOn_bCol_mg * countBits(pos->position[WP] & DARK_SQUARES);
		}
		
		// Bonus for being on same diagonal or anti-diagonal as enemy king ring
		v += (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& b_kingRing) != 0) ? bishop_kingring_mg: 0;
	}

	// Bonus for having doubled rooks
	for (int f = 0; f < 8; f++) {
		if (countBits(rookBrd & FileMasks8[f]) >= 2) {
			v += doubled_rooks_mg;
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Bonus for being on the same file as the enemy queen.
		v += ((FileMasks8[sq % 8] & pos->position[BQ]) != 0) ? rook_on_queen_mg : 0;

		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = open_rook_mg;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) == 0 && (pos->position[BP] & FileMasks8[sq % 8]) != 0) { // Only half-open file. No white pawns only
			fileBonus = semi_rook_mg;
		}
		else {
			fileBonus = 0;
		}
		v += fileBonus;

		// Bonus for being on the enemy king ring.
		v += (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & b_kingRing) != 0) ? rook_kingring_mg : 0;

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
			v -= knight_outpost_bonus_mg * outpost(pos, sq, BLACK, true);
		}
		// Add value depending on amount of defending pawns of the square
		v -= N_pawn_defence_mg * defending_pawns(pos, sq, BLACK);
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Outpost bonus. Only if on rank four or below.
		if (sq / 8 <= RANK_4) {
			assert(pos->pieceList[sq] == BB);
			v -= outpost(pos, sq, BLACK, true);
		}

		// Add value for being defended by pawns. (Smaller than the bonus for knights)
		v -= B_pawn_defence_mg * defending_pawns(pos, sq, BLACK);

		// Penalty for amount of pawns of our own color on the bishops square-color
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) == 0) { // Light square bishop
			v += PawnOn_bCol_mg * countBits(pos->position[BP] & ~DARK_SQUARES);
		}
		else {
			v += PawnOn_bCol_mg * countBits(pos->position[BP] & DARK_SQUARES);
		}

		// Bonus for being on same diagonal or anti-diagonal as enemy king ring
		v -= (((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)])
			& w_kingRing) != 0) ? bishop_kingring_mg : 0;
	}

	// Bonus for having doubled rooks
	for (int f = 0; f < 8; f++) {
		if (countBits(rookBrd & FileMasks8[f]) >= 2) {
			v -= doubled_rooks_mg;
		}
	}

	while (rookBrd != 0) {
		sq = PopBit(&rookBrd);

		// Bonus for same file as enemy queen.
		v -= ((FileMasks8[sq % 8] & pos->position[WQ]) != 0) ? rook_on_queen_mg : 0;

		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // No pawns => fully open file.
			fileBonus = open_rook_mg;
		}
		if ((pos->position[BP] & FileMasks8[sq % 8]) == 0 && (pos->position[WP] & FileMasks8[sq % 8]) != 0) { // There are only white pawns => semi-open
			fileBonus = semi_rook_mg;
		}

		v -= fileBonus;

		// Bonus for being on the enemy king ring.
		v -= (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & w_kingRing) != 0) ? rook_kingring_mg : 0;
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
			v += knight_outpost_bonus_eg * outpost(pos, sq, WHITE, false);
		}
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Give bonus if there are pawns on both sides of the board
		v += (((FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]) & (pos->position[WP] | pos->position[BP])) != 0 &&
			((FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]) & (pos->position[WP] | pos->position[BP])) != 0) ? 40 : 0;

		// Give penalty proportional to the amount of pawns on this bishops colour
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) != 0) {
			v -= PawnOn_bCol_eg * countBits(DARK_SQUARES & pos->position[WP]);
		}
		else {
			v -= PawnOn_bCol_eg * countBits(~DARK_SQUARES & pos->position[WP]);
		}

		// Give bonus proportional to amount of enemy pawns on the bishops diagonals.
		v += enemy_pawns_on_diag_eg * countBits((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)]) & pos->position[BP]);
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
			v += rook_behind_passer_eg;
		}

		// Give bonus for eyeing the black king-ring
		v += (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & b_kingRing) != 0) ? rook_kingring_eg : 0;

		// Bonus for being on the same file as the black queen
		v += ((FileMasks8[sq % 8] & pos->position[BQ]) != 0) ? rook_on_queen_eg : 0;


		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = open_rook_eg;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) == 0 && (pos->position[BP] & FileMasks8[sq % 8]) != 0) { // Only half-open file. No white pawns only
			fileBonus = semi_rook_eg;
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
			v += queen_behind_passer_eg;
		}

		// Give bonus inversely proportional to the manhattan distance to the black king
		v += queen_kingDist_bonus_eg * ManhattanDistance[sq][pos->kingPos[1]];
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
			v -= knight_outpost_bonus_eg * outpost(pos, sq, BLACK, false);
		}
	}

	while (bishopBrd != 0) {
		sq = PopBit(&bishopBrd);

		// Give bonus if there are pawns on both sides of the board
		v -= (((FileMasks8[FILE_A] | FileMasks8[FILE_B] | FileMasks8[FILE_C]) & (pos->position[WP] | pos->position[BP])) != 0 &&
			((FileMasks8[FILE_F] | FileMasks8[FILE_G] | FileMasks8[FILE_H]) & (pos->position[WP] | pos->position[BP])) != 0) ? 40 : 0;

		// Give penalty proportional to the amount of pawns on this bishops colour
		if ((DARK_SQUARES & SETBIT((uint64_t)0, sq)) != 0) {
			v += PawnOn_bCol_eg * countBits(DARK_SQUARES & pos->position[BP]);
		}
		else {
			v += PawnOn_bCol_eg * countBits(~DARK_SQUARES & pos->position[BP]);
		}

		// Give bonus proportional to amount of enemy pawns on the bishops diagonals.
		v -= enemy_pawns_on_diag_eg * countBits((diagonalMasks[7 + (sq / 8) - (sq % 8)] | antidiagonalMasks[(sq / 8) + (sq % 8)]) & pos->position[WP]);
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
			v -= rook_behind_passer_eg;
		}

		// Give bonus for eyeing the white king-ring
		v -= (((FileMasks8[sq % 8] | RankMasks8[sq / 8]) & w_kingRing) != 0) ? rook_kingring_eg : 0;

		// Bonus for being on the same file as the white queen
		v -= ((FileMasks8[sq % 8] & pos->position[WQ]) != 0) ? rook_on_queen_eg : 0;


		// Bonus for open or semi-open file
		int fileBonus = 0;
		if (((pos->position[WP] | pos->position[BP]) & FileMasks8[sq % 8]) == 0) { // Fully open file. No pawns
			fileBonus = open_rook_eg;
		}
		else if ((pos->position[WP] & FileMasks8[sq % 8]) != 0 && (pos->position[BP] & FileMasks8[sq % 8]) == 0) { // Only half-open file. No white pawns only
			fileBonus = semi_rook_eg;
		}
		else {
			fileBonus = 0;
		}
		v -= fileBonus;
	}

	while (queenBrd != 0) {
		sq = PopBit(&queenBrd);

		if ((FileMasks8[sq % 8] & passedPawns) != 0) {
			v -= queen_behind_passer_eg;
		}

		// Give bonus inversely proportional to the manhattan distance to the black king
		v -= queen_kingDist_bonus_eg * ManhattanDistance[sq][pos->kingPos[0]];
	}



	return v;
}




int eval::mobility_mg(const S_BOARD* pos) {
	int v = 0;

	v += knight_mob_mg * (countBits(mobility<KNIGHT>(pos, WHITE)) - countBits(mobility<KNIGHT>(pos, BLACK)));
	v += bishop_mob_mg * (countBits(mobility<BISHOP>(pos, WHITE)) - countBits(mobility<BISHOP>(pos, BLACK)));
	v += rook_mob_mg * (countBits(mobility<ROOK>(pos, WHITE)) - countBits(mobility<ROOK>(pos, BLACK)));
	v += queen_mob_mg * (countBits(mobility<QUEEN>(pos, WHITE)) - countBits(mobility<QUEEN>(pos, BLACK)));

	return v;
}



int eval::mobility_eg(const S_BOARD* pos) {
	int v = 0;

	v += knight_mob_eg * (countBits(mobility<KNIGHT>(pos, WHITE)) - countBits(mobility<KNIGHT>(pos, BLACK)));
	v += bishop_mob_eg * (countBits(mobility<BISHOP>(pos, WHITE)) - countBits(mobility<BISHOP>(pos, BLACK)));
	v += rook_mob_eg * (countBits(mobility<ROOK>(pos, WHITE)) - countBits(mobility<ROOK>(pos, BLACK)));
	v += queen_mob_eg * (countBits(mobility<QUEEN>(pos, WHITE)) - countBits(mobility<QUEEN>(pos, BLACK)));

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






/*
THIS IS AN ATTEMPT TO CREATE A GAME-STAGE INDEPENDENT MATERIAL EVALUATION, AND SHOULD NOT BE USED YET DUE TO LACK OF TESTING.
*/

int eval::material(const S_BOARD* pos, int phase) {
	int v = 0;

	v += phase_material[phase][P] * (countBits(pos->position[WP]) - countBits(pos->position[BP]));
	v += phase_material[phase][N] * (countBits(pos->position[WN]) - countBits(pos->position[BN]));
	v += phase_material[phase][B] * (countBits(pos->position[WB]) - countBits(pos->position[BB]));
	v += phase_material[phase][R] * (countBits(pos->position[WR]) - countBits(pos->position[BR]));
	v += phase_material[phase][Q] * (countBits(pos->position[WQ]) - countBits(pos->position[BQ]));
	v += phase_material[phase][K] * (countBits(pos->position[WK]) - countBits(pos->position[BK]));

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


	return std::min(24, p);
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