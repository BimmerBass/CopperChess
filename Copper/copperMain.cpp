#include "defs.h"

#define FEN "3R2Q1/p1p5/1pk5/4K2p/4P3/7P/5rP1/8 b - - 0 39" // Can't find move in this position, and loses because of it...
#define FEN1 "r1bqr1k1/ppp1bppp/2n2n2/3p4/2P5/2NP1N2/PP1BBPPP/R2QR1K1 w - - 4 15" // Apparently also in this one.
#define FEN2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "

int main() {
	initAll(SetMask, ClearMask);



	Uci_Loop();

	cleanPolyBook();
	return 0;
}
