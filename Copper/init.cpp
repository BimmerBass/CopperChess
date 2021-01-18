#include "defs.h"
#include "evaluation.h"

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


S_OPTIONS* engineOptions = new S_OPTIONS();


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
	initManhattanDistances();
	initOutpostMasks();
	initReductions();
	initPhaseMaterial();
	initKingZones();

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


inline int calcManhattanDistance(int sq1, int sq2) {
	int rank1 = sq1 / 8;
	int file1 = sq1 % 8;

	int rank2 = sq2 / 8;
	int file2 = sq2 % 8;

	int rankDistance = abs(rank2 - rank1);
	int fileDistance = abs(file2 - file1);
	return rankDistance + fileDistance;
}

int ManhattanDistance[64][64] = { {0} };

void initManhattanDistances() {

	for (int sq1 = 0; sq1 < 64; sq1++) {

		for (int sq2 = 0; sq2 < 64; sq2++) {
			ManhattanDistance[sq1][sq2] = calcManhattanDistance(sq1, sq2);
		}

	}

}


// Outpost masks
BitBoard whiteOutpostMasks[64] = {};
BitBoard blackOutpostMasks[64] = {};


void initOutpostMasks() {
	BitBoard mask_w = 0;
	BitBoard mask_b = 0;
	int tsq_w;
	int tsq_b;

	for (int sq = 0; sq < 64; sq++) {
		mask_w = 0;
		mask_b = 0;

		// WHITE MASKS
		if ((((uint64_t)1 << sq) & FileMasks8[FILE_A]) == 0) { // sq is not on the a-file
			tsq_w = sq + 7;
			tsq_b = sq - 9;

			while (tsq_w < 64){
				mask_w |= (uint64_t)1 << tsq_w;
				tsq_w += 8;
			}

			while (tsq_b >= 0) {
				mask_b |= (uint64_t)1 << tsq_b;
				tsq_b -= 8;
			}
		}

		if ((((uint64_t)1 << sq) & FileMasks8[FILE_H]) == 0) { // sq is not on h-file
			tsq_w = sq + 9;
			tsq_b = sq - 7;

			while (tsq_w < 64) {
				mask_w |= (uint64_t)1 << tsq_w;
				tsq_w += 8;
			}

			while (tsq_b >= 0) {
				mask_b |= (uint64_t)1 << tsq_b;
				tsq_b -= 8;
			}
		}

		whiteOutpostMasks[sq] = mask_w;
		blackOutpostMasks[sq] = mask_b;
		

	}
}



int Reductions[MAXPOSITIONMOVES] = {};

void initReductions() {
	for (int i = 1; i < MAXPOSITIONMOVES; i++) {
#if (defined(_WIN32) || defined(_WIN64))
		Reductions[i] = int(22.9 * std::log(i));
#else
		Reductions[i] = int(22.9 * log(i));
#endif
	}
}


int phase_material[25][6] = { {} };


void initPhaseMaterial() {
	int mg_weight;
	int eg_weight;

	for (int phase = 0; phase < 25; phase++) {
		mg_weight = phase;
		eg_weight = 24 - phase;

		// Pawns
		phase_material[phase][0] = ((mg_weight * eval::pawnValMg) + (eg_weight * eval::pawnValEg)) / 24;

		// Knights
		phase_material[phase][1] = ((mg_weight * eval::knightValMg) + (eg_weight * eval::knightValEg)) / 24;

		// Bishops
		phase_material[phase][2] = ((mg_weight * eval::bishopValMg) + (eg_weight * eval::bishopValEg)) / 24;

		// Rooks
		phase_material[phase][3] = ((mg_weight * eval::rookValMg) + (eg_weight * eval::rookValEg)) / 24;

		// Queens
		phase_material[phase][4] = ((mg_weight * eval::queenValMg) + (eg_weight * eval::queenValEg)) / 24;

		// Kings
		phase_material[phase][5] = ((mg_weight * eval::kingValMg) + (eg_weight * eval::kingValEg)) / 24;
	}


}


BitBoard king_zone[64] = { 0 };


void initKingZones() {
	uint64_t kz = 0;

	for (int sq = 0; sq < 64; sq++) {
		kz = 0;

		int rank = sq / 8;
		int file = sq % 8;

		// Create the king ring

		// Horizontal and vertical
		kz |= ((uint64_t(1) << sq) & ~RankMasks8[RANK_8]) << 8;
		kz |= ((uint64_t(1) << sq) & ~RankMasks8[RANK_1]) >> 8;
		kz |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_A]) >> 1;
		kz |= ((uint64_t(1) << sq) & ~FileMasks8[FILE_H]) << 1;
				
		// Diagonal
		kz |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_A] | RankMasks8[RANK_8])) << 7;
		kz |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_H] | RankMasks8[RANK_8])) << 9;
		kz |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_A] | RankMasks8[RANK_1])) >> 9;
		kz |= ((uint64_t(1) << sq) & ~(FileMasks8[FILE_H] | RankMasks8[RANK_1])) >> 7;

		// When doing the kingRing for piece attacks, we OR kz with 1 << sq, but this isn't necessary here.

		// Now go through the directions going three squares out for each.
		int tsq = sq + 8;
		int sq_cnt = 0;

		// North
		while (tsq <= 63 && sq_cnt <= 3) {
			kz |= (uint64_t(1) << tsq);
			sq_cnt++;
			tsq += 8;
		}

		// South
		tsq = sq - 8;
		sq_cnt = 0;

		while (tsq >= 0 && sq_cnt <= 3) {
			kz |= (uint64_t(1) << tsq);
			sq_cnt++;
			tsq -= 8;
		}

		// East
		tsq = sq + 1;
		sq_cnt = 0;
		int tRank = tsq / 8;

		while (tsq >= 0 && tsq <= 63 && tRank == rank && sq_cnt <= 3) {
			kz |= (uint64_t(1) << tsq);
			sq_cnt++;
			tsq += 1;

			tRank = tsq / 8;
		}

		// West
		tsq = sq - 1;
		sq_cnt = 0;
		tRank = tsq / 8;

		while (tsq >= 0 && tsq <= 63 && tRank == rank && sq_cnt <= 3) {
			kz |= (uint64_t(1) << tsq);
			sq_cnt++;
			tsq -= 1;

			tRank = tsq / 8;
		}

	
		// North west
		BitBoard diagonal = antidiagonalMasks[rank + file];

		tsq = sq + 14;

		for (int i = 0; i < 3; i++) {
			kz |= ((uint64_t(1) << tsq) & diagonal);

			tsq += 7;
		}


		// South east
		tsq = sq - 14;

		for (int i = 0; i < 3; i++) {
			kz |= ((uint64_t(1) << tsq) & diagonal);

			tsq -= 7;
		}

		// North east
		diagonal = diagonalMasks[7 + (rank - file)];

		tsq = sq + 18;

		for (int i = 0; i < 3; i++) {
			kz |= ((uint64_t(1) << tsq) & diagonal);

			tsq += 9;
		}

		// South west

		tsq = sq - 18;

		for (int i = 0; i < 3; i++) {
			kz |= ((uint64_t(1) << tsq) & diagonal);

			tsq -= 9;
		}

		king_zone[sq] = kz;
	}
}