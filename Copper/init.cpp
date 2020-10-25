#include "defs.h"

/*
INCLUDES THE FUNCTIONS:
- initBishopAttacks
- initRookAttacks
- initBitMasks
- initHashKeys
- initPawnMasks
- initMvvLva()
- initAll
- initRookSupportMasks()
*/


// Bitmasks for setting and clearing bits.
BitBoard SetMask[64] = {};
BitBoard ClearMask[64] = {};

// Attack bitboards for sliding pieces
BitBoard LineAttackRays[4][64] = { {} };
BitBoard DiagonalAttackRays[4][64] = { {} };

void initAll(BitBoard(&set)[64], BitBoard(&clear)[64]) {
	initBishopAttacks();
	initRookAttacks();
	initMvvLva();
	initBitMasks(set, clear);
	initPawnMasks();
	initHashKeys();
	initRookSupportMasks();

	initPolyBook();
}

void initBitMasks(BitBoard(&set)[64], BitBoard(&clear)[64]) {
	for (int i = 0; i < 64; i++) {
		set[i] = 0;
		clear[i] = 0;
	}
	for (int i = 0; i < 64; i++) {
		set[i] = (uint64_t)1 << i;
		clear[i] = ~set[i];
	}
}

void initRookAttacks() {
	BitBoard nort = 0x0101010101010100;
	for (int i = 0; i < 64; i++, nort <<= 1) {
		LineAttackRays[0][i] = nort;
	}

	BitBoard sout = 0x0080808080808080;
	for (int i = 0; i < 64; i++) {
		LineAttackRays[1][i] = (sout >> (i ^ 63));
	}

	const BitBoard one = 1;
	for (int i = 0; i < 64; i++) {
		LineAttackRays[2][i] = (2 * ((one << (i | 7)) - (one << i)));
	}

	for (int i = 0; i < 64; i++) {
		LineAttackRays[3][i] = ((one << i) - (one << (i & 56)));
	}
}

void initBishopAttacks() {
	for (int i = 0; i < 64; i++) {
		BitBoard noea = 0;
		int file = i % 8;
		int rank = i / 8;
		int num = 0;
		while (file <= 7 && rank <= 7) {
			noea |= (uint64_t)1 << (9 * num + i);
			rank += 1; file += 1; num += 1;
		}
		noea ^= (uint64_t)1 << i;
		DiagonalAttackRays[0][i] = noea;
	}

	for (int i = 0; i < 64; i++) {
		BitBoard nowe = 0;
		int file = i % 8;
		int rank = i / 8;
		int num = 0;
		while (file >= 0 && rank <= 7) {
			nowe |= (uint64_t)1 << (7 * num + i);
			rank += 1; file -= 1; num += 1;
		}
		nowe ^= (uint64_t)1 << i;
		DiagonalAttackRays[1][i] = nowe;
	}

	for (int i = 0; i < 64; i++) {
		BitBoard soea = 0;
		int file = i % 8;
		int rank = i / 8;
		int num = 1;
		while (file <= 7 && rank >= 0) {
			soea |= (uint64_t)1 << (8 * rank + file);
			rank -= 1; file += 1; num += 1;
		}
		soea ^= (uint64_t)1 << i;
		DiagonalAttackRays[2][i] = soea;
	}

	for (int i = 0; i < 64; i++) {
		BitBoard sowe = 0;
		int file = i % 8;
		int rank = i / 8;
		int num = 1;
		while (file >= 0 && rank >= 0) {
			sowe |= (uint64_t)1 << (8 * rank + file);
			rank -= 1; file -= 1; num += 1;
		}
		sowe ^= (uint64_t)1 << i;
		DiagonalAttackRays[3][i] = sowe;
	}

}

BitBoard pieceKeys[13][64] = { {} };
BitBoard sideKey = 0;
BitBoard castleKeys[16] = {};

void initHashKeys() {
	for (int index = 0; index < 13; index++) {
		for (int index2 = 0; index2 < 64; index2++) {
			pieceKeys[index][index2] = RAND_64;
		}
	}
	sideKey = RAND_64;

	for (int index = 0; index < 16; index++) {
		castleKeys[index] = RAND_64;
	}
}

BitBoard whitePassedPawnMasks[64] = {};
BitBoard blackPassedPawnMasks[64] = {};
BitBoard isolatedPawnMasks[8] = {};

void initPawnMasks() {
	// Isolated pawn mask is first initialized
	BitBoard mask = 0;
	for (int f = 0; f < 8; f++) {
		mask = FileMasks8[f];
		if (f == 0) {
			mask |= FileMasks8[f + 1];
			isolatedPawnMasks[f] = mask;
		}
		else if (f == 7) {
			mask |= FileMasks8[f - 1];
			isolatedPawnMasks[f] = mask;
		}
		else {
			mask |= FileMasks8[f - 1];
			mask |= FileMasks8[f + 1];
			isolatedPawnMasks[f] = mask;
		}
	}

	// Passed pawn masks:
	for (int sq = 0; sq < 64; sq++) {

		// White pawnmasks
		mask = 0;
		int tsq = sq + 8;
		while (tsq <= 63) {
			mask |= ((uint64_t)1 << tsq);
			tsq += 8;
		}

		if ((((uint64_t)1 << sq) & FileMasks8[FILE_A]) == 0) { // sq is not on the a-file
			int tsq = sq + 7;
			while (tsq <= 63) {
				mask |= ((uint64_t)1 << tsq);
				tsq += 8;
			}
		}

		if ((((uint64_t)1 << sq) & FileMasks8[FILE_H]) == 0) { // sq is not on the h-file
			int tsq = sq + 9;
			while (tsq <= 63) {
				mask |= ((uint64_t)1 << tsq);
				tsq += 8;
			}
		}
		whitePassedPawnMasks[sq] = mask;

		// Black pawnmasks
		mask = 0;
		tsq = sq - 8;
		while (tsq >= 0) {
			mask |= ((uint64_t)1 << tsq);
			tsq -= 8;
		}

		if ((((uint64_t)1 << sq) & FileMasks8[FILE_A]) == 0) {
			tsq = sq - 9;
			while (tsq >= 0) {
				mask |= ((uint64_t)1 << tsq);
				tsq -= 8;
			}
		}
		if ((((uint64_t)1 << sq) & FileMasks8[FILE_H]) == 0) {
			tsq = sq - 7;
			while (tsq >= 0) {
				mask |= ((uint64_t)1 << tsq);
				tsq -= 8;
			}
		}
		blackPassedPawnMasks[sq] = mask;
	}
}


int MvvLva[12][12] = { {} };
static int victimScores[12] = { 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };

void initMvvLva() {
	for (int att = WP; att <= BK; att++) {
		for (int vic = WP; vic <= BK; vic++) {
			MvvLva[vic][att] = victimScores[vic] + (6 - (victimScores[att] / 100));
		}
	}
}


BitBoard whiteRookSupport[64] = { 0 };
BitBoard blackRookSupport[64] = { 0 };

void initRookSupportMasks() {

	BitBoard mask;

	for (int sq = 0; sq < 64; sq++) {
		mask = 0;

		int tsq = sq - 8;
		while (tsq >= 0) {
			mask |= (uint64_t)1 << tsq;
			tsq -= 8;
		}
		whiteRookSupport[sq] = mask;

		mask = 0;

		tsq = sq + 8;
		while (tsq <= 63) {
			mask |= (uint64_t)1 << tsq;
			tsq += 8;
		}
		blackRookSupport[sq] = mask;
	
	}
}
