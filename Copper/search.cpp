#include "defs.h"
#include "evaluation.h"


#define ASPIRATION 20 // Size of initial aspiration windows.


#define ENDGAME_MAT 1350 // The lower this value becomes, the more delta-cutoffs we get. It is a fine balance between accuracy and speed in quiescence search.
#define DELTA 200 // The delta safety margin for quiescence search. Raise this to get a more accurate quiescence evaluation at the cost of lowered speed.
#define RAZOR_MARGIN 600 // The security margin for razoring. The value is taken from Stockfish's implementation.
#define CONTEMPT 10 // The fctor by which we'll multiply the winning probability of the position.
#define CONTEMPT_SCALING_CONSTANT 2.5 // This is a scaling constant that makes the function for winnning propability more or less sensitive.

/*
INCLUDES THE FUNCTIONS:
	- isRepetition
*/


void Search::CheckUp(S_SEARCHINFO* info) {
	if (info->timeset == true && getTimeMs() > info->stoptime) {
		info->stopped = true;
	}
	ReadInput(info);
}


bool isRepetition(const S_BOARD* pos) {
	for (int i = 0; i < pos->history.moveCount; i++) {
		if (pos->history.history[i].key == pos->posKey) {
			return true;
		}
	}
	return false;
}



int Search::reduction(bool improving, int depth, int moveCount) {
	int r = Reductions[depth] * Reductions[moveCount];
	return ((r + 512) / 1024 + (!improving && r > 1024)) * 1;
}

int Search::contempt_factor(const S_BOARD* pos) {
	// We will get a static evaluation and convert it from centipawns to pawns.
	int static_eval = eval::staticEval(pos, -INF, INF) / 100;

	// The win probability is taken from https://www.chessprogramming.org/Pawn_Advantage,_Win_Percentage,_and_Elo
	double win_prob = 1 / (1 + pow(10, - (static_eval / CONTEMPT_SCALING_CONSTANT)));

	return (int)(- CONTEMPT * win_prob);
}


void Search::pickNextMove(int index, S_MOVELIST* legalMoves){
	S_MOVE temp;
	S_MOVE bestMove;
	int bestNum = index;
	
	int score = 0;
	
	for (int i = bestNum; i < legalMoves->count; i++){
		// If the current move is more promising than a previous one.
		if (legalMoves->moves[i].score > score){
			// Get the index of the move.
			bestNum = i;
			
			// Update score such that next iterations can only find moves with higher scores.
			score = legalMoves->moves[i].score;
		}
	}
	// Find the current value at index.
	temp = legalMoves->moves[index];
	
	// Replace index value with the most promising move.
	legalMoves->moves[index] = legalMoves->moves[bestNum];

	// Insert old move into the index where the best move was found.
	legalMoves->moves[bestNum] = temp;
}

const int pieceValsMg[12] = { eval::pawnValMg, eval::knightValMg, eval::bishopValMg, eval::rookValMg, eval::queenValMg, eval::kingValMg,
							eval::pawnValMg, eval::knightValMg, eval::bishopValMg, eval::rookValMg, eval::queenValMg, eval::kingValMg };
bool badCapture(const S_BOARD* pos, int move) {
	int piece_moved = pos->pieceList[FROMSQ(move)];
	int piece_captured = pos->pieceList[TOSQ(move)];
	if (piece_captured == NO_PIECE) { return false; }

	// Pawn captures can't loose material
	if (piece_moved == WP || piece_moved == BP) {
		return false;
	}

	
	// Capture lower takes higher is better, as well as bishop takes knight
	if (pieceValsMg[piece_captured] >= pieceValsMg[piece_moved]) {
		return false;
	}

	S_SIDE Them = (pos->whitesMove == WHITE) ? BLACK : WHITE;
	if (defending_pawns(pos, TOSQ(move), Them) > 0
		&& pieceValsMg[piece_captured] + 200 - pieceValsMg[piece_moved] < 0) {
		return true;
	}

	// If this isn't a capture, it cannot be considered bad.
	return false;
}

