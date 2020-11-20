#include "defs.h"


// IT SEVERELY BLUNDERS IN THIS POSITION WITH 2 SECONDS MOVETIME
#define FUCKyouFEN "3rr3/pbp4p/2p1kpp1/4n1P1/2p1Pp1P/P2P1N2/1PPB2K1/3RR3 w - - 0 36"

// IT GIVES AWAY THE QUEEN IN THIS POSITION WITH 5 SECONDS MOVETIME
#define WEIRD "2b2rk1/p3qppp/5n2/4p1B1/7Q/2N3P1/Pr2PP1P/R3K2R w KQ - 0 16"


int main() {
	initAll(SetMask, ClearMask);

	Uci_Loop();

	cleanPolyBook();
	return 0;
}
