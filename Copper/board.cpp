#include "defs.h"


/*
INCLUDES THE FUNCTIONS:
- parseFen
- BoardRepresentation::displayBoardState
*/

void BoardRep::displayBoardState(S_BOARD board) {
	std::string output = "................................................................";
	for (int index = 0; index < 12; index++) {
		for (int i = 0; i < 64; i++) {
			if (((board.position[index] >> i) & 1) == 1) {
				if (index == 0) { output[i] = 'P'; }
				else if (index == 1) { output[i] = 'N'; }
				else if (index == 2) { output[i] = 'B'; }
				else if (index == 3) { output[i] = 'R'; }
				else if (index == 4) { output[i] = 'Q'; }
				else if (index == 5) { output[i] = 'K'; }
				else if (index == 6) { output[i] = 'p'; }
				else if (index == 7) { output[i] = 'n'; }
				else if (index == 8) { output[i] = 'b'; }
				else if (index == 9) { output[i] = 'r'; }
				else if (index == 10) { output[i] = 'q'; }
				else if (index == 11) { output[i] = 'k'; }
			}
		}
	}
	std::string sideToMove = (board.whitesMove == WHITE) ? "White" : "Black";
	std::cout << "\nGame state:\n\n";
	for (int rank = 7; rank >= 0; rank--) {
		std::cout << rank + 1 << "  ";
		for (int file = 0; file < 8; file++) {
			std::cout << output[(int)8 * rank + file] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	std::cout << "   a b c d e f g h" << std::endl;
	std::cout << "\n    Side to move: " << sideToMove << std::endl;
	std::printf("\n\n    Position key: %llX\n", board.posKey);
}


void BoardRep::parseFen(const char* fen, S_BOARD& pos){
	ASSERT(fen != NULL);
	
	clearBoard(&pos);
	/*for (int i = 0; i < 4; i++){
		pos.castlingRights[i] = false;
	}*/
	pos.castlePerms = 0;
	
	int rank = RANK_8;
	int file = FILE_A;
	int piece = 0;
	int count = 0;
	int i = 0;
	int index = 0;
	
	while ((rank >= RANK_1) && *fen){
		count = 1;
		switch (*fen){
			case 'p': piece = BP; break;
			case 'n': piece = BN; break;
			case 'b': piece = BB; break;
			case 'r': piece = BR; break;
			case 'q': piece = BQ; break;
			case 'k': piece = BK; break;
			case 'P': piece = WP; break;
			case 'N': piece = WN; break;
			case 'B': piece = WB; break;
			case 'R': piece = WR; break;
			case 'Q': piece = WQ; break;
			case 'K': piece = WK; break;
				
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
				piece = NO_PIECE;
				count = *fen - '0';
				break;
			
			case '/':
			case ' ':
				rank--;
				file = FILE_A;
				fen++;
				continue;
			
			default:
				printf("FEN error \n");
				return;
		}
		
		for (i = 0; i < count; i++){
			index = rank * 8 + file;
			pos.pieceList[index] = piece;
			file++;
		}
		fen++;
	}
	
	ASSERT(*fen == 'w' || *fen == 'b');
	
	pos.whitesMove = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;
	
	for (i = 0; i < 4; i++) {
		if (*fen == ' ') {
			break;
		}
		switch(*fen) {
			case 'K': pos.castlePerms = SETBIT(pos.castlePerms, WKCA); break;
			case 'Q': pos.castlePerms = SETBIT(pos.castlePerms, WQCA); break;
			case 'k': pos.castlePerms = SETBIT(pos.castlePerms, BKCA); break;
			case 'q': pos.castlePerms = SETBIT(pos.castlePerms, BQCA); break;
			default:         break;
		}
		fen++;
	}
	fen++;
	
	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';
		
		ASSERT(file>=FILE_A && file <= FILE_H);
		ASSERT(rank>=RANK_1 && rank <= RANK_8);
		
		pos.enPassantSquare = rank * 8 + file;
	}
	
	for (i = 0; i < 64; i++){
		if (pos.pieceList[i] != NO_PIECE){
			pos.position[pos.pieceList[i]] = SETBIT(pos.position[pos.pieceList[i]], i);
			if (pos.pieceList[i] == WK || pos.pieceList[i] == BK){
				if (pos.pieceList[i] == WK){
					pos.kingPos[0] = i;
				}
				else{
					pos.kingPos[1] = i;
				}
			}
		}
	}
	
	for (int i = WP; i <= WK; i++){
		pos.WHITE_PIECES |= pos.position[i];
	}
	for (int i = BP; i <= BK; i++){
		pos.BLACK_PIECES |= pos.position[i];
	}
	pos.EMPTY_SQUARES = ~(pos.BLACK_PIECES | pos.WHITE_PIECES);
	pos.posKey = generatePosKey(&pos);
}

void BoardRep::clearBoard(S_BOARD* pos) {
	for (int i = 0; i < 12; i++) {
		pos->position[i] = (uint64_t)0;
	}

	for (int i = 0; i < 64; i++) {
		pos->pieceList[i] = NO_PIECE;
	}

	pos->posKey = 0;

	pos->castlePerms = 0;
	pos->enPassantSquare = NO_SQ;

	pos->kingPos[0] = NO_SQ;
	pos->kingPos[1] = NO_SQ;

	pos->EMPTY_SQUARES = 0;
	pos->WHITE_PIECES = 0;
	pos->BLACK_PIECES = 0;

	pos->fiftyMove = 0;

	TT::clearTable(pos->transpositionTable);

	pos->is_checkmate = false;
	pos->is_stalemate = false;

	for (int i = 0; i < MAXDEPTH; i++) {
		pos->pvArray[i] = NOMOVE;
		pos->killerMoves[i][0] = 0;
	}

	for (int i = 0; i < 12; i++) {
		for (int j = 0; j < 64; j++) {
			pos->historyHeuristic[i][j] = 0;
		}
	}
	
	pos->ply = 0;

	pos->history.moveCount = 0;
	for (int i = 0; i < MAXGAMEMOVES; i++) {
		pos->history.history[i] = {};
	}
}
