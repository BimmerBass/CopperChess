#include "defs.h"

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