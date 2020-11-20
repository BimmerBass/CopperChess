#include "evaluation.h"

/*
INCLUDES THE FUNCTIONS:
	- staticEval(S_BOARD* board)
	- getMaterial(const S_BOARD* pos, bool side)
*/

// Alpha and beta are added because lazy evaluation will be added in the future.
int eval::staticEval(const S_BOARD* pos, int alpha, int beta) {
	int v_main = 0;
	int v_mg = 0;
	int v_eg = 0;

	// Check to see if we get a hit from the evaluation cache.
	if (pos->evaluationCache->probeCache(pos, v_main) == true) {
		return v_main;
	}

	// Calculate the game-phase, the middlegame weight and the endgame weight
	int p = phase(pos);
	if (p > 24) { p = 24; }
	int weight_mg = p;
	int weight_eg = 24 - p;

	// Get the middle- and endgame evaluations.
	v_mg += mg_evaluate(pos, alpha, beta);
	v_eg += eg_evaluate(pos, alpha, beta);

	v_main = ((v_mg * weight_mg) + (v_eg * weight_eg)) / 24;

	v_main += imbalance(pos);

	v_main += (pos->whitesMove == WHITE) ? tempo : -tempo;

	// Make v_main relative to the side to move.
	v_main *= (pos->whitesMove == WHITE) ? 1 : -1;

	// Store the evaluation in the evaluation cache
	pos->evaluationCache->storeEvaluation(pos, v_main);

	return v_main;
}
