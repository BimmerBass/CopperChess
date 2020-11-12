#include "defs.h"
#include "evaluation.h"


// IT SEVERELY BLUNDERS IN THIS POSITION WITH 2 SECONDS MOVETIME
#define FUCKyouFEN "3rr3/pbp4p/2p1kpp1/4n1P1/2p1Pp1P/P2P1N2/1PPB2K1/3RR3 w - - 0 36"

#define TEST_WHITE "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define TEST_BLACK "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 "

int main() {
	initAll(SetMask, ClearMask);

	/*S_BOARD wPos;
	S_BOARD bPos;

	BoardRep::parseFen(TEST_WHITE, wPos);
	BoardRep::parseFen(TEST_BLACK, bPos);

	std::cout << eval::staticEval(&wPos, 0, 0, 0) << " : " << eval::staticEval(&bPos, 0, 0, 0) << std::endl;
	*/
	Uci_Loop();

	cleanPolyBook();
	return 0;
}
