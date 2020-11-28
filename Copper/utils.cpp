#include "defs.h"
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

void MoveGeneration::perftTest(int depth, S_BOARD *board){
	std::cout << "[*] Starting test to depth: " << depth << std::endl;
	
	{
		Timer timer;
	leafNodes = 0;
	S_MOVELIST legalMoves;
	validMoves(board, legalMoves);
	int move;
	
	for (int moveNum = 0; moveNum < legalMoves.count; moveNum++){
		move = legalMoves.moves[moveNum].move;

		makeMove(*board, move);
		S_BOARD newBoard = *board;
		undoMove(*board);
		long cumNodes = leafNodes;
		perft(depth - 1, &newBoard);
			
		long oldNodes = leafNodes - cumNodes;
		if ((move & 3) == 0){
			std::string promo[4] = {"n", "b", "r", "q"};
			std::cout << "move " << moveNum + 1 << " " << index_to_square((move >> 4) & 63) << index_to_square(move >> 10) << promo[(move >> 2) & 3] << " : " << oldNodes << std::endl;
		}
		else{
		std::cout << "move " << moveNum + 1 << " " << index_to_square((move >> 4) & 63) << index_to_square(move >> 10) << " : " << oldNodes << std::endl;
		}
	}
	}
	std::cout << "Test completed. " << leafNodes << " visited." << std::endl;
	return;
}


void run_wac() {
	std::vector<std::string> results;
	results.reserve(WAC_LENGTH);

	std::ofstream result_file;

	result_file.open("wac_results.txt");

	S_BOARD* pos = new S_BOARD;

	int failed = 0;
	int passed = 0;
	int total = 0;

	bool passedPos = false;

	result_file << "*--------------- RUNNING WAC TEST ---------------*" << std::endl;

	for (int i = 0; i < WAC_LENGTH; i++) { // Loop through all WAC positions
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
			// Copper has found a move. We print it and continue
			if (pos->pvArray[0] == parseMove(WAC_moves[i][j], pos)) {
				result_file << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << WAC_moves[i][j] << " --------> PASSED" << std::endl;
				passed++;
				passedPos = true;
				break;
			}
		}

		// If we haven't continued, Copper hasn't found one of the right moves.
		if (!passedPos) {
			result_file << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << WAC_moves[i]
				<< " --------> FAILED: Copper found: " << printMove(pos->pvArray[0]) << std::endl;
			failed++;
		}
		/*if (pos.pvArray[0] == parseMove(WAC_moves[i], &pos)) {
			std::cout << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << WAC_moves[i] << " --------> PASSED" << std::endl;
			passed++;
			continue;
		}

		// We didn't find the right move
		else {
			std::cout << "Position " << i + 1 << ": " << WAC_positions[i] << " bm " << WAC_moves[i]
				<< " --------> FAILED: Copper found: " << printMove(pos.pvArray[0]) << std::endl;
			failed++;
			continue;
		}*/
	}

	if (failed == 0) {
		result_file << "*--------------- WAC TEST PASSED ---------------*" << std::endl;
	}
	else {
		result_file << "*--------------- WAC TEST RESULTS ---------------*" << std::endl;
		result_file << "Total: " << total << std::endl;
		result_file << "Passed: " << passed << std::endl;
		result_file << "Failed: " << failed << " (" << double(failed)/double(total) << "% failed)" << std::endl;
	}

	result_file.close();
}