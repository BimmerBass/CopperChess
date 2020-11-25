#pragma once
#include "defs.h"


struct PawnHashEntry {
	uint64_t pawnKey;
	int eval;
};

class PawnHashTable {
public:
	PawnHashTable(uint64_t size);

	~PawnHashTable();

	bool probe_pawn_hash(const S_BOARD* pos, int* ev);

	void store_pawn_eval(const S_BOARD* pos, int* ev);

	void clear_hash();

private:
	PawnHashEntry* entries = nullptr;

	int numEntries = 0;
};