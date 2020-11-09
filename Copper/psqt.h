#pragma once
#include "defs.h"

/*
Header file containing all the piece-square tables

TODO: Adjust the endgame piece-square tables.
*/

namespace psqt {
	// Pawns are incentivized to develop in the center, and to not move in front of a castled king.
	// Pawns in front of the king will be evaluated in the piece-evaluations.
	const int PawnTableMg[64] = {
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
		10	,	10	,	10	,	-15	,	-15	,	0	,	10	,	10	,
		5	,	0	,	0	,	-10	,	-10	,	0	,	0	,	5	,
		-3	,	-5	,	-5	,	15	,	15	,	5	,	0	,	0	,
		-6	,	-10	,	-5	,	7	,	7	,	0	,	0	,	0	,
		0	,	-5	,	5	,	2	,	2	,	0	,	0	,	0	,
		10	,	10	,	15	,	20	,	20	,	15	,	10	,	10	,
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};

	// In the endgame, we generally want the pawns to advance. The engine is incentivized to push pawns on the sides, but this difference
	// isn't that big
	const int PawnTableEg[64]{
		0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,
		10  ,   10  ,   10  ,   10  ,   10  ,   10  ,   10  ,   10  ,
		17  ,   16  ,   15  ,   15  ,   15  ,   15  ,   16  ,   17  ,
		36  ,   33  ,   30  ,   30  ,   30  ,   30  ,   33  ,   36  ,
		58  ,   54  ,   50  ,   50  ,   50  ,   50  ,   54  ,   58  ,
		80  ,   75  ,   70  ,   70  ,   70  ,   70  ,   75  ,   80  ,
		160 ,   155 ,   145 ,   140 ,   140 ,   145 ,   155 ,   160 ,
		0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0   ,   0
	};



	// Knights should be developed to the center, and stay away from the edges.
	const int KnightTableMg[64] = {
		-5	,	-15	,	-5	,	-5	,	-5	,	-5	,	-15	,	-5	,
		-5	,	0	,	0	,	5	,	5	,	0	,	0	,	-5	,
		-5	,	0	,	15	,	0	,	0	,	15	,	0	,	-5	,
		-5	,	5	,	5	,	10	,	10	,	5	,	5	,	-5	,
		-5	,	7	,	7	,	10	,	10	,	7	,	7	,	-5	,
		-5	,	0	,	5	,	5	,	5	,	5	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5
	};

	const int KnightTableEg[64] = {
	-5	,	-15	,	-5	,	-5	,	-5	,	-5	,	-15	,	-5	,
	-5	,	0	,	0	,	5	,	5	,	0	,	0	,	-5	,
	-5	,	0	,	15	,	0	,	0	,	15	,	0	,	-5	,
	-5	,	5	,	5	,	10	,	10	,	5	,	5	,	-5	,
	-5	,	7	,	7	,	10	,	10	,	7	,	7	,	-5	,
	-5	,	0	,	5	,	5	,	5	,	5	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5
	};

	// Bishops should also be developed in the center or to b2 og g2
	const int BishopTableMg[64] = {
		-5	,	-5	,	-12	,	-5	,	-5	,	-12	,	-5	,	-5	,
		-5	,	7	,	5	,	5	,	5	,	5	,	7	,	-5	,
		-5	,	5	,	7	,	3	,	3	,	7	,	5	,	-5	,
		-5	,	4	,	7	,	10	,	10	,	7	,	4	,	-5	,	
		-5	,	0	,	5	,	9	,	9	,	5	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5
	};

	const int BishopTableEg[64] = {
	-5	,	-5	,	-12	,	-5	,	-5	,	-12	,	-5	,	-5	,
	-5	,	7	,	5	,	5	,	5	,	5	,	7	,	-5	,
	-5	,	5	,	7	,	3	,	3	,	7	,	5	,	-5	,
	-5	,	4	,	7	,	10	,	10	,	7	,	4	,	-5	,
	-5	,	0	,	5	,	9	,	9	,	5	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5
	};

	// Rooks are incentivized to move to the center and 7th rank. They will later gain value by occupying open files.
	const int RookTableMg[64] = {
		-5	,	0	,	0	,	6	,	6	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
		20	,	20	,	20	,	20	,	20	,	20	,	20	,	20	,
		10	,	10	,	10	,	10	,	10	,	10	,	10	,	10
	};

	const int RookTableEg[64] = {
	-5	,	0	,	0	,	4	,	4	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	-5	,	0	,	0	,	0	,	0	,	0	,	0	,	-5	,
	20	,	20	,	20	,	20	,	20	,	20	,	20	,	20	,
	10	,	10	,	10	,	10	,	10	,	10	,	10	,	10
	};

