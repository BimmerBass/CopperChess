#include "defs.h"


/*
INCLUDES THE FUNCTIONS:
- MoveGeneration::validMoves
- MoveGeneration::pseudoLegalMoves
- MoveGeneration::pawnMoves
- MoveGeneration::knightMoves
- MoveGeneration::bishopMoves
- MoveGeneration::rookMoves
- MoveGeneration::queenMoves
- MoveGeneration::kingMoves
- moveExists
*/

inline void scoreQuiets(const S_BOARD* board, S_MOVELIST* legalMoves) {
	// Check if it is a killer move, and score accordingly
	if (board->pieceList[TOSQ(legalMoves->moves[legalMoves->count].move)] == NO_PIECE) { // Not a capture move
		if (board->killerMoves[board->ply][0] == legalMoves->moves[legalMoves->count].move) {
			legalMoves->moves[legalMoves->count].score = 900000; // We'll make the latest killer move found, the first to get searched
		}
		else if (board->killerMoves[board->ply][1] == legalMoves->moves[legalMoves->count].move) {
			legalMoves->moves[legalMoves->count].score = 800000;
		}
		else {
			legalMoves->moves[legalMoves->count].score = board->historyHeuristic[board->pieceList[FROMSQ(legalMoves->moves[legalMoves->count].move)]]
				[TOSQ(legalMoves->moves[legalMoves->count].move)];
		}
	}
	else {
		return;
	}
}

void MoveGeneration::validMoves(S_BOARD* board, S_MOVELIST &legalMoves){
	legalMoves.count = 0;
	S_MOVELIST pseudoLegal;
	pseudoLegalMoves(board, pseudoLegal); // Get all pseudolegal moves
	
	int kingSq = (board->whitesMove == WHITE) ? board->kingPos[0]:board->kingPos[1];
	
	BitBoard potentialBlockers = (board->whitesMove == WHITE) ? genKingRays(kingSq) & board->WHITE_PIECES : genKingRays(kingSq) & board->BLACK_PIECES;

	BitBoard empty = EMPTY;
	
	if (sqAttacked(kingSq, !board->whitesMove, board)){ // King square is attacked by the other side.
		//board->inCheck = true;
		for (int moveNum = 0; moveNum < pseudoLegal.count; moveNum++){
			if (((pseudoLegal.moves[moveNum].move >> 4) & 63) == kingSq){

				piece king = (board->whitesMove == WHITE) ? WK : BK;
				board->position[king] = 0;
				if (!sqAttacked((pseudoLegal.moves[moveNum].move >> 10), !board->whitesMove, board)){
					legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];

					scoreQuiets(board, &legalMoves);

					legalMoves.count++;
				}
				board->position[king] = SETBIT(board->position[king], kingSq);
				continue;
			}
			
			else{ // Other pieces
				// Make the move
				makeMove(*board, pseudoLegal.moves[moveNum].move);
				if (!sqAttacked(kingSq, board->whitesMove, board)){ // Not in check
					undoMove(*board);
					legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];
					scoreQuiets(board, &legalMoves);
					legalMoves.count++;
				}
				else{
					undoMove(*board);
				}
			}
			
		}
		if (legalMoves.count == 0){
			board->is_checkmate = true;
		}
	}
	
	else{ // King not in check
		for (int moveNum = 0; moveNum < pseudoLegal.count; moveNum++){
			
			if (((pseudoLegal.moves[moveNum].move >> 4) & 63) == kingSq){ // King move
				piece this_king = (board->whitesMove == WHITE) ? WK : BK;
				board->position[this_king] = 0;
				if (!sqAttacked((pseudoLegal.moves[moveNum].move >> 10), !board->whitesMove, board)){
					legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];
					scoreQuiets(board, &legalMoves);
					legalMoves.count++;
				}
				board->position[this_king] = SETBIT(board->position[this_king], kingSq);
			}
			
			else if ((pseudoLegal.moves[moveNum].move & 3) == 1){ // En-passant
				makeMove(*board, pseudoLegal.moves[moveNum].move);
				if (!sqAttacked(kingSq, board->whitesMove, board)){
					undoMove(*board);
					legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];
					scoreQuiets(board, &legalMoves);
					legalMoves.count++;
				}
				else{
					undoMove(*board);
					continue;
				}
			}
			
			else if ((SETBIT(empty, ((pseudoLegal.moves[moveNum].move >> 4) & 63)) & potentialBlockers) != 0){
				makeMove(*board, pseudoLegal.moves[moveNum].move);

				if (!sqAttacked(kingSq, board->whitesMove, board)){
					undoMove(*board);
					legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];
					scoreQuiets(board, &legalMoves);
					legalMoves.count++;
				}
				else{
					undoMove(*board);
					continue;
				}
				
				
			}
			
			else {
				legalMoves.moves[legalMoves.count] = pseudoLegal.moves[moveNum];
				legalMoves.count++;
			}
			
		}
		if (legalMoves.count == 0){
			board->is_stalemate = true;
		}
	}
}

