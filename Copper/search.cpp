#include "defs.h"
#include "evaluation.h"


#define ASPIRATION 20 // Size of initial aspiration windows.


#define ENDGAME_MAT 1350 // The lower this value becomes, the more delta-cutoffs we get. It is a fine balance between accuracy and speed in quiescence search.
#define DELTA 200 // The delta safety margin for quiescence search. Raise this to get a more accurate quiescence evaluation at the cost of lowered speed.
#define RAZOR_MARGIN 600 // The security margin for razoring. The value is taken from Stockfish's implementation.

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


void Search::pickNextMove(int index, S_MOVELIST* legalMoves){
	S_MOVE temp;
	S_MOVE bestMove;
	int bestNum = index;
	
	int score = 0;
	
	for (int i = 0; i < legalMoves->count; i++){
		// If the current move is more promising than a previous one.
		if (legalMoves->moves[i].score > score){
			// Get the move
			bestMove = legalMoves->moves[i];
			// Get the index of the move.
			bestNum = i;
			
			// Update score such that next iterations can only find moves with higher scores.
			score = legalMoves->moves[i].score;
		}
	}
	// Find the current value at index.
	temp = legalMoves->moves[index];
	
	// Replace index value with the most promising move.
	legalMoves->moves[index] = bestMove;
	legalMoves->moves[index].score = -1000000;

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
	int side = (pos->whitesMove == WHITE) ? 1 : -1;
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1) {
		return side*eval::staticEval(pos, 0, alpha, beta);
	}

	int stand_pat = side*eval::staticEval(pos, 0, alpha, beta);

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
	int side = (pos->whitesMove == WHITE) ? 1 : -1;
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

	if (pos->inCheck) { depth++; extend = true; }
	if (depth == 0) {
		return Quiescence(alpha, beta, pos, info);
		
	}

	int bestScore = -INF;
	int bestMove = NOMOVE;
	int value = -INF;
	int oldAlpha = alpha;

	bool f_prune = false;
	bool flagInCheck = pos->inCheck; // We won't do LMR if we are getting out of check

	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1) {
		return side*eval::staticEval(pos, depth, alpha, beta);
	}

	if (TT::probePos(pos, depth, alpha, beta, &bestMove, &value) == true) {
		return value;
	}

	S_MOVELIST list;
	MoveGeneration::validMoves(pos, list);
	
	if (list.count == 0) {
		if (sqAttacked(kingSq, !pos->whitesMove, pos)) {
			return -INF + pos->ply; // Checkmate at this depth
		}
		else {
			return 0; // stalemate
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


	/*
	STATIC NULL MOVE PRUNING / EVAL PRUNING
	*/

	if (depth < 3
		&& !pos->inCheck
		&& abs(beta - 1) > -MATE) {
		int static_eval = side * eval::staticEval(pos, depth, alpha, beta);

		int eval_margin = 120 * depth; // FIXME: Tune this value
		
		if (static_eval - eval_margin >= beta) {
			return (static_eval - eval_margin);
		}

	}

	/*
	END OF EVAL PRUNING
	*/


	/*
	
	NULL MOVE PRUNING

	*/
	
	if ((depth > 2)
		&& doNull
		&& (side * eval::staticEval(pos, depth, alpha, beta)) >= beta
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
			if (value >= beta) { return beta; }
			
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
	if (depth <= 3 && !pos->inCheck && abs(alpha) < 9000) {
		int eval = side * eval::staticEval(pos, depth, alpha, beta);

		// We'll only do razoring if depth = 2, the eval is below alpha by some margin, we arent extending and there isn't a PV-move
		if (depth < 2 && eval < (alpha - side * RAZOR_MARGIN) && !extend && bestMove == NOMOVE) {
			return Quiescence(alpha, beta, pos, info);
		}

		if (eval + (side * fMargin[depth]) <= alpha) {
			f_prune = true;
		}
	}

	int reduction_depth = 0;
	int new_depth = 0;
	int raised_alpha = 0;

	int moves_tried = 0;
	for (int moveNum = 0; moveNum < list.count; moveNum++) {
		pickNextMove(moveNum, &list);

		MoveGeneration::makeMove(*pos, list.moves[moveNum].move);
		moves_tried++;

		// FUTILITY PRUNING
		if (f_prune && pos->pieceList[TOSQ(list.moves[moveNum].move)] == NO_PIECE && SPECIAL(list.moves[moveNum].move) != 0
			&& !pos->inCheck) {
			MoveGeneration::undoMove(*pos);
			continue;
		}

		// LATE MOVE REDUCTION
		reduction_depth = 0;
		new_depth = depth - 1;

		if (new_depth >= 3
			&& moves_tried > 3 
			&& !pos->inCheck
			&& !flagInCheck &&
			pos->killerMoves[pos->ply][0] != list.moves[moveNum].move
			&& pos->killerMoves[pos->ply][1] != list.moves[moveNum].move
			&& pos->pieceList[TOSQ(list.moves[moveNum].move)] != NO_PIECE
			&& !(SPECIAL(list.moves[moveNum].move) == 0 || SPECIAL(list.moves[moveNum].move) == 1)) {

			reduction_depth = 1;

			if (moves_tried > 8) {
				reduction_depth += 1;
			}

			//new_depth = (reduction_depth > 0) ? (new_depth - reduction_depth) : new_depth;
			new_depth -= reduction_depth;
		}

		re_search:

		// Here we introduce PVS search.
		//value = -alphabeta(pos, info, new_depth, -beta, -alpha, true);
		if (!raised_alpha) {
			value = -alphabeta(pos, info, new_depth, -beta, -alpha, true, extend);
		}
		else {
			// First try to refute a move. If it fails, do a real search
			if (-alphabeta(pos, info, new_depth, -alpha - 1, -alpha, true, extend) > alpha) {
				value = -alphabeta(pos, info, new_depth, -beta, -alpha, true, extend);
			}
		}

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
					pos->historyHeuristic[pos->pieceList[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth*depth;

					// If the search is super deep, the history table will overflow, so we'll half all the values if this happens at one of them.
					if (pos->historyHeuristic[pos->pieceList[FROMSQ(bestMove)]][TOSQ(bestMove)] > 900000) {
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
				moves.moves[i].score = 2000000;
				break;
			}
		}
	}


	for (int i = 0; i < moves.count; i++) {
		pickNextMove(i, &moves);

		MoveGeneration::makeMove(*pos, moves.moves[i].move);
		info_currmove(moves.moves[i].move, depth, i);
		// Introduces PVS at root
		if (i == 0 || -alphabeta(pos, info, depth - 1, -alpha - 1, -alpha, true, extend) > alpha) {
			value = -alphabeta(pos, info, depth - 1, -beta, -alpha, true, extend);
		}

		MoveGeneration::undoMove(*pos);

		if (value > alpha) {
			bestMove = moves.moves[i].move;
			bestScore = alpha;
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
	if (bestMove == NOMOVE) { // Go into iterative deepening of we didn't find a book move.
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

			if (mateDist != 0) { // If there's been found a mate, print score in mate distance instead of centipawns
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
