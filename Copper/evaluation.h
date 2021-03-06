#pragma once
#include "defs.h"
#include "psqt.h"

//#define TUNE


enum PieceType {
	NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
	ALL_PIECES = 0,
	PIECE_TYPE_NB = 8
};

enum KAS_SIDE :int { w = 0, b = 1 };

// Short for KingAttackS
struct KAS { // For all arrays, the zero'th element is white and the first is black.
	int attacks[2] = { 0 }; // The amount of attacks
	int attackWeights[2] = { 0 }; // The index to the safety table

	// Only the attacked squares around the king.
	BitBoard attacked_squares[2] = { 0 };

	BitBoard kingZones[2] = { 0 };

	// The below bitboards are in place such that we don't have to run mobility again for all pieces.
	BitBoard KnightAttacks[2] = { 0 };
	BitBoard BishopAttacks[2] = { 0 };
	BitBoard RookAttacks[2] = { 0 };
	BitBoard QueenAttacks[2] = { 0 };
};



inline int doubledCnt(uint64_t pawnBrd) {
	int cnt = 0;
	for (int i = 0; i < 8; i++) { // Iterate over all files
		cnt += (countBits(pawnBrd & FileMasks8[i]) >= 2) ? 1 : 0;
	}
	return cnt;
}

inline uint64_t kingRing(const S_BOARD* pos, S_SIDE s) {
	BitBoard K = 0;
	if (s == WHITE) {
		K = pos->position[WK];
	}
	else {
		K = pos->position[BK];
	}


	BitBoard kingMoves = 0;

	// Horizontal and vertical
	kingMoves |= (K & ~RankMasks8[RANK_8]) << 8;
	kingMoves |= (K & ~RankMasks8[RANK_1]) >> 8;
	kingMoves |= (K & ~FileMasks8[FILE_A]) >> 1;
	kingMoves |= (K & ~FileMasks8[FILE_H]) << 1;

	// Diagonal
	kingMoves |= (K & ~(FileMasks8[FILE_A] | RankMasks8[RANK_8])) << 7;
	kingMoves |= (K & ~(FileMasks8[FILE_H] | RankMasks8[RANK_8])) << 9;
	kingMoves |= (K & ~(FileMasks8[FILE_A] | RankMasks8[RANK_1])) >> 9;
	kingMoves |= (K & ~(FileMasks8[FILE_H] | RankMasks8[RANK_1])) >> 7;

	return (kingMoves | K);
}



// evaluation.cpp
int addPsqtVal(int sq, int pce, bool eg);
int defending_pawns(const S_BOARD* pos, int sq, S_SIDE side);

int pawnless_flank(const S_BOARD* pos, bool side);


namespace eval {
	int staticEval(const S_BOARD* pos, int alpha, int beta);

	int imbalance(const S_BOARD* pos); // Tord Romstad's second order polynomial imbalance will be implemented at some point. For now, we only give a
	// bonus for bishop pair and penalty for rook and knight pair. We also raise and lower the rooks and knights value depending on amount of pawns
	// respectively

	int mg_evaluate(const S_BOARD* pos, int alpha, int beta, KAS* kas = nullptr);
	int eg_evaluate(const S_BOARD* pos, int alpha, int beta, KAS* kas = nullptr);


	int material(const S_BOARD* pos, int phase);

	// material_mg and material_eg are phase separated and called in mg_evaluate and eg_evaluate, but material is not, and i think it is faster.
	int material_mg(const S_BOARD* pos);
	int material_eg(const S_BOARD* pos);

	int psqt_mg(const S_BOARD* pos);
	int psqt_eg(const S_BOARD* pos);

	int phase(const S_BOARD* pos);

	int pawns_mg(const S_BOARD* pos);
	int pawns_eg(const S_BOARD* pos);

	int pieces_mg(const S_BOARD* pos);
	int pieces_eg(const S_BOARD* pos);

	int king_mg(const S_BOARD* pos, KAS* kas = nullptr);
	int king_eg(const S_BOARD* pos, KAS* kas = nullptr);


	template <S_SIDE Us>
	int pawn_shield(const S_BOARD* pos, int kingSq, bool mg);


	template <PieceType pce>
	uint64_t mobility(const S_BOARD* pos, S_SIDE side);

	int mobility_mg(const S_BOARD* pos, KAS* kas = nullptr);
	int mobility_eg(const S_BOARD* pos, KAS* kas = nullptr);

	int scale_factor(const S_BOARD* pos, int eg_eval);

	int outpost(const S_BOARD* pos, int sq, S_SIDE side, bool middlegame);

	bool material_draw(const S_BOARD* pos);

	int getMaterial(const S_BOARD* pos, bool side);


	enum mgValues : int {
		pawnValMg = 171,
		knightValMg = 899,
		bishopValMg = 823,
		rookValMg = 1084,
		queenValMg = 2218,
		kingValMg = 20000
	};


	enum egValues : int {
		pawnValEg = 61,
		knightValEg = 364,
		bishopValEg = 348,
		rookValEg = 631,
		queenValEg = 1248,
		kingValEg = 20000
	};


	static int pieceValMg[13] = { pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg,
	pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg, 0 };
}