void MoveGeneration::pseudoLegalMoves(S_BOARD* boardState, S_MOVELIST& pseudoLegal) {
	pawnMoves(boardState, pseudoLegal);
	knightMoves(boardState, pseudoLegal);
	bishopMoves(boardState, pseudoLegal);
	rookMoves(boardState, pseudoLegal);
	queenMoves(boardState, pseudoLegal);
	kingMoves(boardState, pseudoLegal);
}

void MoveGeneration::kingMoves(S_BOARD* board, S_MOVELIST& s_moves) {
	BitBoard K = 0;
	if (board->whitesMove) {
		K = board->position[WK];
	}
	else {
		K = board->position[BK];
	}
	BitBoard OCCUPIED = ~board->EMPTY_SQUARES;

	// Horizontal and vertical
	BitBoard kingMoves = 0;
	kingMoves = kingMoves | (K & (OCCUPIED ^ FileMasks8[7])) << 1;
	kingMoves = kingMoves | (K & (OCCUPIED ^ FileMasks8[0])) >> 1;
	kingMoves = kingMoves | (K & (OCCUPIED ^ RankMasks8[0])) >> 8;
	kingMoves = kingMoves | (K & (OCCUPIED ^ RankMasks8[7])) << 8;
	
	// Diagonal
	kingMoves = kingMoves | (K & (OCCUPIED ^ (RankMasks8[7] | FileMasks8[7]))) << 9;
	kingMoves = kingMoves | (K & (OCCUPIED ^ (RankMasks8[0] | FileMasks8[7]))) >> 7;
	kingMoves = kingMoves | (K & (OCCUPIED ^ (RankMasks8[0] | FileMasks8[0]))) >> 9;
	kingMoves = kingMoves | (K & (OCCUPIED ^ (RankMasks8[7] | FileMasks8[0]))) << 7;

	int index = bitScanForward(K);
	BitBoard LegalKingMoves = 0;
	if (board->whitesMove) {
		LegalKingMoves |= kingMoves & (board->EMPTY_SQUARES | board->BLACK_PIECES);
	}
	else {
		LegalKingMoves |= kingMoves & (board->EMPTY_SQUARES | board->WHITE_PIECES);
	}
	int destination;
	while (LegalKingMoves != 0) {
		destination = bitScanForward(LegalKingMoves);
		s_moves.moves[s_moves.count].move = newMove(destination, index, 0, 3);
		
		// Set MMV-LVA scores
		if (board->pieceList[destination] != NO_PIECE) {
			s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[destination]][board->pieceList[bitScanForward(K)]] + 1000000;
		}
		
		s_moves.count += 1;
		LegalKingMoves ^= (uint64_t)1 << destination;
	}

	// Castling moves
	if ((board->whitesMove == WHITE && index == 4) || (board->whitesMove == BLACK && index == 60)) {
		int ks = (board->whitesMove == WHITE) ? WKCA : BKCA;
		int qs = (board->whitesMove == WHITE) ? WQCA : BQCA;
		if (((board->castlePerms >> ks) & 1) == 1 && !sqAttacked(index, !board->whitesMove, board)
			&& !sqAttacked(index + 1, !board->whitesMove, board) && !sqAttacked(index + 2, !board->whitesMove, board)) {
			if ((board->whitesMove && ((board->position[WR] >> 7) & 1) == 1) || (!board->whitesMove && ((board->position[BR] >> 63) & 1) == 1)){
				if ((board->whitesMove == WHITE && ((OCCUPIED >> 5) & 1) == 0 && ((OCCUPIED >> 6) & 1) == 0) ||
				(board->whitesMove == BLACK && ((OCCUPIED >> 61) & 1) == 0 && ((OCCUPIED >> 62) & 1) == 0)) {
					s_moves.moves[s_moves.count].move = newMove(index + 2, index, 0, 2);
					s_moves.count += 1;
				}
			}
		}
		if (((board->castlePerms >> qs) & 1) == 1 && !sqAttacked(index, !board->whitesMove, board)
			&& !sqAttacked(index - 1, !board->whitesMove, board) && !sqAttacked(index - 2, !board->whitesMove, board)) {
			if ((board->whitesMove && (board->position[WR] & 1) == 1) || (!board->whitesMove && ((board->position[BR] >> 56) & 1) == 1)){
				if ((board->whitesMove == WHITE && ((OCCUPIED >> 3) & 1) == 0 && ((OCCUPIED >> 2) & 1) == 0 && ((OCCUPIED >> 1) & 1) == 0) ||
				(board->whitesMove == BLACK && ((OCCUPIED >> 59) & 1) == 0 && ((OCCUPIED >> 58) & 1) == 0 && ((OCCUPIED >> 57) & 1) == 0)) {
					s_moves.moves[s_moves.count].move = newMove(index - 2, index, 0, 2);
					s_moves.count += 1;
				}
			}
		}
	}
}

