#pragma once
#include "defs.h"
#include "psqt.h"


// evaluation.cpp
int addPsqtVal(int sq, int pce, bool eg);
namespace eval {
	int staticEval(const S_BOARD* pos, int depth, int alpha, int beta);

	int material_mg(const S_BOARD* pos);
	int material_eg(const S_BOARD* pos);
	void material_both(const S_BOARD* pos, int& v_mg, int& v_eg);

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
}