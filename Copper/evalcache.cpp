#include "defs.h"


S_EVALCACHE::S_EVALCACHE(uint64_t size, bool gigabytes){
	numEntries = (gigabytes == true) ? (GB(size) / sizeof(S_EVALENTRY)) : (MB(size) / sizeof(S_EVALENTRY));

	entry = new S_EVALENTRY[numEntries];

	// Clear the cache
	for (int i = 0; i < numEntries; i++) {
		entry[i].posKey = 0;
		entry[i].score = 0;
	}

	std::cout << "Initialized evaluation cache with " << numEntries << " entries." << std::endl;
}

S_EVALCACHE::~S_EVALCACHE(){
	delete[] entry;
}

/*
bool probeCache(const S_BOARD* pos, int& score);
void storeEvaluation(const S_BOARD* pos, int score);
*/

bool S_EVALCACHE::probeCache(const S_BOARD* pos, int& score) {
	int index = pos->posKey % numEntries;

	if (entry[index].posKey == pos->posKey) {
		score = entry[index].score;
		return true;
	}
	return false;
}


void S_EVALCACHE::storeEvaluation(const S_BOARD* pos, int score) {
	int index = pos->posKey % numEntries;

	entry[index].posKey = pos->posKey;
	entry[index].score = score;
}

