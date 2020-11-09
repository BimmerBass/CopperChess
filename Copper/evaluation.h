#pragma once
#include "defs.h"
#include "psqt.h"

enum PieceType {
	NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
	ALL_PIECES = 0,
	PIECE_TYPE_NB = 8
};



// evaluation.cpp
int addPsqtVal(int sq, int pce, bool eg);
namespace eval {
	int staticEval(const S_BOARD* pos, int depth, int alpha, int beta);

	int imbalance(const S_BOARD* pos); // Tord Romstad's second order polynomial imbalance will be implemented at some point. For now, we only give a
	// bonus for bishop pair and penalty for rook and knight pair. We also raise and lower the rooks and knights value depending on amount of pawns
	// respectively

	int mg_evaluate(const S_BOARD* pos);
	int eg_evaluate(const S_BOARD* pos);

	int material_mg(const S_BOARD* pos);
	int material_eg(const S_BOARD* pos);

	int psqt_mg(const S_BOARD* pos);
	int psqt_eg(const S_BOARD* pos);

	int phase(const S_BOARD* pos);

	void kingEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);
	void queenEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);
	void rookEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);
	void bishopEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);
	void knightEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);
	void pawnEval(const S_BOARD* pos, int& mgScore, int& egScore, int& score, int sq);

	int getMaterial(const S_BOARD* pos, bool side);

	/*
	The piece values are inspired by the ones found on chessprogramming.org in the sense that they meet the same requirements:

	1. B > N > 3P
	2. B + N > R + P
	3. 2R > Q
	4. R > B + 2P > N + 2P
	*/
	enum mgValues : int {
		pawnValMg = 100,
		knightValMg = 320,
		bishopValMg = 350,
		rookValMg = 560,
		queenValMg = 1050,
		kingValMg = 20000
	};

	/*
		pawnValEg = 200,
		knightValEg = 560,
		bishopValEg = 600,
		rookValEg = 1200,
		queenValEg = 2300,
		kingValEg = 20000
	*/

	enum egValues : int {
		pawnValEg = 100,
		knightValEg = 320,
		bishopValEg = 350,
		rookValEg = 560,
		queenValEg = 1050,
		kingValEg = 20000
	};

	static int pieceValMg[13] = { pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg,
	pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg, 0 };


	class Tuner{
	public:
		Tuner(int &tuningVar, int learningRate, int initialBounds, int gameCnt, int movetime);
		~Tuner();

	private:
		/*
		- We'll copy the variable to tune and only change it after the tuning run
		- The learning rate is the number we'll increment tunedVar with after each game result
		- The bounds are the initial padding around the variable:
			- If the tuned variable is 100, and the bounds are 20. The two competing engines will have
				the values 80 and 120.
			- If engine(80) wins, we'll add the learningRate to tunedVar, such if for example
				learninRate = 2, tunedVar = 102.
			- This will repeat with the new bounds, engine(82) vs. engine(122) until we have played all the games.
		*/
		int tunedVar;
		int learningRate;
		int bounds;

		int gameCount;
		int movetime;

		void RunTuning();
	};
}

// Constants
const int tempo = 18; // The value added to the side to move.

// The below imbalances are taken from GM Larry Kaufman's paper, "The evaluation of piece imbalances"
// The ratios are taken from the CPW-engine
const int bishop_pair = eval::pawnValMg / 2;
const int p_knight_pair = (int)((double)bishop_pair / 3.75);
const int p_rook_pair = (int)((double)bishop_pair / 1.875);


const int knight_pawn_penalty = eval::pawnValMg / 8;
const int rook_pawn_bonus = eval::pawnValMg / 16;

const int passedPawnValue[8] = { 0, 5, 10, 25, 35, 60, 100, 140 };
const int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };