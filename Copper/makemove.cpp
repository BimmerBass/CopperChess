#include "defs.h"

/*
INCLUDES THE FUNCTIONS:
- makeMove()
- undoMove()
*/

inline bool InCheck(const S_BOARD* pos) {
	if (pos->whitesMove == WHITE) {
		return sqAttacked(pos->kingPos[0], BLACK, pos);
	}
	else {
		return sqAttacked(pos->kingPos[1], WHITE, pos);
	}
	return false;
}


void MoveGeneration::undoMove(S_BOARD &board){
	if (board.history.moveCount > 0){
		S_UNDO previousPos = board.history.history[board.history.moveCount - 1];
		
		std::copy(previousPos.bitboards, previousPos.bitboards + 12, board.position);

		std::fill(board.pieceList, board.pieceList + 64, NO_PIECE);

		BitBoard pceBrd = 0;
		int sq = NO_SQ;
		for (int pce = 0; pce < 12; pce++) {
			pceBrd = board.position[pce];

			while (pceBrd != 0){
				sq = PopBit(&pceBrd);
				
				board.pieceList[sq] = pce;
			}
		}

		board.whitesMove = previousPos.side;

		board.inCheck = previousPos.inCheck;

		board.castlePerms = previousPos.castlingPerms;
		board.enPassantSquare = previousPos.enPassantSq;
		board.fiftyMove = previousPos.fiftyMove;
		
		int whiteKingPos = bitScanForward(previousPos.bitboards[WK]);
		int blackKingPos = bitScanForward(previousPos.bitboards[BK]);
		board.kingPos[0] = whiteKingPos;
		board.kingPos[1] = blackKingPos;
		
		board.WHITE_PIECES = (board.position[WP] | board.position[WN] | board.position[WB] | board.position[WR] | board.position[WQ] | board.position[WK]);
		board.BLACK_PIECES = (board.position[BP] | board.position[BN] | board.position[BB] | board.position[BR] | board.position[BQ] | board.position[BK]);
		board.EMPTY_SQUARES = ~(board.WHITE_PIECES | board.BLACK_PIECES);
		
		board.posKey = previousPos.key;

		board.ply -= 1;
		board.history.moveCount -= 1;
	}

	else {
		return;
	}
}