int Search::Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info) {
	int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return contempt_factor(pos);
	}

	if (pos->ply > MAXDEPTH - 1) {
		return eval::staticEval(pos, alpha, beta);
	}

	int stand_pat = eval::staticEval(pos, alpha, beta);

	if (stand_pat >= beta) {
		return stand_pat;
	}
	else if (alpha < stand_pat) {
		alpha = stand_pat;
	}

	S_MOVELIST list;
	MoveGeneration::validMoves(pos, list);

	if (list.count == 0) {
		if (sqAttacked(kingSq, !pos->whitesMove, pos)) {
			return -INF + pos->ply;
		}
		else {
			return 0;
		}
	}

	int score = -INF;
	int captures = 0;

	for (int moveNum = 0; moveNum < list.count; moveNum++) {
		pickNextMove(moveNum, &list);

		// We'll only search captures (+en-passant) and promotions
		if (pos->pieceList[TOSQ(list.moves[moveNum].move)] != NO_PIECE || SPECIAL(list.moves[moveNum].move) == 0
			|| SPECIAL(list.moves[moveNum].move) == 1) {
			captures++;

			/*
			DELTA PRUNING
			*/

			if ((stand_pat + eval::pieceValMg[pos->pieceList[TOSQ(list.moves[moveNum].move)]] + DELTA < alpha)
				&& (eval::getMaterial(pos, !pos->whitesMove) - eval::pieceValMg[pos->pieceList[TOSQ(list.moves[moveNum].move)]] > ENDGAME_MAT)
				&& SPECIAL(list.moves[moveNum].move) != 0) {
				continue;
			}

			/*
			END OF DELTA PRUNING
			*/

			// If the capture looses material immediately, we will not search it, as it probably also looses in the long run.
			if (badCapture(pos, list.moves[moveNum].move)
				&& SPECIAL(list.moves[moveNum].move != 0)) {
				continue;
			}

			MoveGeneration::makeMove(*pos, list.moves[moveNum].move);

			score = -Quiescence(-beta, -alpha, pos, info);

			MoveGeneration::undoMove(*pos);

			if (info->stopped == true) { return 0; }

			if (score > alpha) {
				if (score >= beta) {
					if (captures == 1) {
						info->fhf++;
					}
					info->fh++;
					return beta;
				}
				alpha = score;
			}
		}
	}

	return alpha;
}

