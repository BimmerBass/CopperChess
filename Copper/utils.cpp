#include "defs.h"
#include "code_analysis.h"
#include "evaluation.h"
#include "wac_positions.h"

#include <iostream>
#include <fstream>

/*
INCLUDES THE FUNCTIONS:
- perft
- perftTest
*/


long leafNodes;

void MoveGeneration::perft(int depth, S_BOARD *board){
	if (depth == 0){
		leafNodes++;
		return;
	}
	
	S_MOVELIST moveList;
	validMoves(board, moveList);
	
	int moveNum = 0;
	for (moveNum = 0; moveNum < moveList.count; moveNum++){
		makeMove(*board, moveList.moves[moveNum].move);
		perft(depth - 1, board);
		undoMove(*board);
	}
	return;
}

void MoveGeneration::perftTest(int depth, S_BOARD* board) {
	std::cout << "[*] Starting test to depth: " << depth << std::endl;

	{
		Timer timer;
		leafNodes = 0;
		S_MOVELIST legalMoves;
		validMoves(board, legalMoves);
		int move;

		for (int moveNum = 0; moveNum < legalMoves.count; moveNum++) {
			move = legalMoves.moves[moveNum].move;

			makeMove(*board, move);
			S_BOARD newBoard = *board;
			undoMove(*board);
			long cumNodes = leafNodes;
			perft(depth - 1, &newBoard);

			long oldNodes = leafNodes - cumNodes;
			if ((move & 3) == 0) {
				std::string promo[4] = { "n", "b", "r", "q" };
				std::cout << "move " << moveNum + 1 << " " << index_to_square((move >> 4) & 63) << index_to_square(move >> 10) << promo[(move >> 2) & 3] << " : " << oldNodes << std::endl;
			}
			else {
				std::cout << "move " << moveNum + 1 << " " << index_to_square((move >> 4) & 63) << index_to_square(move >> 10) << " : " << oldNodes << std::endl;
			}
		}
	}
	std::cout << "Test completed. " << leafNodes << " visited." << std::endl;
	return;
}



void run_wac() {
	// Vector with information about all the positions Copper fails.
	std::vector<std::string> failed_positions;

	std::ofstream result_file;

	result_file.open("wac_results.txt");

	S_BOARD* pos = new S_BOARD;

	int failed = 0;
	int passed = 0;
	int total = 0;

	bool passedPos = false;
	std::string bestMoves = "";

	result_file << "*--------------- RUNNING WAC TEST ---------------*" << std::endl;

	for (int i = 0; i < WAC_LENGTH; i++) { // Loop through all WAC positions
		bestMoves = "";
		total++;
		passedPos = false;

		BoardRep::parseFen(WAC_positions[i], *pos);

		S_SEARCHINFO info;

		info.starttime = getTimeMs();

		info.stoptime = info.starttime + 5000;

		info.timeset = true;

		info.depth = MAXDEPTH;
		
		Search::searchPosition(pos, &info);
		
		for (int j = 0; j < 5; j++) {
			bestMoves += (j != 4) ? WAC_moves[i][j] + " " : WAC_moves[i][j];
			// Copper has found a move. We print it and continue
			if (pos->pvArray[0] == parseMove(WAC_moves[i][j], pos)) {
				result_file << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << WAC_moves[i][j] << " --------> PASSED" << std::endl;
				passed++;
				passedPos = true;
				break;
			}
		}

		// If we haven't continued, Copper hasn't found one of the right moves.
		if (passedPos == false) {
			result_file << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << bestMoves
				<< " --------> FAILED: Copper found: " << printMove(pos->pvArray[0]) << std::endl;
			
			std::string failed_position = "Position: ";
			failed_position += ": ";
			failed_position += WAC_positions[i];
			failed_position += " bm " + bestMoves;
			failed_position += " --------> FAILED: Copper found: " + printMove(pos->pvArray[0]);

			failed_positions.push_back(failed_position);

			failed++;
		}
	}


	if (failed == 0) {
		result_file << "*--------------- WAC TEST PASSED ---------------*" << std::endl;
	}
	else {
		result_file << "*--------------- WAC TEST RESULTS ---------------*" << std::endl;
		result_file << "Total: " << total << std::endl;
		result_file << "Passed: " << passed << std::endl;
		result_file << "Failed: " << failed << " (" << double(failed)/double(total) << "% failed)" << std::endl;
		result_file << "\n\n";
		result_file << "*--------------- FAILED POSITIONS ---------------*" << std::endl;
		for (int p = 0; p < failed_positions.size(); p++) {
			result_file << failed_positions[p] << std::endl;
		}

	}

	result_file.close();
}


void eval_balance() {
	
	std::cout << "*------------- STARTING EVALUATION FUNCTION BALANCE TEST -------------*" << std::endl;

	S_BOARD* pos = new S_BOARD;

	int total = 0;
	int failed = 0;

	int eval = 0;
	int mirror_eval = 0;

	for (int i = 0; i < WAC_LENGTH; i++) {
		total++;

		BoardRep::parseFen(WAC_positions[i], *pos);

		std::cout << "Position " << i + 1 << ": ";
		
		eval = eval::staticEval(pos, 0, 0);

		BoardRep::mirrorBoard(pos);

		mirror_eval = eval::staticEval(pos, 0, 0);

		if (eval == mirror_eval) {
			std::cout << "PASSED" << std::endl;
		}
		else {
			std::cout << "FAILED (" << eval << ", " << mirror_eval << ")" << std::endl;
			failed++;
		}
	}

	std::cout << "*------------- EVALUATION FUNCTION BALANCE TEST RESULTS -------------*\n" << std::endl;

	std::cout << "Total positions tested: " << total << std::endl;
	std::cout << "Amount of positions failed: " << failed << " (" << (double(failed) / double(total)) * 100.0 << "%)" << std::endl;

	return;
}
