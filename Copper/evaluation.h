#pragma once
#include "defs.h"
#include "psqt.h"



enum PieceType {
	NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
	ALL_PIECES = 0,
	PIECE_TYPE_NB = 8
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

	int mg_evaluate(const S_BOARD* pos, int alpha, int beta);
	int eg_evaluate(const S_BOARD* pos, int alpha, int beta);


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

	int king_mg(const S_BOARD* pos);
	int king_eg(const S_BOARD* pos);



	template <PieceType pce>
	uint64_t mobility(const S_BOARD* pos, S_SIDE side);

	int mobility_mg(const S_BOARD* pos);
	int mobility_eg(const S_BOARD* pos);

	int scale_factor(const S_BOARD* pos, int eg_eval);

	int outpost(const S_BOARD* pos, int sq, S_SIDE side);

	bool material_draw(const S_BOARD* pos);

	int getMaterial(const S_BOARD* pos, bool side);

	/*
	The piece values are inspired by the ones found on chessprogramming.org in the sense that they meet the same requirements:

	1. B > N > 3P
	2. B + N > R + P
	3. 2R > Q
	4. R > B + 2P > N + 2P
	*/
	/*enum mgValues : int {
		pawnValMg = 120,
		knightValMg = 360,
		bishopValMg = 390,
		rookValMg = 600,
		queenValMg = 1150,
		kingValMg = 20000
	};*/

	/*enum mgValues : int {
		pawnValMg = 200,
		knightValMg = 640,
		bishopValMg = 700,
		rookValMg = 1120,
		queenValMg = 2100,
		kingValMg = 20000
	};*/

	/*enum mgValues : int {
		pawnValMg = 100,
		knightValMg = 320,
		bishopValMg = 350,
		rookValMg = 560,
		queenValMg = 1050,
		kingValMg = 20000
	};*/

	extern int pawnValMg;
	extern int knightValMg;
	extern int bishopValMg;
	extern int rookValMg; 
	extern int queenValMg;
	extern int kingValMg;
	
	extern int pawnValEg;
	extern int knightValEg;
	extern int bishopValEg;
	extern int rookValEg;
	extern int queenValEg;
	extern int kingValEg;

	/*enum egValues : int {
		pawnValEg = 100,
		knightValEg = 320,
		bishopValEg = 350,
		rookValEg = 560,
		queenValEg = 1050,
		kingValEg = 20000
	};*/
	

	/*enum egValues : int {
		pawnValEg = 200,
		knightValEg = 490,
		bishopValEg = 530,
		rookValEg = 905,
		queenValEg = 1775,
		kingValEg = 20000
	};*/

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

// Pawn coefficients
extern int passedPawnValue[8];
constexpr int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };
constexpr int doubled_penalty = 11;
constexpr int blocked_penalty[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };


// Piece coefficients
extern int outpost_bonus;
extern int safe_outpost_bonus;


// King bonuses and penalties
constexpr int castling_bonus = 25;


enum material_type { P = 0, N = 1, B = 2, R = 3, Q = 4, K = 5 };

extern int phase_material[25][6];