#include "defs.h"


S_EVALCACHE::S_EVALCACHE(uint64_t size, bool gigabytes){
	numEntries = (gigabytes == true) ? (GB(size) / sizeof(S_EVALENTRY)) : (MB(size) / sizeof(S_EVALENTRY));

	entry = new S_EVALENTRY[numEntries];

	// Clear the cache
	for (int i = 0; i < numEntries; i++) {
		entry[i].posKey = 0;
		entry[i].score = 0;
	}
#if defined(COPPER_VERBOSE)
	std::cout << "Initialized evaluation cache with " << numEntries << " entries." << std::endl;
#endif
}

S_EVALCACHE::~S_EVALCACHE(){
	delete[] entry;
}

bool S_EVALCACHE::probeCache(const S_BOARD* pos, int& score) {
	int index = pos->posKey % numEntries;

	if (entry[index].posKey == pos->posKey) {
		score = entry[index].score;
		return true;
	}
	return false;
}


void S_EVALCACHE::storeEvaluation(const S_BOARD* pos, int& score) {
	int index = pos->posKey % numEntries;

	entry[index].posKey = pos->posKey;
	entry[index].score = score;
}

void S_EVALCACHE::clearCache() {
	for (int index = 0; index < numEntries; index++) {
		entry[index].posKey = 0;
		entry[index].score = 0;
	}
}

void S_EVALCACHE::resize_cache(uint64_t mb_size) {
	// Delete the old cache
	delete[] entry;

	numEntries = (MB(mb_size) / sizeof(S_EVALENTRY));

	// Now create a new cache
	try {
		entry = new S_EVALENTRY[numEntries];
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "std::bad_alloc thrown while resizing evaluation cache: " << ba.what() << std::endl;
	}
#if defined(COPPER_VERBOSE)
	std::cout << "Resized evaluation cache to " << mb_size << "MB with " << numEntries << " entries" << std::endl;
#endif
}


void S_EVALCACHE::prefetch_cache(const S_BOARD* pos) {
	prefetch(&entry[pos->posKey % numEntries]);
}