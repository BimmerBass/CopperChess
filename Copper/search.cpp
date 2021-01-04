#include "defs.h"
#include "evaluation.h"


#define ASPIRATION 20 // Size of initial aspiration windows.


#define ENDGAME_MAT 1300 // The lower this value becomes, the more delta-cutoffs we get. It is a fine balance between accuracy and speed in quiescence search.
#define DELTA 200 // The delta safety margin for quiescence search. Raise this to get a more accurate quiescence evaluation at the cost of lowered speed.
#define RAZOR_MARGIN 600 // The security margin for razoring. The value is taken from Stockfish's implementation.

#define CONTEMPT 10 // The factor by which we'll multiply the winning probability of the position.
#define CONTEMPT_SCALING_CONSTANT 2.5 // This is a scaling constant that makes the function for winnning propability more or less sensitive.


int static_eval[MAXDEPTH] = { 0 };

// Move to exclude when doing singular extension search.
int excludedMove = NOMOVE;


void Search::CheckUp(S_SEARCHINFO* info) {
	if (info->timeset == true && getTimeMs() > info->stoptime) {
		info->stopped = true;
	}
	ReadInput(info);
}


bool isRepetition(const S_BOARD* pos) {
	assert(pos != NULL);
	for (int i = 0; i < pos->history.moveCount; i++) {
		assert(&pos->history.history[i].key != NULL);
		if (pos->history.history[i].key == pos->posKey) {
			return true;
		}
	}
	return false;
}




int Search::futility_margin(int d, bool i) {
	return (175 - 50 * i) * d;
}


int Search::reduction(bool improving, int depth, int moveCount) {
	int r = Reductions[depth] * Reductions[moveCount];
	return ((r + 512) / 1024 + (!improving && r > 1024)) * 1;
}

