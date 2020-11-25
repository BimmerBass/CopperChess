#include "defs.h"


PawnHashTable::PawnHashTable(uint64_t size) {
	
	numEntries = MB(size) / sizeof(PawnHashEntry);

	entries = new PawnHashEntry[numEntries];

	clear_hash();

	std::cout << "Initialized pawn hash table (" << size << "MB) with " << numEntries << " entries." << std::endl;
}

PawnHashTable::~PawnHashTable() {
	delete[] entries;
}

bool PawnHashTable::probe_pawn_hash(const S_BOARD* pos, int* ev) {
	int index = pos->pawnKey % numEntries;

	if (entries[index].pawnKey == pos->pawnKey) { // This entry holds the same pawn-structure
		*ev = entries[index].eval;
		return true;
	}

	return false;
}

void PawnHashTable::store_pawn_eval(const S_BOARD* pos, int* ev){
	int index = pos->pawnKey % numEntries;

	entries[index].pawnKey = pos->pawnKey;
	entries[index].eval = *ev;
}

void PawnHashTable::clear_hash() {
	for (int entry = 0; entry < numEntries; entry++) {
		entries[entry].pawnKey = 0;
		entries[entry].eval = 0;
	}
}