	// Queen should not occupy the eight'th rank in middlegame, because we want the rooks connected. Small centralization bonus
	const int QueenTableMg[64] = {
		-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,
		0	,	0	,	1	,	1	,	1	,	1	,	0	,	0	,
		0	,	0	,	1	,	2	,	2	,	1	,	0	,	0	,
		0	,	0	,	2	,	3	,	3	,	2	,	0	,	0	,
		0	,	0	,	2	,	3	,	3	,	2	,	0	,	0	,
		0	,	0	,	1	,	2	,	2	,	1	,	0	,	0	,
		0	,	0	,	1	,	1	,	1	,	1	,	0	,	0	,
		0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};

	const int QueenTableEg[64] = {
	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,	-5	,
	0	,	0	,	1	,	1	,	1	,	1	,	0	,	0	,
	0	,	0	,	1	,	2	,	2	,	1	,	0	,	0	,
	0	,	0	,	2	,	3	,	3	,	2	,	0	,	0	,
	0	,	0	,	2	,	3	,	3	,	2	,	0	,	0	,
	0	,	0	,	1	,	2	,	2	,	1	,	0	,	0	,
	0	,	0	,	1	,	1	,	1	,	1	,	0	,	0	,
	0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
	};


	const int KingTableMg[64] = {
		20    ,    30    ,    10    ,    0    ,    0    ,    10    ,    30    ,    20    ,
		20    ,    20    ,    0    ,    0    ,    0    ,    0    ,    20    ,    20    ,
		-10    ,    -20    ,    -20    ,    -20    ,    -20    ,    -20    ,    -20    ,    -10    ,
		-20    ,    -30    ,    -30    ,    -40    ,    -40    ,    -30    ,    -30    ,    -20    ,
		-30    ,    -40    ,    -40    ,    -50    ,    -50    ,    -40    ,    -40    ,    -30 ,
		-30    ,    -40    ,    -40    ,    -50    ,    -50    ,    -40    ,    -40    ,    -30 ,
		-30    ,    -40    ,    -40    ,    -50    ,    -50    ,    -40    ,    -40    ,    -30 ,
		-30    ,    -40    ,    -40    ,    -50    ,    -50    ,    -40    ,    -40    ,    -30
	};

	const int KingTableEg[64] = {
		-50    ,    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,    -50    ,
		-30    ,    -30    ,    0    ,    0    ,    0    ,    0    ,    -30    ,    -30    ,
		-30    ,    -10    ,    20    ,    30    ,    30    ,    20    ,    -10    ,    -30    ,
		-30    ,    -10    ,    30    ,    40    ,    40    ,    30    ,    -10    ,    -30    ,
		-30    ,    -10    ,    30    ,    40    ,    40    ,    30    ,    -10    ,    -30    ,
		-30    ,    -10    ,    20    ,    30    ,    30    ,    20    ,    -10    ,    -30    ,
		-30    ,    -20    ,    -10    ,    0    ,    0    ,    -10    ,    -20    ,    -30    ,
		-50    ,    -40    ,    -30    ,    -20    ,    -20    ,    -30    ,    -40    ,    -50
	};

	// This is for the black pieces. If a black piece is at index 63, this will be index 0 on the psqt's
// 63 -> 0
// 0 -> 63
	const int Mirror64[64] = {
		H8, G8, F8, E8, D8, C8, B8, A8,
		H7, G7, F7, E7, D7, C7, B7, A7,
		H6, G6, F6, E6, D6, C6, B6, A6,
		H5, G5, F5, E5, D5, C5, B5, A5,
		H4, G4, F4, E4, D4, C4, B4, A4,
		H3, G3, F3, E3, D3, C3, B3, A3,
		H2, G2, F2, E2, D2, C2, B2, A2,
		H1, G1, F1, E1, D1, C1, B1, A1,
	};


	// A pawn-square table for the penalties of blocking a pawn.
#define S(mg, eg) make_score(mg, eg)
	const Score blockedPawnTable[64] = {
		S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),
		S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),
		S(0, 0),	S(-2, 0),	S(0, 0),	S(-15, 0),	S(-15, 0),	S(0, 0),	S(-2, 0),	S(0, 0),
		S(0, 0),	S(0, 0),	S(0, 0),	S(-10, 0),	S(-10, 0),	S(0, 0),	S(0, 0),	S(0, 0),
		S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),
		S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),	S(0, 0),
		S(0, -5),	S(0, -5),	S(0, -5),	S(0, -5),	S(0, -5),	S(0, -5),	S(0, -5),	S(0, -5),
		S(0, -10),	S(0, -10),	S(0, -10),	S(0, -10),	S(0, -10),	S(0, -10),	S(0, -10),	S(0, -10)
	};
#undef S
}

