#include "defs.h"



int main() {
	initAll(SetMask, ClearMask);

	Uci_Loop();

	cleanPolyBook();
	return 0;
}