int Search::alphabeta(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta, bool doNull, bool extend) {
	bool improving = true;
	int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];

	/*
	MATE DISTANCE PRUNING
	*/
	if (alpha < -(INF - pos->ply)) { alpha = -(INF - pos->ply); }
	if (beta > ((INF - pos->ply) - 1)) { beta = (INF - pos->ply) - 1; }
	if (alpha >= beta) { return alpha; }
	/*
	END OF MATE DISTANCE PRUNING
	*/

	/*
	IN CHECK EXTENSION
		- If we are in check, we want to search the entire sequence as it is very forcing. Therefore we increase the search depth by 1.
	*/
	if (pos->inCheck) { depth++; extend = true; improving = false; }

	/*
	QUIESCENCE SEARCH
		- If we are in a leaf node, we will search all captures, promotions and checks such that we only evaluate a quiet position.
	*/
	if (depth == 0) {
		return Quiescence(alpha, beta, pos, info);
		
	}

	int bestScore = -INF;
	int bestMove = NOMOVE;
	int value = -INF;
	int oldAlpha = alpha;

	// By default futility pruning should not be allowed.
	bool f_prune = false;
	bool flagInCheck = pos->inCheck; // We won't do LMR if we are getting out of check. Will be used after making a move.

	// If we have searched a certain number of nodes, we need to check if the GUI has sent a "stop" or "quit" command.
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	// If this position has been reached before, we need not explore it further as it can be a draw.
	// Therefore we will return a contempt factor, that is negative if Copper is winning, and positive if Copper is loosing.
	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return contempt_factor(pos);
	}

	if (pos->ply > MAXDEPTH - 1) {
		return eval::staticEval(pos, alpha, beta);
	}

	// We will probe the transposition table, and if it has a value for the position, we dont need to search it.
	if (TT::probePos(pos, depth, alpha, beta, &bestMove, &value) == true) {
		return value;
	}

	S_MOVELIST list;
	MoveGeneration::validMoves(pos, list); // Generate all legal moves for the position. Later on, this will be all pseudo-legal moves
												// and we'll then determine a move's legality by trying to make it and see if we're in check.
	
	// If there are no legal moves, it is either a checkmate or stalemate.
	// We'll also return the contempt factor for stalemate, since if we have a queen and king vs king, it'll be very bad if we end up in stalemate.
	// And vice versa.
	if (list.count == 0) {
		if (sqAttacked(kingSq, !pos->whitesMove, pos)) {
			return -INF + pos->ply; // Checkmate at this ply-depth
		}
		else {
			return contempt_factor(pos); // stalemate
		}
	}


	if (depth < 3 && !pos->inCheck) {
		int staticEval = eval::staticEval(pos, alpha, beta);


		/*
		EVAL PRUNING
			- If beta is not close to a checkmate score, we can return the static evaluation.
				with a safety margin of 120cp multiplied by the depth, but only if their difference exceeds beta.
		*/
		if (abs(beta - 1) > -INF + 100) {

			int eval_margin = 120 * depth;

			if (staticEval - eval_margin >= beta) {
				return (staticEval - eval_margin);
			}
		}

		/*
		STATIC NULL MOVE PRUNING
		*/
		if (!doNull && abs(beta) <= MATE) {
			if (depth == 1 && staticEval - 300 > beta) return beta;
			if (depth == 2 && staticEval - 525 > beta) return beta;
			if (depth == 3 && staticEval - 900 > beta) depth--;
		}

	}


	/*
	
	NULL MOVE PRUNING

	*/
	
	if ((depth > 2)
		&& doNull
		&& (eval::staticEval(pos, alpha, beta)) >= beta
		&& !pos->inCheck) {

		// FIXME: This boolean endgame-determination value probably needs to be changed.
		bool endgame = ((pos->position[WQ] | pos->position[BQ]) == 0 || (pos->position[WN] | pos->position[WB] | pos->position[WR]
			| pos->position[BN] | pos->position[BB] | pos->position[BR]) == 0) ? true : false;
		if (!endgame) {
			int oldEp = pos->enPassantSquare;

			makeNullMove(pos);

			int R = 2;
			if (depth > 6) { R = 3; }

			value = -alphabeta(pos, info, depth - R - 1, -beta, -beta + 1, false, extend);

			undoNullMove(pos, oldEp);

			if (info->stopped == true) { return 0; }
			if (value >= beta && abs(value) < MATE) { return beta; }
			
			/*if (value >= beta) { // NULL MOVE REDUCTIONS. We'll have to test this further before implementing it
				depth -= 3;

				if (depth <= 0) {
					return Quiescence(alpha, beta, pos, info);
				}

			}*/

		}

	}

	/*
	
	END OF NULL MOVE PRUNING

	*/


	// Futility pruning and razoring
	if (depth <= 3 && !pos->inCheck) {
		int eval = eval::staticEval(pos, alpha, beta);

		// We'll only do razoring if depth = 2, the eval is below alpha by some margin, we arent extending and there isn't a PV-move
		if (eval + RAZOR_MARGIN < beta) { // Likely a fail-low node
			int new_val = Quiescence(alpha, beta, pos, info);
			if (new_val < beta) { return new_val; }
		}

		if (eval + fMargin[depth] <= alpha && abs(alpha) < 9000) {
			f_prune = true;
		}
	}

	// Score the transposition table move highest, so it'll be searched first.
	if (bestMove != NOMOVE) {
		for (int i = 0; i < list.count; i++) {
			if (list.moves[i].move == bestMove) {
				list.moves[i].score = 4000000;
				break;
			}
		}
	}

	int reduction_depth = 0;
	int new_depth = 0;
	int raised_alpha = 0;
	value = bestScore;

	int moves_tried = 0;
	for (int moveNum = 0; moveNum < list.count; moveNum++) {
		pickNextMove(moveNum, &list);

		MoveGeneration::makeMove(*pos, list.moves[moveNum].move);

		// FUTILITY PRUNING
		if (f_prune && pos->pieceList[TOSQ(list.moves[moveNum].move)] == NO_PIECE && SPECIAL(list.moves[moveNum].move) != 0
			&& !pos->inCheck) {
			MoveGeneration::undoMove(*pos);
			continue;
		}

		moves_tried++;

		// LATE MOVE REDUCTION
		reduction_depth = 0;
		new_depth = depth - 1;

		// When PVS is introduced, we can try to do LMR at moves_tried > 1, because we can be more confident that we're in a PV node.
		if (new_depth >= 3
			&& moves_tried > 3 
			&& !pos->inCheck
			&& !flagInCheck &&
			pos->killerMoves[pos->ply][0] != list.moves[moveNum].move
			&& pos->killerMoves[pos->ply][1] != list.moves[moveNum].move
			&& pos->pieceList[TOSQ(list.moves[moveNum].move)] != NO_PIECE
			&& !(SPECIAL(list.moves[moveNum].move) == 0 || SPECIAL(list.moves[moveNum].move) == 1)) {

			reduction_depth = reduction(improving, new_depth, moves_tried);

			if (moves_tried > 8) {
				reduction_depth += 1;
			}

			new_depth -= reduction_depth;
		}

		re_search:

		value = -alphabeta(pos, info, new_depth, -beta, -alpha, true, extend);


		// Sometimes reduced search brings us above alpha. Then we'll have to retry
		if (reduction_depth && value > alpha) {
			new_depth += reduction_depth;
			reduction_depth = 0;
			goto re_search;
		}

		MoveGeneration::undoMove(*pos);

		if (info->stopped == true) { return 0; }

		if (value > alpha) {
			bestMove = list.moves[moveNum].move;

			if (value >= beta) {
				if (moveNum == 0) {
					info->fhf++;
				}
				info->fh++;

				// Update killer moves*
				if (pos->pieceList[TOSQ(list.moves[moveNum].move)] == NO_PIECE) { // Move is not a capture
					pos->killerMoves[pos->ply][1] = pos->killerMoves[pos->ply][0]; // move first move to the last index
					pos->killerMoves[pos->ply][0] = list.moves[moveNum].move; // Replace first move with this move.


					// Update the history heuristics
					pos->historyHeuristic[pos->pieceList[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth * depth;

					// If the search is super deep, the history table will overflow, so we'll half all the values if this happens at one of them.
					// This is done to prevent the history heuristic from surpassing the killer moves heuristic.
					// Therefore, we will not let the history-values surpass the minimal value added to killer moves.
					if (pos->historyHeuristic[pos->pieceList[FROMSQ(bestMove)]][TOSQ(bestMove)] >= 800000) {
						for (int pce = 0; pce < 12; pce++) {
							for (int sq = 0; sq < 64; sq++) {
								pos->historyHeuristic[pce][sq] = pos->historyHeuristic[pce][sq] / 2;
							}
						}
					}

				}

				TT::storeEntry(pos, bestMove, depth, UPPER, beta);

				return beta;
			}

			raised_alpha = 1;
			alpha = value;
			bestScore = value;
		}
	}

	if (alpha != oldAlpha) {
		TT::storeEntry(pos, bestMove, depth, EXACT, bestScore);
	}
	else {
		TT::storeEntry(pos, bestMove, depth, LOWER, alpha);
	}
	return bestScore;
}

inline void info_currmove(int move, int depth, int moveNum) {
	std::string moveStr = printMove(move);
	std::cout << "info depth " << depth << " currmove " << moveStr << " currmovenumber " << (moveNum + 1) << std::endl;
	return;
}


int Search::searchRoot(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta) {
	bool extend = false;
	if (pos->inCheck) { depth++; extend = true; }

	S_MOVELIST moves;
	MoveGeneration::validMoves(pos, moves);

	int bestMove = NOMOVE;
	int bestScore = -INF;
	int value = -INF;
	int oldAlpha = alpha;

	if (moves.count == 0) {
		int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];
		if (sqAttacked(kingSq, !pos->whitesMove, pos) == true) {
			return -INF;
		}
		else {
			return 0;
		}
	}

	// Check for the best move at the moment, and score this highest (if there's any)
	int pvMove = TT::probePvMove(pos);
	if (pvMove != NOMOVE) {
		for (int i = 0; i < 64; i++) {
			if (moves.moves[i].move == pvMove) {
				moves.moves[i].score = 4000000;
				break;
			}
		}
	}


	for (int i = 0; i < moves.count; i++) {
		pickNextMove(i, &moves);

		MoveGeneration::makeMove(*pos, moves.moves[i].move);

		info_currmove(moves.moves[i].move, depth, i);

		value = -alphabeta(pos, info, depth - 1, -beta, -alpha, true, extend);

		MoveGeneration::undoMove(*pos);

		if (value > alpha) {
			bestMove = moves.moves[i].move;
			//bestScore = value;
			if (value >= beta) {
				if (i == 0) {
					info->fhf++;
				}
				info->fh++;

				TT::storeEntry(pos, bestMove, depth, UPPER, beta);

				return beta;
			}

			TT::storeEntry(pos, bestMove, depth, LOWER, value);
			alpha = value;
			bestScore = alpha;
		}
	}

	TT::storeEntry(pos, bestMove, depth, EXACT, bestScore);
	return alpha;
}