// Got y = 0.5*(x^2) + 6.5*x + 1 to fit such that for d = 1, lim = 8. d = 2, lim = 16, d = 3, lim = 25
int Search::lmp_limit(int d, bool i) {
	//return int(0.5 * pow(d, 2) + 6.5 * d + 1) + ((i) ? 1 : 0);
	return (4 * d + ((i) ? 1 : 0));
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

	if (info->stopped) { return 0; }

	info->nodes++;

	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return 0;
	}


	// Prefetch the evaluation cache as we'll probe it when getting the stand_pat score
	pos->evaluationCache->prefetch_cache(pos);


	if (pos->ply > MAXDEPTH - 1) {
		return eval::staticEval(pos, alpha, beta);
	}

	int stand_pat = eval::staticEval(pos, alpha, beta);

	if (stand_pat >= beta) {
		// Since we're using fail-hard alpha-beta we return beta. For fail soft, one would return stand_pat
		return beta;
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

			/*
			DELTA PRUNING
			*/

			if ((stand_pat + eval::pieceValMg[pos->pieceList[TOSQ(list.moves[moveNum].move)]] + DELTA <= alpha)
				&& (eval::getMaterial(pos, !pos->whitesMove) - eval::pieceValMg[pos->pieceList[TOSQ(list.moves[moveNum].move)]] > ENDGAME_MAT)
				&& SPECIAL(list.moves[moveNum].move) != 0) {
				continue;
			}

			/*
			END OF DELTA PRUNING
			*/

			// If the capture looses material immediately, we will not search it, as it probably also looses in the long run.
			if (badCapture(pos, list.moves[moveNum].move)
				&& SPECIAL(list.moves[moveNum].move) != 0) {
				continue;
			}

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

int Search::alphabeta(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta, bool doNull, bool is_pv) {

	assert(depth >= 0);
	assert(beta > alpha);


	if (depth == 0) {
		return Quiescence(alpha, beta, pos, info);
	}

	info->nodes++;
	int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];


	if ((isRepetition(pos) || pos->fiftyMove >= 100) && pos->ply > 0) {
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1) {
		return eval::staticEval(pos, alpha, beta);
	}

	int score = -INF;
	int bestMove = NOMOVE;
	bool raised_alpha = false;

	// Prefetch the entry that we'll probe later on.
	prefetch(&pos->transpositionTable->tableEntry[pos->posKey % pos->transpositionTable->numEntries]);


	// Used to calculate futility margins. If our position has just worsened we could be close to mate
	//	which means that we should be more cautious about pruning.
	bool improving;

	// Remember if the parent node is in check. This will be used for LMR.
	bool flagInCheck = pos->inCheck;

	// Futility pruning flag. This is set to false by default.
	bool f_prune = false;
	
	/*
	MATE DISTANCE PRUNING:
		- If we have already found a checkmate sequence, we will not bother searching longer ones.
			Tested with this position: 4n2k/p1p1p1pp/bp1pn1Q1/8/7R/6RP/1BB3PK/5r2 b - - 2 4		(mate in 5 for white)
			Nodes searched at depth = 10 before implementing MDP: 98796502
			Nodes searched after implementation, at depth = 10: 6742285
			Which gives a speed increase of around 93% in this particular position.
	*/
	int mate_value = INF - pos->ply;
	if (alpha < -mate_value) { alpha = -mate_value; }
	if (beta > mate_value - 1) { beta = mate_value - 1; }
	if (alpha >= beta) { return alpha; }


	/*
	TRANSPOSITION TABLE PROBING:
		- We probe the transposition table, and if this position has been searched to the desired depth before, we'll just return the score.
	*/
	int ttMove = NOMOVE;
	int ttScore = -INF;
	bool ttHit = false;
	TT_FLAG ttFlag = NO_FLAG;
	int ttDepth = 0;

	/*if (TT::probePos(pos, depth, alpha, beta, &ttMove, &ttScore)) {
		return ttScore;
	}*/

	S_TTENTRY* ttEntry = TT::extract_entry(pos, ttHit);

	ttMove = (ttHit) ? ttEntry->move : NOMOVE;
	ttScore = (ttHit) ? ttEntry->score : -INF;

	if (ttScore > MATE) { ttScore -= pos->ply; }
	else if (ttScore < MATE) { ttScore += pos->ply; }

	ttFlag = (ttHit) ? ttEntry->flag : NO_FLAG;
	ttDepth = (ttHit) ? ttEntry->depth : -1;

	if (ttDepth >= depth && ttHit) {

		switch (ttFlag) {
		case LOWER:
			if (ttScore <= alpha) {
				return alpha;
			}
			break;
		case UPPER:
			if (ttScore >= beta) {
				return beta;
			}
			break;
		case EXACT:
			return ttScore;
		}
	}



	// If we have searched a certain number of nodes, we need to check if the GUI has sent a "stop" or "quit" command.
	if ((info->nodes & 2047) == 0) {
		CheckUp(info);
	}


	/*
	IN CHECK EXTENSIONS:
		- If we're in check, we want to see how this variation ends, and we'll therefore increase the depth we search to.
			This is okay since the variation is very forced, which means that the branching factor will usually be pretty small.
			We also don't want to accidentally prune this branch, so we'll go directly to the moves loop.
	*/
	if (flagInCheck == true) {
		depth++;

		static_eval[pos->ply] = VALUE_NONE;
		improving = false;

		goto moves_loop;
	}



	// If there is already an evaluation in the cache, we'll use that. Otherwise, we need to evaluate the position statically.
	if (!pos->evaluationCache->probeCache(pos, static_eval[pos->ply])) {
		assert(!flagInCheck);

		static_eval[pos->ply] = eval::staticEval(pos, alpha, beta);
	}



	/*
	REVERSE FUTILITY PRUNING:
		- If we are at or below the pre-pre-frontier nodes, we will perform a static evaluation, and if this beats beta by some margin depending on
			the depth we're at, this is very probably a fail-high node, and we can return beta.
			We of course cannot do this in the PV or if we're in check, since our position can deteriorate drastically if we dont search these sequences
			fully. We also don't want to do this if beta is close to a mate score.
	*/
	if (depth <= 3
		&& !flagInCheck
		&& !is_pv
		&& abs(beta) < MATE) {

		int eval_margin = 120 * depth;

		if (static_eval[pos->ply] - eval_margin >= beta) {
			return beta;
		}
	}




	/*
	NULL MOVE PRUNING
		- If the static evaluation beats beta, we give the opponent a free move (just reversing side to move) and if they still can't improve their position
			over beta, we can be fairly certain that our position is so good that we don't need to search it any further, and just return beta.
	*/

	if (depth > 2
		&& !is_pv
		&& doNull
		&& !flagInCheck
		&& static_eval[pos->ply] >= beta) {

		// FIXME: This boolean endgame-determination value probably needs to be changed.
		bool endgame = ((pos->position[WQ] | pos->position[BQ]) == 0 || (pos->position[WN] | pos->position[WB] | pos->position[WR]
			| pos->position[BN] | pos->position[BB] | pos->position[BR]) == 0) ? true : false;

		int oldEp = pos->enPassantSquare;

		makeNullMove(pos);

		int R = 2;

		if (depth > 6) { R = 3; }

		score = -alphabeta(pos, info, depth - R - 1, -beta, -beta + 1, false, false);

		undoNullMove(pos, oldEp);

		/*
		NULL MOVE PRUNING
			- If we're not in the endgame, we can prune this branch if it still beats beta, and isn't a mate score.
				we can't do this in the endgame due to the danger of pruning a zugzwang branch.
		*/
		if (!endgame) {
			if (score >= beta && abs(score) < MATE) { return beta; }
		}

		/*
		NULL MOVE REDUCTIONS
			- If we are in the endgame, we cannot prune, but we are fairly certain that our position is really good, so we can reduce the search depth
				considerably. This removes the danger of zugzwang.
		*/
		else {
			if (score >= beta && abs(score) < MATE) {
				depth -= 1;

				// If we are close to the frontier nodes before null move search, we can just go straight into quiescence.
				if (depth <= 0) {
					assert(!flagInCheck);

					return Quiescence(alpha, beta, pos, info);
				}
			}
		}
	}




	/*
	FUTILITY PRUNING:
		- If we are not in a pv-node and not in check, we'll allow non-tactical moves to be pruned if the evaluation is below alpha by a certain margin
		dependent on the depth and wether or not our position has improved since the last move. 
		This margin should be fairly big such as to only prune futile moves. Moves that are very unlikely to raise alpha significantly.
		Also, we'll only do this if alpha is no where near a checkmate score.
	*/

	// Firstly, we'll check if we are improving our position or not.
	// If the static evaluation of this position is better than the last time this site were to move, we're probably improving.7
	// Also if we were in check last move, we are also improving since we're getting out of check.
	if (pos->ply >= 2) {
		improving = static_eval[pos->ply] >= static_eval[pos->ply - 2] || static_eval[pos->ply - 2] == VALUE_NONE;
	}
	else {
		improving = false;
	}

	if (depth <= 3 &&
		!is_pv && !flagInCheck
		&& abs(alpha) < 10000
		&& static_eval[pos->ply] + futility_margin(depth, improving) <= alpha) {

		f_prune = true;
	}



	/*
	INTERNAL ITERATIVE DEEPENING:
		- If the transposition table didn't return a move to search first and this is a PV-node, we will search to a reduced depth to get an estimate
			of the best move.
	*/
	if (depth >= 8 && !ttHit && is_pv) {

		// A lot more pruning is allowed if we do not search as a PV-node and since this will only be an estimate, we don't need high search accuracy.
		score = -alphabeta(pos, info, depth - 7, -beta, -alpha, true, false);

		ttEntry = TT::extract_entry(pos, ttHit);

		ttMove = (ttHit) ? ttEntry->move : NOMOVE;
		ttScore = (ttHit) ? ttEntry->score : -INF;

		if (ttScore > MATE) { ttScore -= pos->ply; }
		else if (ttScore < MATE) { ttScore += pos->ply; }

		ttFlag = (ttHit) ? ttEntry->flag : NO_FLAG;
		ttDepth = (ttHit) ? ttEntry->depth : -1;
	}




	// If we are in check, we dont want to prune anything and will therefore just go to the moves loop.
	moves_loop:

	S_MOVELIST pos_moves;
	MoveGeneration::validMoves(pos, pos_moves);

	if (pos_moves.count == 0) {
		if (sqAttacked(kingSq, !pos->whitesMove, pos)) {
			return -INF + pos->ply;
		}
		else {
			return 0;
		}
	}



	/*
	If the transposition table returned a move, we will score this highest, so it'll be searched first.
	*/
	if (ttMove != NOMOVE) {
		for (int i = 0; i < pos_moves.count; i++) {
			if (pos_moves.moves[i].move == ttMove) {
				pos_moves.moves[i].score = 2000000;
				break;
			}
		}
	}


	/*
	ENHANCED TRANSPOSITION CUTOFF:
		- If the transposition table didn't return a score for this position, we can search the strongest moves in hopes that it does have entries in some of
			the child nodes that produce a cutoff. This is only done at depth >= 3 as it would otherwise be too computationally expensive near the leaf nodes.
	*/
	/*if (!ttHit && depth >= 3) {
		int next_alpha = -beta;
		int next_beta = -alpha;
		bool new_ttHit = false;
		int new_ttScore = -INF;
		int new_ttDepth = 0;
		TT_FLAG new_ttFlag = NO_FLAG;

		int oldMoveScore = 0;

		// We'll only check the five expected strongest moves.
		for (int i = 0; i < 5; i++) {
			
			oldMoveScore = pos_moves.moves[i].score;

			pickNextMove(i, &pos_moves);

			MoveGeneration::makeMove(*pos, pos_moves.moves[i].move);

			pos_moves.moves[i].score = oldMoveScore;

			ttEntry = TT::extract_entry(pos, new_ttHit);

			new_ttScore = (new_ttHit) ? ttEntry->score : INF;

			if (new_ttScore > MATE) { new_ttScore -= pos->ply; }
			else if (new_ttScore < MATE) { new_ttScore += pos->ply; }


			new_ttDepth = (new_ttHit) ? ttEntry->depth : 0;

			new_ttFlag = (new_ttHit) ? ttEntry->flag : NO_FLAG;


			if (new_ttHit && new_ttDepth >= depth - 1){
				switch (new_ttFlag) {
				case LOWER:
					if (new_ttScore <= next_alpha) {
						return alpha;
					}
					break;
				case UPPER:
					if (new_ttScore >= next_beta) {
						return beta;
					}
					break;
				case EXACT:
					return new_ttScore;
				}
			}

		}


	}*/



	int piece_captured = NO_PIECE;
	int new_depth = 0;
	int reduction_depth = 0;
	int moves_tried = 0;

	// Extension given by singular search
	int singularLMR = 0;

	// The depth to extend by.
	int extension = 0;

	for (int moveNum = 0; moveNum < pos_moves.count; moveNum++) {
		pickNextMove(moveNum, &pos_moves);


		/*
		SINGULAR EXTENSION SEARCH:
			- If one move in the position seems to be a lot better than all the others, we will extend the depth. This is relatively cheap since we
				expect that all other moves will fail low pretty fast.
		*/
		if (ttHit && depth >= 8 &&
			pos_moves.moves[moveNum].move == ttMove &&
			excludedMove == NOMOVE && // This is done to not do singular search recursively.
			abs(ttScore) < 10000 &&
			ttFlag == LOWER &&
			ttDepth >= depth - 3) {

			int singular_beta = abs(ttScore - 2 * depth);

			int half_depth = depth / 2;

			excludedMove = pos_moves.moves[moveNum].move;

			score = alphabeta(pos, info, half_depth, singular_beta - 1, singular_beta, true, false);

			excludedMove = NOMOVE;

			if (score < singular_beta) {
				extension = 1;

				singularLMR++;

				if (score < singular_beta - std::min(3 * depth, 39)) {
					singularLMR++;
				}
			}
		}



		piece_captured = pos->pieceList[TOSQ(pos_moves.moves[moveNum].move)];

		assert(piece_captured != WK && piece_captured != BK);

		MoveGeneration::makeMove(*pos, pos_moves.moves[moveNum].move);

		/*
		FUTILITY PRUNING:
			- If the futility pruning flag is set and the move is not a tactical move (promotion, capture or check), we'll not search it further
				as it most probably won't raise alpha.
		*/
		if (f_prune && piece_captured == NO_PIECE && SPECIAL(pos_moves.moves[moveNum].move) != 0
			&& !pos->inCheck) {
			MoveGeneration::undoMove(*pos);
			continue;
		}


		/*
		LATE MOVE PRUNING:
			- If we are lower than or in a pre-pre-frontier node, and we have already searched the most promising moves, we can with relatively big
				certainty safely prune the next moves. Obviously, this shouldn't be done for tactical moves (captures, checks, out-of-checks, promotions
				 and en-passants) or in a pv_node.
		*/
		/*if (depth <= 3 && !flagInCheck && !is_pv && !pos->inCheck && piece_captured == NO_PIECE && SPECIAL(pos_moves.moves[moveNum].move) != 0) {
			if (moves_tried >= lmp_limit(depth, improving)) {
				MoveGeneration::undoMove(*pos);
				continue;
			}
		}*/


		new_depth = depth - 1 + extension;
		reduction_depth = 0;

		/*
		LATE MOVE REDUCTIONS:
			- If we haven't raised alpha with the first 3 moves, this is likely a fail-low node since these tend to be the best moves. 
				Therefore, we will reduce the search depth, and only do a full depth search if this reduced depth raises alpha.
				This will not be used for tactical moves or if the depth is less than 3.
				Tactical moves are moves that goes out of check, moves that give check, captures and promotions.
				If we are not in a pv node, the moves_tried should only be more than one, and the reduction will be bigger.
		*/
		if (moves_tried > (1 + 3 * is_pv)
			&& depth >= 3
			&& !flagInCheck
			&& !pos->inCheck
			&& piece_captured == NO_PIECE
			&& SPECIAL(pos_moves.moves[moveNum].move) != 0) {

			reduction_depth = reduction(improving, depth, moves_tried);
			
			if (is_pv) {
				reduction_depth--;
			}

			reduction_depth -= singularLMR;

			new_depth = std::max(new_depth - reduction_depth, 0);
		}
		

		/*
		PRINCIPAL VARIATION SEARCH
			- Here we use PVS to only search the principal variation or one that is expected to be the latter. This means, that if we haven't found
			a line that reaises alpha, we'll perform a full window search. But if we, on the other hand already have raised alpha,
			we assume that line to be the principal variation, and will perform a null window search. If this then fails low, we might've
			found a better PV, and we'll search this with a full window.
		*/
		re_search:

		if (!raised_alpha) {
			score = -alphabeta(pos, info, new_depth, -beta, -alpha, true, is_pv);
		}
		else {
			score = -alphabeta(pos, info, new_depth, -(alpha + 1), -alpha, true, false);
			if (score > alpha) {
				score = -alphabeta(pos, info, new_depth, -beta, -alpha, true, true);
			}
		}


		// If reduced depth search raises alpha, we need to do a full-depth re search of that variation since it might be good for us.
		if (reduction_depth && score > alpha) {
			new_depth = depth - 1;
			reduction_depth = 0;

			goto re_search;
		}

		MoveGeneration::undoMove(*pos);

		moves_tried++;

		if (info->stopped == true) { return 0; }

		if (score > alpha) {
			bestMove = pos_moves.moves[moveNum].move;

			if (score >= beta) {
				if (moveNum == 0) {
					info->fhf++;
				}
				info->fh++;


				if (piece_captured == NO_PIECE && SPECIAL(pos_moves.moves[moveNum].move) != 0) { // Move is not a capture or en-passant
					pos->killerMoves[pos->ply][1] = pos->killerMoves[pos->ply][0]; // move first move to the last index
					pos->killerMoves[pos->ply][0] = pos_moves.moves[moveNum].move; // Replace first move with this move.

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


				TT::storeEntry(pos, bestMove, depth, UPPER, score);

				return beta;
			}

			alpha = score;
			raised_alpha = true;
		}
	}

	if (raised_alpha) {
		TT::storeEntry(pos, bestMove, depth, EXACT, alpha);
	}
	else {
		TT::storeEntry(pos, bestMove, depth, LOWER, alpha);
	}

	return alpha;
}

inline void info_currmove(int move, int depth, int moveNum) {
	std::string moveStr = printMove(move);
	std::cout << "info depth " << depth << " currmove " << moveStr << " currmovenumber " << (moveNum + 1) << std::endl;
	return;
}


int Search::searchRoot(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta) {
	S_MOVELIST moves;
	MoveGeneration::validMoves(pos, moves);


	assert(depth > 0);
	assert(beta > alpha);


	int bestMove = NOMOVE;
	int value = -INF;
	bool raised_alpha = false;

	if (moves.count == 0) {
		int kingSq = (pos->whitesMove == WHITE) ? pos->kingPos[0] : pos->kingPos[1];
		if (sqAttacked(kingSq, !pos->whitesMove, pos) == true) {
			return -INF;
		}
		else {
			return 0;
		}
	}

	// Static evaluation.
	if (!pos->evaluationCache->probeCache(pos, static_eval[pos->ply])) {
		static_eval[pos->ply] = eval::staticEval(pos, alpha, beta);
	}

	/*
	IN CHECK EXTENSIONS:
		- If we're in check, we want to see how this variation ends, and we'll therefore increase the depth we search to.
			This is okay since the variation is very forced, which means that the branching factor will usually be pretty small.
	*/
	if (pos->inCheck == true) {
		depth++;
	}
	
	// Check for the best move at the moment, and score this highest (if there's any)
	int pvMove = TT::probePvMove(pos);
	if (pvMove != NOMOVE) {
		for (int i = 0; i < moves.count; i++) {
			if (moves.moves[i].move == pvMove) {
				moves.moves[i].score = 4000000;
				break;
			}
		}
	}

	for (int i = 0; i < moves.count; i++) {
		pickNextMove(i, &moves);

		assert(pos->pieceList[TOSQ(moves.moves[i].move)] != WK && pos->pieceList[TOSQ(moves.moves[i].move)] != BK);

		MoveGeneration::makeMove(*pos, moves.moves[i].move);

#if defined(COPPER_VERBOSE)
		info_currmove(moves.moves[i].move, depth, i);
#endif

		if (!raised_alpha) {
			value = -alphabeta(pos, info, depth - 1, -beta, -alpha, true, true);
		}
		else {
			value = -alphabeta(pos, info, depth - 1, -(alpha + 1), -alpha, true, false);
			if (value > alpha) {
				value = -alphabeta(pos, info, depth - 1, -beta, -alpha, true, true);
			}
		}

		MoveGeneration::undoMove(*pos);

		if (value > alpha) {

			bestMove = moves.moves[i].move;

			if (value >= beta) {

				if (i == 0) {
					info->fhf++;
				}
				info->fh++;

				TT::storeEntry(pos, bestMove, depth, UPPER, beta);

				return beta;
			}

			raised_alpha = true;
			alpha = value;
		}
	}

	if (raised_alpha) {
		TT::storeEntry(pos, bestMove, depth, EXACT, alpha);
	}
	else {
		TT::storeEntry(pos, bestMove, depth, LOWER, alpha);
	}

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
		alpha = std::max(estimate - delta, -INF);
		beta = std::min(estimate + delta, INF);
	}

asp_search:
	v = searchRoot(pos, info, depth, alpha, beta);

	if (v <= alpha) {
		beta = (alpha + beta) / 2;
		alpha = std::max(v - delta, -INF);

		delta += (delta / 4) + 5;
		goto asp_search;
	}
	else if (v >= beta) {
		beta = std::min(v + delta, INF);
		delta += (delta / 4) + 5;
		goto asp_search;
	}

	return v;
}


