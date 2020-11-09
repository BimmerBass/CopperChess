#include "defs.h"


// IT SEVERELY BLUNDERS IN THIS POSITION WITH 2 SECONDS MOVETIME
#define FUCKyouFEN "3rr3/pbp4p/2p1kpp1/4n1P1/2p1Pp1P/P2P1N2/1PPB2K1/3RR3 w - - 0 36"

int main() {
	initAll(SetMask, ClearMask);

	Uci_Loop();

	cleanPolyBook();
	return 0;
}