void MoveGeneration::queenMoves(S_BOARD* board, S_MOVELIST& s_moves) {
	rookMoves(board, s_moves, true);
	bishopMoves(board, s_moves, true);
}

void MoveGeneration::bishopMoves(S_BOARD* board, S_MOVELIST& s_moves, bool queen) {
	BitBoard Bishop = 0;
	if (board->whitesMove) {
		if (queen == true) {
			Bishop = board->position[WQ];
		}
		else {
			Bishop = board->position[WB];
		}
	}
	else {
		if (queen == true) {
			Bishop = board->position[BQ];
		}
		else {
			Bishop = board->position[BB];
		}
	}
	BitBoard destinationSquares;
	int index;
	while (Bishop != 0) {
		index = bitScanForward(Bishop);
		destinationSquares = 0;
		
		attacks::positiveDiagAttacks(destinationSquares, (~board->EMPTY_SQUARES), 0, index); // North east
		attacks::positiveDiagAttacks(destinationSquares, (~board->EMPTY_SQUARES), 1, index); // North west
		attacks::negativeDiagAttacks(destinationSquares, (~board->EMPTY_SQUARES), 2, index); // South east
		attacks::negativeDiagAttacks(destinationSquares, (~board->EMPTY_SQUARES), 3, index); // South west
		
		BitBoard validDests = (board->whitesMove == WHITE) ? (destinationSquares & (board->EMPTY_SQUARES | board->BLACK_PIECES)) :
			destinationSquares & (board->EMPTY_SQUARES | board->WHITE_PIECES);

		int validIndex;
		while (validDests != 0) {
			validIndex = bitScanForward(validDests);
			s_moves.moves[s_moves.count].move = newMove(validIndex, index, 0, 3);
			
			// Set MVV-LVA scores
			if (board->pieceList[validIndex] != NO_PIECE) {
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[validIndex]][board->pieceList[index]] + 1000000;
			}
			
			s_moves.count += 1;
			validDests ^= (uint64_t)1 << validIndex;
		}
		Bishop ^= (uint64_t)1 << index;
	}
}

void MoveGeneration::rookMoves(S_BOARD* board, S_MOVELIST& s_moves, bool queen) {
	BitBoard R = 0;
	if (board->whitesMove) {
		if (queen == true) {
			R = board->position[WQ];
		}
		else {
			R = board->position[WR];
		}
	}
	else {
		if (queen == true) {
			R = board->position[BQ];
		}
		else {
			R = board->position[BR];
		}
	}

	int index;
	BitBoard destinationSquares = 0;

	while (R != 0) {
		index = bitScanForward(R);

		destinationSquares = 0;
		attacks::positiveLineAttacks(destinationSquares, ~board->EMPTY_SQUARES, 0, index); // North
		attacks::positiveLineAttacks(destinationSquares, ~board->EMPTY_SQUARES, 2, index); // East
		attacks::negativeLineAttacks(destinationSquares, ~board->EMPTY_SQUARES, 1, index); // South
		attacks::negativeLineAttacks(destinationSquares, ~board->EMPTY_SQUARES, 3, index); // West

		BitBoard validDests = 0;
		if (board->whitesMove) {
			validDests = destinationSquares & (board->EMPTY_SQUARES | board->BLACK_PIECES);
		}
		else {
			validDests = destinationSquares & (board->EMPTY_SQUARES | board->WHITE_PIECES);
		}
		int validIndex;
		while (validDests != 0) {
			validIndex = bitScanForward(validDests);
			s_moves.moves[s_moves.count].move = newMove(validIndex, index, 0, 3);
			
			// Set MVV-LVA scores
			if (board->pieceList[validIndex] != NO_PIECE) {
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[validIndex]][board->pieceList[index]] + 1000000;
			}
			
			s_moves.count += 1;
			validDests ^= (uint64_t)1 << validIndex;
		}
		R ^= (uint64_t)1 << index;
	}
}

