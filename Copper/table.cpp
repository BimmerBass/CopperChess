#include "defs.h"


// Initialize.
S_TABLE::S_TABLE(uint64_t size, bool gigabytes){
	
	// Calculate numEntries and allocate memory for this number of S_TTENTRY
	if (gigabytes) {
		numEntries = (GB(size)) / sizeof(S_TTENTRY);
	}
	else{
		numEntries = (MB(size)) / sizeof(S_TTENTRY);
	}
	tableEntry = new S_TTENTRY[numEntries];
	
	TT::clearTable(this);

#if defined(COPPER_VERBOSE)
	std::cout << "Initialized transposition table with " << numEntries << " entries." << std::endl;
#endif
}

S_TABLE::~S_TABLE() {
	delete[] tableEntry;
}


void S_TABLE::resize(uint64_t mb_size) {
	// To resize the table, we have to first delete the contents and then write new ones.
	delete[] tableEntry;
	numEntries = (MB(mb_size) / sizeof(S_TTENTRY));

	try {
		tableEntry = new S_TTENTRY[numEntries];
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "std::bad_alloc error caught: " << ba.what() << "\n";
	}
#if defined(COPPER_VERBOSE)
	std::cout << "Resized transposition table with " << numEntries << " entries." << std::endl;
#endif
}


void TT::clearTable(S_TABLE* table){
	for (int i = 0; i < table->numEntries; i++){
		table->tableEntry[i].posKey = (uint64_t)0;
		table->tableEntry[i].flag = LOWER;
		table->tableEntry[i].move = NOMOVE;
		table->tableEntry[i].score = 0;
		table->tableEntry[i].depth = 0;
	}

	table->overWrite = 0;
	table->hit = 0;
	table->cut = 0;
	table->newWrite = 0;
}


void TT::storeEntry(S_BOARD* pos, int move, int depth, TT_FLAG flg, int score){
	int index = pos->posKey % pos->transpositionTable->numEntries;

	if (pos->transpositionTable->tableEntry[index].posKey == 0) {
		pos->transpositionTable->newWrite++;
	}
	else {
		pos->transpositionTable->overWrite++;
	}

	if (score > MATE) {
		score += pos->ply;
	}
	else if (score < -MATE) {
		score -= pos->ply;
	}

	pos->transpositionTable->tableEntry[index].posKey = pos->posKey;
	pos->transpositionTable->tableEntry[index].move = move;
	pos->transpositionTable->tableEntry[index].score = score;
	pos->transpositionTable->tableEntry[index].depth = depth;
	pos->transpositionTable->tableEntry[index].flag = flg;
	
}




S_TTENTRY* TT::extract_entry(const S_BOARD* pos, bool& ttHit) {
	int index = pos->posKey % pos->transpositionTable->numEntries;

	// We have the position in the transposition table.
	if (pos->transpositionTable->tableEntry[index].posKey == pos->posKey) {
		ttHit = true;
		return &pos->transpositionTable->tableEntry[index];
	}

	ttHit = false;
	return nullptr;
}





int TT::probePos(const S_BOARD* pos, int depth, int alpha, int beta, int* move, int* score) {
	int index = pos->posKey % pos->transpositionTable->numEntries;

	if (pos->transpositionTable->tableEntry[index].posKey == pos->posKey) {
		*move = pos->transpositionTable->tableEntry[index].move;
		
		*score = pos->transpositionTable->tableEntry[index].score;

		if (*score > MATE) { *score -= pos->ply; }
		else if (*score < -MATE) { *score += pos->ply; }


		if (pos->transpositionTable->tableEntry[index].depth >= depth) {
			pos->transpositionTable->hit++;

			switch (pos->transpositionTable->tableEntry[index].flag) {
			case LOWER:
				if (*score <= alpha) {
					*score = alpha;
					return true;
				}
				break;

			case UPPER:
				if (*score >= beta) {
					*score = beta;
					return true;
				}
				break;
			case EXACT:
				return true;
				break;
			}
		}
	}
	return false;
}

int TT::probePvMove(const S_BOARD* pos) {
	int index = pos->posKey % pos->transpositionTable->numEntries;

	
	if (pos->posKey == pos->transpositionTable->tableEntry[index].posKey) {
		if (pos->transpositionTable->tableEntry[index].flag == EXACT) {
			return pos->transpositionTable->tableEntry[index].move;
		}
	}
	return NOMOVE;
}

int TT::getPvLine(S_BOARD* pos, int depth){
	pos->ply = 0;
	int move = probePvMove(pos);
	
	int count = 0;
	
	while (move != NOMOVE && count < depth){
		// Make the move and store it.
		if (moveExists(pos, move)) { // If the move is valid in the position
			MoveGeneration::makeMove(*pos, move);
			pos->pvArray[count] = move;
			count++;

			// Find the next possible move.
			move = probePvMove(pos);
			continue;
		}
		break;
	}
	
	// Undo all the moves that has been made.
	while (pos->ply > 0) {
		MoveGeneration::undoMove(*pos);
	}
	
	return count;
}
