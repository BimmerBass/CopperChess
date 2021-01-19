#include "defs.h"
#include "genetics.h"



int main() {
	initAll(SetMask, ClearMask);


	Uci_Loop();
	

	cleanPolyBook();
	return 0;
}