void MoveGeneration::knightMoves(S_BOARD* board, S_MOVELIST& s_moves) {
	BitBoard N;
	if (board->whitesMove) { N = board->position[WN]; }
	else { N = board->position[BN]; }
	BitBoard destinationSquares;
	int index;

	while (N != 0) {// theres a knight at index i
		index = bitScanForward(N);
		destinationSquares = 0;
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[7] ^ UNIVERSE)) << 17);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[0] ^ UNIVERSE)) << 15);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) << 10);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) >> 6);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[7] ^ UNIVERSE)) >> 15);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & (FileMasks8[0] ^ UNIVERSE)) >> 17);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) << 6);
		destinationSquares = destinationSquares | ((((uint64_t)1 << index) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) >> 10);

		BitBoard validDestinations = 0;
		if (board->whitesMove) {
			validDestinations = destinationSquares & (~board->WHITE_PIECES);
		}
		else {
			validDestinations = destinationSquares & (~board->BLACK_PIECES);
		}
		int validIndex;
		while (validDestinations != 0) {
			validIndex = bitScanForward(validDestinations);
			s_moves.moves[s_moves.count].move = newMove(validIndex, index, 0, 3);
			
			// Set MVV-LVA scores
			if (board->pieceList[validIndex] != NO_PIECE) {
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[validIndex]][board->pieceList[index]] + 1000000;
			}
			
			s_moves.count += 1;
			validDestinations ^= (uint64_t)1 << validIndex;
		}
		N ^= (uint64_t)1 << index;
	}
}

