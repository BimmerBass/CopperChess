#include "evaluation.h"


template<>
uint64_t eval::mobility<KNIGHT>(const S_BOARD* pos, S_SIDE side) {
	uint64_t knightBrd = (side == WHITE) ? pos->position[WN] : pos->position[BN];

	uint64_t destinationSquares = 0;
	int index;

	while (knightBrd != 0) {
		index = PopBit(&knightBrd);

		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[7] ^ UNIVERSE)) << 17);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[0] ^ UNIVERSE)) << 15);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) << 10);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) >> 6);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[7] ^ UNIVERSE)) >> 15);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[0] ^ UNIVERSE)) >> 17);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) << 6);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) >> 10);
	}

	uint64_t pseudoLegalDests = (side == WHITE) ? (destinationSquares & (~pos->WHITE_PIECES)) : (destinationSquares & (~pos->BLACK_PIECES));

	return pseudoLegalDests;
}

template<>
uint64_t eval::mobility<BISHOP>(const S_BOARD* pos, S_SIDE side) {
	uint64_t bishopBrd = (side == WHITE) ? pos->position[WB] : pos->position[BB];

	uint64_t destinationSquares = 0;
	int index;

	while (bishopBrd != 0) {
		index = PopBit(&bishopBrd);


		attacks::positiveDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 0, index); // North east
		attacks::positiveDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 1, index); // North west
		attacks::negativeDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 2, index); // South east
		attacks::negativeDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 3, index); // South west
	}

	uint64_t pseudoLegalDests = (side == WHITE) ? (destinationSquares & (~pos->WHITE_PIECES)) : (destinationSquares & (~pos->BLACK_PIECES));

	return pseudoLegalDests;
}

template<>
uint64_t eval::mobility<ROOK>(const S_BOARD* pos, S_SIDE side) {
	uint64_t rookBrd = (side == WHITE) ? pos->position[WR] : pos->position[BR];

	uint64_t destinationSquares = 0;
	int index;

	while (rookBrd != 0) {
		index = PopBit(&rookBrd);


		attacks::positiveLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 0, index); // North
		attacks::positiveLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 2, index); // East
		attacks::negativeLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 1, index); // South
		attacks::negativeLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 3, index); // West
	}

	uint64_t pseudoLegalDests = (side == WHITE) ? (destinationSquares & (~pos->WHITE_PIECES)) : (destinationSquares & (~pos->BLACK_PIECES));

	return pseudoLegalDests;
}

template<>
uint64_t eval::mobility<QUEEN>(const S_BOARD* pos, S_SIDE side) {
	uint64_t queenBrd = (side == WHITE) ? pos->position[WQ] : pos->position[BQ];

	uint64_t destinationSquares = 0;
	int index;

	while (queenBrd != 0) {
		index = PopBit(&queenBrd);

		attacks::positiveLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 0, index); // North
		attacks::positiveLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 2, index); // East
		attacks::negativeLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 1, index); // South
		attacks::negativeLineAttacks(destinationSquares, ~pos->EMPTY_SQUARES, 3, index); // West

		attacks::positiveDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 0, index); // North east
		attacks::positiveDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 1, index); // North west
		attacks::negativeDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 2, index); // South east
		attacks::negativeDiagAttacks(destinationSquares, (~pos->EMPTY_SQUARES), 3, index); // South west
	}

	uint64_t pseudoLegalDests = (side == WHITE) ? (destinationSquares & (~pos->WHITE_PIECES)) : (destinationSquares & (~pos->BLACK_PIECES));

	return pseudoLegalDests;
}