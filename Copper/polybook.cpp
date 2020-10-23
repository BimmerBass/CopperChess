#include "polykeys.h"
#include "defs.h"

struct S_BOOKENTRY {
	uint64_t key = 0;
	uint16_t move = 0;
	uint16_t weight = 0;
	uint32_t learn = 0;
};

int polyPce[13] = { 1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10, -1 };

long numEntries = 0; // Tells us how many entries we have.

S_BOOKENTRY* entries;


void initPolyBook() {
	FILE* pFile;
	// Open the opening book ile
	fopen_s(&pFile, "performance.bin", "rb");
	
	// Make sure it isn't pointing at nothing.
	if (pFile == NULL) {
		std::cout << "[!] Failed to open book file." << std::endl;
	}
	else {
		fseek(pFile, 0, SEEK_END); // Go to the end of file to see how much data there is.
		long position = ftell(pFile);
		
		// If there's not even enough data to read into a single entry, return.
		if (position < sizeof(S_BOOKENTRY)) {
			std::cout << "[!] No entries found in book." << std::endl;
			return;
		}

		// Find the number of entries.
		numEntries = position / sizeof(S_BOOKENTRY);
		std::cout << numEntries << " entries found in book." << std::endl;

		// Allocate numEntries of book entry structures in memory.
		entries = new S_BOOKENTRY[numEntries];

		// Find the start.
		rewind(pFile);

		// Read numEntries into entries with size sizeof(S_BOOKENTRY) each from pFile.
		size_t returnValue;
		returnValue = fread(entries, sizeof(S_BOOKENTRY), numEntries, pFile);

		std::cout << "fread() " << returnValue << " entries loaded into memory." << std::endl;
	}
}

void cleanPolyBook() {
	delete[] entries;
}



inline bool hasPawnForCapture(const S_BOARD* pos) {
	int sqWithPawn = 0;

	int attackPce = (pos->whitesMove == WHITE) ? WP : BP;

	if (pos->enPassantSquare != NO_SQ) {
		if (pos->whitesMove == WHITE) {
			sqWithPawn = pos->enPassantSquare - 8;
		}
		else {
			sqWithPawn = pos->enPassantSquare + 8;
		}

		if ((pos->pieceList[sqWithPawn + 1] == attackPce) && (((uint64_t)1 << sqWithPawn) & FILE_H) == 0) {
			return true;
		}
		else if ((pos->pieceList[sqWithPawn - 1] == attackPce) && (((uint64_t)1 << sqWithPawn) & FILE_A) == 0) {
			return true;
		}

	}
	return false;
}

uint64_t polyKeyFromBoard(const S_BOARD* pos) {

	uint64_t finalKey = 0;
	int offset = 0;
	int pce = NO_PIECE;
	int polyPiece = NO_PIECE;

	int file = 0;
	int rank = 0;

	for (int sq = 0; sq < 64; sq++) {
		pce = pos->pieceList[sq];
		if (pce != NO_PIECE) {
			ASSERT(pce >= wP && pce <= bK);

			polyPiece = polyPce[pce];

			rank = sq / 8;
			file = sq % 8;

			finalKey ^= Random64Poly[(64 * polyPiece) + (8 * rank) + file];
		}
	}

	// Castling
	offset = 768;

	if (((pos->castlePerms >> WKCA) & 1) == 1) {
		finalKey ^= Random64Poly[offset + 0];
	}
	if (((pos->castlePerms >> WQCA) & 1) == 1) {
		finalKey ^= Random64Poly[offset + 1];
	}
	if (((pos->castlePerms >> BKCA) & 1) == 1) {
		finalKey ^= Random64Poly[offset + 2];
	}
	if (((pos->castlePerms >> BQCA) & 1) == 1) {
		finalKey ^= Random64Poly[offset + 3];
	}


	// En-passant
	offset = 772;
	if (hasPawnForCapture(pos) == true) {
		file = pos->enPassantSquare % 8;
		
		finalKey ^= Random64Poly[offset + file];
	}

	// Side
	if (pos->whitesMove == WHITE) {
		finalKey ^= Random64Poly[780];
	}

	return finalKey;
}

inline uint16_t endian_swap_u16(uint16_t x) {
	x = (x >> 8) | (x << 8);
	return x;
}

inline uint32_t endian_swap_u32(uint32_t x) {
	x = (x >> 24) | ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | (x << 24);
	return x;
}

inline uint64_t endian_swap_u64(uint64_t x)
{
	x = (x >> 56) |
		((x << 40) & 0x00FF000000000000) |
		((x << 24) & 0x0000FF0000000000) |
		((x << 8) & 0x000000FF00000000) |
		((x >> 8) & 0x00000000FF000000) |
		((x >> 24) & 0x0000000000FF0000) |
		((x >> 40) & 0x000000000000FF00) |
		(x << 56);
	return x;
}

const char FILES[8] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h' };
const char RANKS[8] = { '1', '2', '3', '4', '5', '6', '7', '8' };

int ConvertPolyMove(uint16_t move, S_BOARD* pos) {

	int fromFile = (move >> 6) & 7;
	int fromRank = (move >> 9) & 7;
	int toFile = (move >> 0) & 7;
	int toRank = (move >> 3) & 7;

	int promoPce = (move >> 12) & 7;

	char moveStr[6];
	if (promoPce == 0) {
		sprintf_s(moveStr, "%c%c%c%c", 
			FILES[fromFile], 
			RANKS[fromRank], 
			FILES[toFile], 
			RANKS[toRank]);

	}
	else {
		char promChar = 'q';

		switch (promoPce) {
		case 1:
			promChar = 'n'; break;
		case 2:
			promChar = 'b'; break;
		case 3:
			promChar = 'r'; break;
		}

		sprintf_s(moveStr, "%c%c%c%c%c",
			FILES[fromFile],
			RANKS[fromRank],
			FILES[toFile],
			RANKS[toRank],
			promChar);
	}
	return parseMove(moveStr, pos);
}

int getBookMove(S_BOARD* pos) {
	uint64_t polyKey = polyKeyFromBoard(pos);

	int index = 0;
	S_BOOKENTRY* entry;
	uint16_t move = 0;

	const int MAXBOOKMOVES = 32;
	int bookMoves[MAXBOOKMOVES] = {};
	int tempMove = NOMOVE;
	int count = 0;

	for (int i = 0; i < numEntries; i++) {
		if (polyKey == endian_swap_u64(entries[i].key)) {
			move = endian_swap_u16(entries[i].move);
			tempMove = ConvertPolyMove(move, pos);
			if (tempMove != NOMOVE) {
				bookMoves[count] = tempMove;
				count++;
				if (count > MAXBOOKMOVES) {
					break;
				}
			}
		}
		index++;
	}

	if (count != 0) {
		int randMove = rand() % count;
		return bookMoves[randMove];
	}
	else {
		return NOMOVE;
	}
}