void MoveGeneration::pawnMoves(S_BOARD* board, S_MOVELIST& s_moves) { // Needs en-passant captures
	if (board->whitesMove) {
		BitBoard WP = board->position[0];
		int index;

		//One square forward
		BitBoard oneSq = (WP << 8) & board->EMPTY_SQUARES;
		while (oneSq != 0) {
			index = bitScanForward(oneSq);
			if (index >= 56) {
				for (int i = 0; i <= 3; i++) {
					s_moves.moves[s_moves.count].move = newMove(index, index - 8, i, 0);
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index - 8, 0, 3);
				s_moves.count += 1;
			}
			oneSq ^= (uint64_t)1 << index;
		}

		//Two squares forward
		BitBoard twoSqForward = ((((WP & RankMasks8[1]) << 8) & board->EMPTY_SQUARES) << 8) & board->EMPTY_SQUARES;

		while (twoSqForward != 0) {
			index = bitScanForward(twoSqForward);
			s_moves.moves[s_moves.count].move = newMove(index, index - 16, 0, 3);
			s_moves.count += 1;
			twoSqForward ^= (uint64_t)1 << index;
		}

		// Attacks to the right
		BitBoard rightAttacks = ((WP & ~FileMasks8[7]) << 9) & board->BLACK_PIECES;
		while (rightAttacks != 0) {
			index = bitScanForward(rightAttacks);
			if (index >= 56) {
				for (int i = 0; i <= 3; i++) {
					s_moves.moves[s_moves.count].move = newMove(index, index - 9, i, 0);
					s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][0] + 1000000;
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index - 9, 0, 3);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][0] + 1000000;
				s_moves.count += 1;
			}
			rightAttacks ^= (uint64_t)1 << index;
		}

		// Attacks to the left
		BitBoard leftAttacks = ((WP & ~FileMasks8[0]) << 7) & board->BLACK_PIECES;
		while (leftAttacks != 0) {
			index = bitScanForward(leftAttacks);
			if (index >= 56) {
				for (int i = 0; i <= 3; i++) {
					s_moves.moves[s_moves.count].move = newMove(index, index - 7, i, 0);
					s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][0] + 1000000;
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index - 7, 0, 3);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][0] + 1000000;
				s_moves.count += 1;
			}
			leftAttacks = CLRBIT(leftAttacks, index);
		}

		// En-passant
		rightAttacks = (WP & ~FileMasks8[7]) << 9;
		leftAttacks = (WP & ~FileMasks8[0]) << 7;
		while (rightAttacks != 0) {
			index = bitScanForward(rightAttacks);
			if (index == board->enPassantSquare) {
				s_moves.moves[s_moves.count].move = newMove(index, index - 9, 0, 1);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index - 8]][0] + 1000000;
				s_moves.count += 1;
			}
			rightAttacks = CLRBIT(rightAttacks, index);
		}
		while (leftAttacks != 0) {
			index = bitScanForward(leftAttacks);
			if (index == board->enPassantSquare) {
				s_moves.moves[s_moves.count].move = newMove(index, index - 7, 0, 1);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index - 8]][0] + 1000000;
				s_moves.count += 1;
			}
			leftAttacks = CLRBIT(leftAttacks, index);
		}
	}

	else {
		BitBoard BP = board->position[6];
		int index = 0;

		// One square moves
		BitBoard oneSquare = (BP >> 8) & board->EMPTY_SQUARES;
		while (oneSquare != 0) {
			index = bitScanForward(oneSquare);
			if (index <= 7) { // rank 1
				for (int piece = 0; piece <= 3; piece++) {
					s_moves.moves[s_moves.count].move = newMove(index, index + 8, piece, 0);
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index + 8, 0, 3);
				s_moves.count += 1;
			}
			oneSquare ^= (uint64_t)1 << index;
		}

		// Two squares initial moves
		BitBoard twoSquare = ((((BP & RankMasks8[6]) >> 8) & board->EMPTY_SQUARES) >> 8) & board->EMPTY_SQUARES;
		while (twoSquare != 0) {
			index = bitScanForward(twoSquare);
			s_moves.moves[s_moves.count].move = newMove(index, index + 16, 0, 3);
			s_moves.count += 1;
			twoSquare ^= (uint64_t)1 << index;
		}

		// Attacks to the right
		BitBoard rightAttacks = ((BP & ~FileMasks8[7]) >> 7) & board->WHITE_PIECES;
		while (rightAttacks != 0) {
			index = bitScanForward(rightAttacks);
			if (index <= 7) {
				for (int piece = 0; piece <= 3; piece++) {
					s_moves.moves[s_moves.count].move = newMove(index, index + 7, piece, 0);
					s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][6] + 1000000;
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index + 7, 0, 3);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][6] + 1000000;
				s_moves.count += 1;
			}
			rightAttacks ^= (uint64_t)1 << index;
		}


		// Attacks to the left
		BitBoard leftAttacks = ((BP & ~FileMasks8[0]) >> 9) & board->WHITE_PIECES;
		while (leftAttacks != 0) {
			index = bitScanForward(leftAttacks);
			if (index <= 7) {
				for (int piece = 0; piece <= 3; piece++) {
					s_moves.moves[s_moves.count].move = newMove(index, index + 9, piece, 0);
					s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][6] + 1000000;
					s_moves.count += 1;
				}
			}
			else {
				s_moves.moves[s_moves.count].move = newMove(index, index + 9, 0, 3);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index]][6] + 1000000;
				s_moves.count += 1;
			}
			leftAttacks ^= (uint64_t)1 << index;
		}

		// En-passant
		rightAttacks = (BP & ~FileMasks8[7]) >> 7;
		leftAttacks = (BP & ~FileMasks8[0]) >> 9;
		while (rightAttacks != 0) {
			index = bitScanForward(rightAttacks);
			if (index == board->enPassantSquare) {
				s_moves.moves[s_moves.count].move = newMove(index, index + 7, 0, 1);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index + 8]][6] + 1000000;
				s_moves.count += 1;
			}
			rightAttacks = CLRBIT(rightAttacks, index);
		}
		while (leftAttacks != 0) {
			index = bitScanForward(leftAttacks);
			if (index == board->enPassantSquare) {
				s_moves.moves[s_moves.count].move = newMove(index, index + 9, 0, 1);
				s_moves.moves[s_moves.count].score = MvvLva[board->pieceList[index + 8]][6] + 1000000;
				s_moves.count += 1;
			}
			leftAttacks = CLRBIT(leftAttacks, index);
		}
	}
}

inline bool moveExists(S_BOARD* pos, const int move) {
	S_MOVELIST list;
	MoveGeneration::validMoves(pos, list);

	for (int i = 0; i < list.count; i++){
		if (list.moves[i].move == move) {
			return true;
		}
	}
	return false;
}

