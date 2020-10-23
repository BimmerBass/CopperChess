#include "defs.h"
#include <string>

/*
INCLUDES THE FUNCTIONS:
    - parseMove
    - printMovelist
    - printMove
*/


std::string printMove(int move){
    std::string out = index_to_square(FROMSQ(move)) + index_to_square(TOSQ(move));
    if (SPECIAL(move) == 0){ // Promotion.
        if (PROMTO(move) == 0){
            out += "n";
        }
        else if (PROMTO(move) == 1){
            out += "b";
        }
        else if (PROMTO(move) == 2){
            out += "r";
        }
        else{
            out += "q";
        }
    }
    
    
    return out;
}


void printMovelist(S_BOARD* board) {
    int move = 0;
    int score = 0;
    std::string toPce = "";
    S_MOVELIST list;
    MoveGeneration::validMoves(board, list);
    std::cout << "\n";
    for (int i = 0; i < list.count; i++) {
        move = list.moves[i].move;
        score = list.moves[i].score;
        std::string to = index_to_square(TOSQ(move));
        std::string from = index_to_square(FROMSQ(move));
        if (SPECIAL(move) == 0) { // Promotion
            if (PROMTO(move) == 0) {
                toPce = "n";
            }
            else if (PROMTO(move) == 1) {
                toPce = "b";
            }
            else if (PROMTO(move) == 2) {
                toPce = "r";
            }
            else {
                toPce = "q";
            }
            std::cout << "Move " << i + 1 << ": " << from << to << toPce << " (score: " << score << ")" << std::endl;
            continue;
        }
        std::cout << "Move " << i + 1 << ": " << from << to << " (score: " << score << ")" << std::endl;
        
    }
    return;
}


int parseMove(std::string userInput, S_BOARD* board) {
    if (userInput.length() > 5) { return NOMOVE; }
    if (userInput[0] < 'a' || userInput[0] > 'h') { return NOMOVE; }
    if (userInput[1] < '1' || userInput[1] > '8') { return NOMOVE; }
    if (userInput[2] < 'a' || userInput[2] > 'h') { return NOMOVE; }
    if (userInput[3] < '1' || userInput[3] > '8') { return NOMOVE; }

    int from = ((userInput[0] - 'a') + (userInput[1] - '1') * 8);
    int to = ((userInput[2] - 'a') + (userInput[3] - '1') * 8);
    
    ASSERT(from >= 0 && from <= 63);
    ASSERT(to >= 0 && to <= 63);

    S_MOVELIST list;
    MoveGeneration::validMoves(board, list);
    int move = 0;
    int promPce = NO_PIECE;
    int spcFlag = 3;

    for (int moveNum = 0; moveNum < list.count; moveNum++) {
        move = list.moves[moveNum].move;
        if (FROMSQ(move) == from && TOSQ(move) == to) {
            spcFlag = SPECIAL(move);
            if (spcFlag == 0) { // promotion
                promPce = PROMTO(move);
                if (toupper(userInput[4]) == 'N' && promPce == 0) {
                    return move;
                }
                else if (toupper(userInput[4]) == 'B' && promPce == 1) {
                    return move;
                }
                else if (toupper(userInput[4]) == 'R' && promPce == 2) {
                    return move;
                }
                else if (toupper(userInput[4]) == 'Q' && promPce == 3) {
                    return move;
                }
                continue;
            }
            return move;
        }
    }

    return NOMOVE;
}
