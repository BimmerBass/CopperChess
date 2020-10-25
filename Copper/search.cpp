#include "defs.h"

#define ASPIRATION 50 // Size of aspiration window on each side of previous score. Taken from https://www.chessprogramming.org/CPW-Engine_search

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
			return -INFINITE + pos->ply;
		}
		else {
			return 0;
		}
	}

	int score = -INFINITE;
	int captures = 0;

	for (int moveNum = 0; moveNum < list.count; moveNum++) {
		pickNextMove(moveNum, &list);

		// We'll only search captures and promotions
		if (pos->pieceList[TOSQ(list.moves[moveNum].move)] != NO_PIECE || SPECIAL(list.moves[moveNum].move) == 0
			|| SPECIAL(list.moves[moveNum].move) == 1) {
			captures++;

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

int Search::alphabeta(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta, bool doNull) {
	int side = (pos->whitesMove == WHITE) ? 1 : -1;
	int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];
	if (pos->inCheck) { depth++; }
	if (depth == 0) {
		return Quiescence(alpha, beta, pos, info);
		
	}

	int bestScore = -INFINITE;
	int bestMove = NOMOVE;
	int value = -INFINITE;
	int oldAlpha = alpha;

	bool f_prune = false;

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
			return -INFINITE + pos->ply; // Checkmate at this depth
		}
		else {
			return 0; // stalemate
		}
	}

	// Score the transposition table move highest, so it'll be searched first.
	if (bestMove != NOMOVE) {
		for (int i = 0; i < list.count; i++) {
			if (list.moves[i].move == bestMove) {
				list.moves[i].score = 2000000;
				break;
			}
		}
	}

	/*
	
	NULL MOVE PRUNING

	*/
	
	if ((depth > 2)
		&& doNull
		&& (side * eval::staticEval(pos, depth, alpha, beta)) >= beta
		&& !pos->inCheck) {
		bool endgame = ((pos->position[WQ] | pos->position[BQ]) == 0 || (pos->position[WN] | pos->position[WB] | pos->position[WR]
			| pos->position[BN] | pos->position[BB] | pos->position[BR]) == 0) ? true : false;
		if (!endgame) {
			int oldEp = pos->enPassantSquare;

			makeNullMove(pos);

			int R = 2;
			if (depth > 6) { R = 3; }

			value = -alphabeta(pos, info, depth - R - 1, -beta, -beta + 1, false);

			undoNullMove(pos, oldEp);

			if (info->stopped == true) { return 0; }
			if (value >= beta) { return beta; }
		}

	}

	/*
	
	END OF NULL MOVE PRUNING

	*/


	// Determine if futility pruning is applicable
	if (depth <= 3 && !pos->inCheck && abs(alpha) < 9000) {
		if (side*(eval::staticEval(pos, depth, alpha, beta) + fMargin[depth]) <= alpha) {
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

		if (f_prune && pos->pieceList[TOSQ(list.moves[moveNum].move)] == NO_PIECE && SPECIAL(list.moves[moveNum].move) != 0
			&& !pos->inCheck) {
			MoveGeneration::undoMove(*pos);
			continue;
		}

		// LATE MOVE REDUCTION
		reduction_depth = 0;
		new_depth = depth - 1;

		if (new_depth > 3
			&& moves_tried > 3 
			&& !pos->inCheck &&
			pos->killerMoves[pos->ply][0] != list.moves[moveNum].move
			&& pos->killerMoves[pos->ply][1] != list.moves[moveNum].move
			&& pos->pieceList[TOSQ(list.moves[moveNum].move)] != NO_PIECE
			&& !(SPECIAL(list.moves[moveNum].move) == 0 || SPECIAL(list.moves[moveNum].move) == 1)) {

			reduction_depth = 1;

			if (moves_tried > 8) {
				reduction_depth += 1;
			}

			new_depth = (reduction_depth > 0) ? (new_depth - reduction_depth) : new_depth;
		}

		re_search:

		value = -alphabeta(pos, info, new_depth, -beta, -alpha, true);

		// Sometimes reduced search brings us above alpha. Then we'll have to retry
		if (reduction_depth && value > alpha) {
			new_depth += reduction_depth;
			reduction_depth = 0;
			goto re_search;
		}

		MoveGeneration::undoMove(*pos);

		if (info->stopped == true) { return 0; }

			if (value > alpha) {
				raised_alpha = 1;

				bestMove = list.moves[moveNum].move;
				bestScore = value;
				if (value >= beta) {
					if (moveNum == 0) {
						info->fhf++;
					}
					info->fh++;

					// Update killer moves*
					if (pos->pieceList[TOSQ(list.moves[moveNum].move)] == NO_PIECE) { // Move is not a capture
						pos->killerMoves[pos->ply][1] = pos->killerMoves[pos->ply][0]; // move first move to the last index
						pos->killerMoves[pos->ply][0] = list.moves[moveNum].move; // Replace first move with this move.
					}

					TT::storeEntry(pos, bestMove, depth, UPPER, beta);

					return beta;
				}
				alpha = value;

				// Update history heuristics
				if (pos->pieceList[TOSQ(bestMove)] == NO_PIECE) {
					pos->historyHeuristic[pos->pieceList[FROMSQ(bestMove)]][TOSQ(bestMove)] += depth;
				}
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
	if (pos->inCheck) { depth++; }

	S_MOVELIST moves;
	MoveGeneration::validMoves(pos, moves);
	int bestMove = NOMOVE;
	int bestScore = -INFINITE;
	int value = -INFINITE;
	int oldAlpha = alpha;

	for (int i = 0; i < moves.count; i++) {
		pickNextMove(i, &moves);

		MoveGeneration::makeMove(*pos, moves.moves[i].move);
		info_currmove(moves.moves[i].move, depth, i);
		// Introduces PVS at root
		if (i == 0 || -alphabeta(pos, info, depth - 1, -alpha - 1, -alpha, true) > alpha) {
			value = -alphabeta(pos, info, depth - 1, -beta, -alpha, true);
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


// Aspiration window search.
int Search::search_widen(S_BOARD* pos, S_SEARCHINFO* info, int depth, int value) {
	int temp = value, alpha = value - ASPIRATION, beta = value + ASPIRATION;
	int alphaCount = 2;
	int betaCount = 2;

	temp = searchRoot(pos, info, depth, alpha, beta);
	
	// If value is outside of [value-ASPIRATION, value+ASPIRATION], a re-search has to be done.
	if ((temp <= alpha) || (temp >= beta)) {
		temp = searchRoot(pos, info, depth, -INFINITE, INFINITE);
	}
	return temp;
}


/*

MTD(f) is experimental and not used yet.

*/
int Search::MTDF(S_BOARD* pos, S_SEARCHINFO* info, int estimate, int depth) {
	int lowerBound = -INFINITE;
	int upperBound = INFINITE;
	int score = estimate;

	int alpha = 0;
	int beta = 0;

	while (lowerBound < upperBound) {
		beta = max(score, lowerBound + 1);

		score = searchRoot(pos, info, depth, beta - 1, beta);
		
		if (score < beta) {
			upperBound = score;
		}
		else {
			lowerBound = score;
		}
	}
	return score;
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
	int score = searchRoot(pos, info, 1, -INFINITE, INFINITE);
	long long nps = 0;
	int mateDist = 0;

	// Iterative deepening.
	if (bestMove == NOMOVE) { // Go into iterative deepening of we didn't find a book move.
		for (int currDepth = 1; currDepth <= info->depth; currDepth++) {
			mateDist = 0;

			score = search_widen(pos, info, currDepth, score);

			//score = MTDF(pos, info, score, currDepth);

			nps = (info->nodes) / ((getTimeMs() - info->starttime) * 0.001);

			if (score > MATE) {
				int side = (pos->whitesMove == WHITE) ? 1 : -1;
				mateDist = side * (INFINITE - score) / 2;
			}
			else if (score < -MATE) {
				int side = (pos->whitesMove == WHITE) ? -1 : 1;
				mateDist = side * (score + INFINITE) / 2;
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