void MoveGeneration::makeMove(S_BOARD &board, int move){
	S_UNDO positionDescriptors;
	for (int i = 0; i < 12; i++){
		positionDescriptors.bitboards[i] = board.position[i];
	}

	positionDescriptors.side = board.whitesMove;
	
	positionDescriptors.castlingPerms = board.castlePerms;
	positionDescriptors.enPassantSq = board.enPassantSquare;
	positionDescriptors.fiftyMove = board.fiftyMove;

	positionDescriptors.key = board.posKey;
	positionDescriptors.inCheck = board.inCheck;


	int destination = move >> 10;
	int origin = (move >> 4) & 63;
	int promotion = (move >> 2) & 3;
	int spcFlags = move & 3;

	int pieceMoved = board.pieceList[origin];
	int pieceCaptured = board.pieceList[destination];
	board.pieceList[origin] = NO_PIECE;
	board.enPassantSquare = NO_SQ;

	if (pieceMoved == WP || pieceMoved == BP) {

		if (spcFlags == 0) { // Promotion
			// Remove origin
			board.position[pieceMoved] = CLRBIT(board.position[pieceMoved], origin);

			if (board.whitesMove) {
				board.position[promoPieces[promotion][0]] = SETBIT(board.position[promoPieces[promotion][0]], destination);
				board.pieceList[destination] = promoPieces[promotion][0];
			}
			else {
				board.position[promoPieces[promotion][1]] = SETBIT(board.position[promoPieces[promotion][1]], destination);
				board.pieceList[destination] = promoPieces[promotion][1];
			}

			if (pieceCaptured != NO_PIECE) {
				board.position[pieceCaptured] = CLRBIT(board.position[pieceCaptured], destination);
			}
		}

		else if (spcFlags == 1) { // En-passant
			// Remove origin and place at destination
			board.position[pieceMoved] = SETBIT(CLRBIT(board.position[pieceMoved], origin), destination);
			board.pieceList[destination] = pieceMoved;

			// Remove captured pawn
			if (board.whitesMove == WHITE) {
				board.position[BP] = CLRBIT(board.position[BP], destination - 8);
				board.pieceList[destination - 8] = NO_PIECE;
			}
			else {
				board.position[WP] = CLRBIT(board.position[WP], destination + 8);
				board.pieceList[destination + 8] = NO_PIECE;
			}
		}

		else { // Regular pawn move
			// Place on destination and remove from origin
			board.position[pieceMoved] = SETBIT(CLRBIT(board.position[pieceMoved], origin), destination);
			board.pieceList[destination] = pieceMoved;

			if (abs(destination - origin) == 16) { // Two square move -> set en passant square
				board.enPassantSquare = (pieceMoved == WP) ? (destination - 8) : (destination + 8);
			}

			if (pieceCaptured != NO_PIECE) {
				board.position[pieceCaptured] = CLRBIT(board.position[pieceCaptured], destination);
			}
		}
	}


	else if (pieceMoved == WK || pieceMoved == BK) {
		if (pieceMoved == WK) {
			board.kingPos[0] = destination;
		}
		else {
			board.kingPos[1] = destination;
		}

		if (spcFlags == 2) { // Castling move
			// Place king on destination
			board.position[pieceMoved] = CLRBIT(SETBIT(board.position[pieceMoved], destination), origin);
			board.pieceList[destination] = pieceMoved;

			if (destination == origin + 2) { // King side

				if (board.whitesMove == WHITE) {
					board.position[WR] = SETBIT(CLRBIT(board.position[WR], 7), destination - 1);
					board.pieceList[7] = NO_PIECE;
					board.pieceList[destination - 1] = WR;

					board.castlePerms = CLRBIT(board.castlePerms, WKCA);
					board.castlePerms = CLRBIT(board.castlePerms, WQCA);

				}
				else {
					board.position[BR] = SETBIT(CLRBIT(board.position[BR], 63), destination - 1);
					board.pieceList[63] = NO_PIECE;
					board.pieceList[destination - 1] = BR;

					board.castlePerms = CLRBIT(board.castlePerms, BKCA);
					board.castlePerms = CLRBIT(board.castlePerms, BQCA);

				}
			}

			else { // Queen side
				if (board.whitesMove) {
					board.position[WR] = SETBIT(CLRBIT(board.position[WR], 0), destination + 1);
					board.pieceList[0] = NO_PIECE;
					board.pieceList[destination + 1] = WR;

					board.castlePerms = CLRBIT(board.castlePerms, WKCA);
					board.castlePerms = CLRBIT(board.castlePerms, WQCA);

				}
				else {
					board.position[BR] = SETBIT(CLRBIT(board.position[BR], 56), destination + 1);
					board.pieceList[56] = NO_PIECE;
					board.pieceList[destination + 1] = BR;

					board.castlePerms = CLRBIT(board.castlePerms, BKCA);
					board.castlePerms = CLRBIT(board.castlePerms, BQCA);

				}
			}
		}

		else { // Regular king move
			// Remove origin and place on destination
			board.position[pieceMoved] = SETBIT(CLRBIT(board.position[pieceMoved], origin), destination);
			board.pieceList[destination] = pieceMoved;

			if (pieceCaptured != NO_PIECE) {
				board.position[pieceCaptured] = CLRBIT(board.position[pieceCaptured], destination);
			}

			// Adjust castling rights
			if (board.whitesMove == WHITE) {
				board.castlePerms = CLRBIT(board.castlePerms, WKCA);
				board.castlePerms = CLRBIT(board.castlePerms, WQCA);
			}
			else {
				board.castlePerms = CLRBIT(board.castlePerms, BKCA);
				board.castlePerms = CLRBIT(board.castlePerms, BQCA);
			}

		}

	}


	else {
		// Change castling rights if rook-move.
		if (pieceMoved == WR || pieceMoved == BR) {

			// If a white rook is moved and wks OR wqs is true.
			if (pieceMoved == WR && (((board.castlePerms >> WKCA) & 1) == 1 || ((board.castlePerms >> WQCA) & 1) == 1)) {
				if (origin == 0 && ((board.castlePerms >> WQCA) & 1) == 1) {
					board.castlePerms = CLRBIT(board.castlePerms, WQCA);
				}
				else if (origin == 7 && ((board.castlePerms >> WKCA) & 1) == 1) {
					board.castlePerms = CLRBIT(board.castlePerms, WKCA);
				}
			}

			else if (pieceMoved == BR && (((board.castlePerms >> BKCA) & 1) == 1 || ((board.castlePerms >> BQCA) & 1) == 1)) {
				if (origin == 63 && ((board.castlePerms >> BKCA) & 1) == 1) {
					board.castlePerms = CLRBIT(board.castlePerms, BKCA);
				}
				else if (origin == 56 && ((board.castlePerms >> BQCA) & 1) == 1) {
					board.castlePerms = CLRBIT(board.castlePerms, BQCA);
				}
			}

		}

		// Remove piece from origin and place on destination
		board.position[pieceMoved] = SETBIT(CLRBIT(board.position[pieceMoved], origin), destination);
		if (pieceCaptured != NO_PIECE) { // If the destination is an enemy piece
			board.position[pieceCaptured] = CLRBIT(board.position[pieceCaptured], destination);
		}
		board.pieceList[destination] = pieceMoved;
	}
	
	board.posKey = generatePosKey(&board);

	if (pieceMoved == WP || pieceMoved == BP || pieceCaptured != NO_PIECE) {
		board.fiftyMove = 0;
	}
	else {
		board.fiftyMove += 1;
	}
	
	board.whitesMove = (board.whitesMove == WHITE) ? BLACK : WHITE;
	
	board.WHITE_PIECES = (board.position[WP] | board.position[WN] | board.position[WB] | board.position[WR] | board.position[WQ] | board.position[WK]);
	board.BLACK_PIECES = (board.position[BP] | board.position[BN] | board.position[BB] | board.position[BR] | board.position[BQ] | board.position[BK]);
	board.EMPTY_SQUARES = ~(board.WHITE_PIECES | board.BLACK_PIECES);

	board.ply += 1;
	board.history.history[board.history.moveCount] = positionDescriptors;
	board.history.moveCount++;

	board.inCheck = InCheck(&board);
}


void makeNullMove(S_BOARD* pos) {
	pos->whitesMove = (pos->whitesMove == WHITE) ? BLACK : WHITE;
	pos->posKey ^= sideKey;
	pos->ply++;
	if (pos->enPassantSquare != NO_SQ) {
		pos->posKey ^= pieceKeys[NO_PIECE][pos->enPassantSquare];
		pos->enPassantSquare = NO_SQ;
	}
}

void undoNullMove(S_BOARD* pos, int ep) {
	pos->whitesMove = (pos->whitesMove == WHITE) ? BLACK : WHITE;
	pos->posKey ^= sideKey;
	pos->ply--;
	if (ep != NO_SQ) {
		pos->posKey ^= pieceKeys[NO_PIECE][ep];
		pos->enPassantSquare = ep;
	}
}