// Aspiration window search. Inspired by Stockfish's implementation.
int Search::search_widen(S_BOARD* pos, S_SEARCHINFO* info, int depth, int estimate) {
	int alpha = -INF;
	int beta = INF;
	int delta = alpha;
	int v = -INF;

	if (depth >= 5){
		delta = ASPIRATION;
		alpha = max(estimate - delta, -INF);
		beta = min(estimate + delta, INF);
	}

asp_search:
	v = searchRoot(pos, info, depth, alpha, beta);

	if (v <= alpha) {
		beta = (alpha + beta) / 2;
		alpha = max(v - delta, -INF);

		delta += (delta / 4) + 5;
		goto asp_search;
	}
	else if (v >= beta) {
		beta = min(v + delta, INF);
		delta += (delta / 4) + 5;
		goto asp_search;
	}

	return v;
}


void Search::searchPosition(S_BOARD* pos, S_SEARCHINFO* info) {
	int bestMove = NOMOVE;
	int pvMoves = 0;

	clearForSearch(pos, info);

	bestMove = getBookMove(pos); // Probe the opening book for a move.
	if (bestMove != NOMOVE) {
		std::cout << "Found a book move!" << std::endl;
	}
	// Search to depth 1 to get estimate of value, and pad this to get the aspiration window.
	int score = searchRoot(pos, info, 1, -INF, INF);
	long long nps = 0;
	int mateDist = 0;

	// Iterative deepening.
	if (bestMove == NOMOVE) { // Go into iterative deepening if we didn't find a book move.
		for (int currDepth = 1; currDepth <= info->depth; currDepth++) {

			mateDist = 0;

			score = search_widen(pos, info, currDepth, score);

			//score = MTDF(pos, info, score, currDepth);

			nps = (info->nodes) / ((getTimeMs() - info->starttime) * 0.001);

			if (score > MATE) { // We can deliver checkmate
				mateDist = (INF - score) / 2;
				/*
				int side = (pos->whitesMove == WHITE) ? 1 : -1;
				mateDist = side * (INFINITE - score) / 2;*/
			}
			else if (score < -MATE) { // We are being checkmated.
				mateDist = -(INF - score) / 2;
				/*
				int side = (pos->whitesMove == WHITE) ? -1 : 1;
				mateDist = side * (score + INFINITE) / 2;*/
			}

			if (info->stopped == true) {
				break; // Break out if the GUI has interrupted the search
			}

			pvMoves = TT::getPvLine(pos, currDepth);
			bestMove = pos->pvArray[0];

			if (mateDist != 0) { // If there has been found a mate, print score in mate distance instead of centipawns
				std::cout << "info score mate " << mateDist << " depth " << currDepth
					<< " nodes " << info->nodes << " time " << getTimeMs() - info->starttime << " nps " << nps;

				std::cout << " pv ";
				for (int i = 0; i < pvMoves; i++) {
					std::cout << printMove(pos->pvArray[i]) << " ";
				}
				std::cout << "\n";
			}


			else {
				std::cout << "info score cp " << score << " depth " << currDepth
					<< " nodes " << info->nodes << " time " << getTimeMs() - info->starttime << " nps " << nps;

				std::cout << " pv ";
				for (int i = 0; i < pvMoves; i++) {
					std::cout << printMove(pos->pvArray[i]) << " ";
				}
				std::cout << "\n";
			}
			std::cout << "Ordering: " << (info->fhf / info->fh)*100 << "%" << std::endl;

		}
	}
	
	std::cout << "bestmove " << printMove(bestMove) << "\n";
}


void Search::clearForSearch(S_BOARD* pos, S_SEARCHINFO *info){
	pos->ply = 0;
	

	info->stopped = false;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;

	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 64; j++) {
			pos->historyHeuristic[i][j] = 0;
		}
	}

	for (int i = 0; i < 64; i++) {
		pos->killerMoves[i][0] = 0;
		pos->killerMoves[i][1] = 0;
	}

}