int Search::searchPosition(S_BOARD* pos, S_SEARCHINFO* info) {
	int bestMove = NOMOVE;
	int pvMoves = 0;

	clearForSearch(pos, info);


	// If Copper can use an opening book, we can look up the position to see if there is a move.
	if (engineOptions->use_book == true) {
		bestMove = getBookMove(pos);
	}


	// Search to depth 1 to get estimate of value, and pad this to get the aspiration window.
	int score = searchRoot(pos, info, 1, -INF, INF);
	double nps = 0;
	int mateDist = 0;

	// Iterative deepening.
	if (bestMove == NOMOVE) { // Go into iterative deepening if we didn't find a book move.
		for (int currDepth = 1; currDepth <= info->depth; currDepth++) {

			mateDist = 0;

			score = search_widen(pos, info, currDepth, score);

			//score = MTDF(pos, info, score, currDepth);

			nps = (info->nodes) / ((getTimeMs() - info->starttime) * 0.001);

			if (abs(score) > MATE) { // There is a mate in the position.
				// According to UCI, the mate distance should be given in full moves (for example 1. e4, e5 is one full move)
				// Therefore we'll find the mate distance in plies and divide by two to convert to full moves.
				// If mate is in n and 0.5 moves, the full mate score will be (n + n % 2) / 2
				mateDist = (INF - abs(score)) / 2;

				if ((INF - abs(score)) % 2 != 0) {
					mateDist++;
				}

				// Make it negative if Copper is being mated:
				mateDist *= (score < -MATE) ? -1 : 1;
			}

			if (info->stopped == true) {
				break; // Break out if the GUI has interrupted the search
			}

			pvMoves = TT::getPvLine(pos, currDepth);
			bestMove = pos->pvArray[0];
#if defined(COPPER_VERBOSE)
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
#endif
		}
	}
#if defined(COPPER_VERBOSE)
	std::cout << "bestmove " << printMove(bestMove) << "\n";
#endif

	return score;
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

	for (int i = 0; i < MAXDEPTH; i++) {
		static_eval[i] = 0;
	}

}