// Constants
static int LAZYNESS_MG = eval::knightValMg; // The safety margin for lazy evaluation
static int LAZYNESS_EG = eval::rookValEg;

constexpr int tempo = 18; // The value added to the side to move.

// The below imbalances are taken from GM Larry Kaufman's paper, "The evaluation of piece imbalances"
// The ratios are taken from the CPW-engine
static int bishop_pair = eval::pawnValMg / 2;
static int p_knight_pair = (int)((double)bishop_pair / 3.75);
static int p_rook_pair = (int)((double)bishop_pair / 1.875);


static int knight_pawn_penalty = eval::pawnValMg / 8;
static int rook_pawn_bonus = eval::pawnValMg / 16;

/*
Pawn coefficients.
*/
constexpr int passedPawnValue[8] = { 0, 12, 19, 34, 80, 167, 193, 0 };
constexpr int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };
constexpr int mirrorFileNum[8] = { FILE_H, FILE_G, FILE_F, FILE_E, FILE_D, FILE_C, FILE_B, FILE_A };

// Penalties for doubled pawns
constexpr int doubled_penalty_mg = 8;
constexpr int doubled_penalty_eg = 61;

constexpr int blocked_penalty[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// Penalties for isolated and doubled pawns:
constexpr int DoubledIsolatedMg = 31;
constexpr int DoubledIsolatedEg = 51;

constexpr int isolatedMg = 11;
constexpr int isolatedEg = 26;

// Bonus for a pawn being supported
constexpr int supported_mg = 24;
constexpr int supported_eg = 10;



/*
Piece coefficients
*/

// Outpost bonuses:
constexpr int outpost_bonus_mg = 1;
constexpr int outpost_bonus_eg = 3;

constexpr int safe_outpost_bonus_mg = 2;
constexpr int safe_outpost_bonus_eg = 3;

constexpr int knight_outpost_bonus_mg = 1;
constexpr int knight_outpost_bonus_eg = 2;



// Pawn related piece coefficients
constexpr int N_pawn_defence_mg = 11;
constexpr int B_pawn_defence_mg = 3;

constexpr int PawnOn_bCol_mg = 1;
constexpr int PawnOn_bCol_eg = 9;

constexpr int enemy_pawns_on_diag_eg = 14;

// Bonuses for "eye'ing" the enemy king-ring
constexpr int bishop_kingring_mg = 12;


// Rook coefficients
constexpr int doubled_rooks_mg = 69;

constexpr int rook_on_queen_mg = 9;
constexpr int rook_on_queen_eg = 14;

constexpr int rook_behind_passer_eg = 125;

constexpr int rook_kingring_mg = 8;
constexpr int rook_kingring_eg = 19;

constexpr int open_rook_mg = 34;
constexpr int open_rook_eg = 23;

constexpr int semi_rook_mg = 14;
constexpr int semi_rook_eg = 16;


// Queen coefficents
constexpr int queen_behind_passer_eg = 13;
constexpr int queen_kingDist_bonus_eg = 4;




/*
Mobility coefficients
*/
constexpr int knight_mob_mg = 11;
constexpr int knight_mob_eg = 4;

constexpr int bishop_mob_mg = 5;
constexpr int bishop_mob_eg = 5;

constexpr int rook_mob_mg = 7;
constexpr int rook_mob_eg = 8;

constexpr int queen_mob_mg = 5;
constexpr int queen_mob_eg = 7;


/*
King coefficients
*/
constexpr int castling_bonus = 25;


enum material_type { P = 0, N = 1, B = 2, R = 3, Q = 4, K = 5 };

extern int phase_material[25][6];

constexpr int safety_mg[100] = {
	0	,	  0		,		1	,	2		,	   3	,	   5	,	   7	,   9	,  12	,  15	,
	18	,	  22	,	  26	,	30		,	  35	,	  39	,	  44	,  50	,  56	,  62	,
	68	,	  75	,	  82	,	85		,	  89	,	  97	,	 105	, 113	, 122	, 131	,
	140	,	 150	,	 169	,	180		,	 191	,	 202	,	 213	, 225	, 237	, 248	,
	260	,	 272	,	 283	,	295		,	 307	,	 319	,	 330	, 342	, 354	, 366	,
	377	,	 389	,	 401	,	412		,	 424	,	 436	,	 448	, 459	, 471	, 483	,
	494	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500
};



constexpr int safety_eg[100] = {
	0	,	  0		,		1	,	2		,	   3	,	   5	,	   7	,   9	,  12	,  15	,
	18	,	  22	,	  26	,	30		,	  35	,	  39	,	  44	,  50	,  56	,  62	,
	68	,	  75	,	  82	,	85		,	  89	,	  97	,	 105	, 113	, 122	, 131	,
	140	,	 150	,	 169	,	180		,	 191	,	 202	,	 213	, 225	, 237	, 248	,
	260	,	 272	,	 283	,	295		,	 307	,	 319	,	 330	, 342	, 354	, 366	,
	377	,	 389	,	 401	,	412		,	 424	,	 436	,	 448	, 459	, 471	, 483	,
	494	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500	,
	500	,	 500	,	 500	,	500		,	 500	,	 500	,	 500	, 500	, 500	, 500
};