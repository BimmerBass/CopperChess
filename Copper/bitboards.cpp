#include "defs.h"

/*
INCLUDES THE FUNCTIONS:
- arrayToBitBoards
*/


// This function either needs to be removed or updated as the engine uses FEN-strings now.
S_BOARD BoardRep::arrayToBitboards(std::string arrayRepresentation[8][8], S_SIDE white, bool castlingPermitions[4], int enPassant) {
    S_BOARD positionBitboards;
    BitBoard WP = 0, WN = 0, WB = 0, WR = 0, WQ = 0, WK = 0, BP = 0, BN = 0, BB = 0, BR = 0, BQ = 0, BK = 0;
    std::vector<std::string> squares = genSquareList(arrayRepresentation);
    std::string binary;
    for (int i = 0; i < 64; i++) {
        binary = "0000000000000000000000000000000000000000000000000000000000000000";
        binary = binary.substr(i + 1) + "1" + binary.substr(0, i);
        if (squares[i] == "P") { WP = WP | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 0;  }
        else if (squares[i] == "N") { WN = WN | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 1; }
        else if (squares[i] == "B") { WB = WB | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 2; }
        else if (squares[i] == "R") { WR = WR | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 3; }
        else if (squares[i] == "Q") { WQ = WQ | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 4; }
        else if (squares[i] == "K") { WK = WK | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 5; }
        else if (squares[i] == "p") { BP = BP | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 6; }
        else if (squares[i] == "n") { BN = BN | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 7; }
        else if (squares[i] == "b") { BB = BB | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 8; }
        else if (squares[i] == "r") { BR = BR | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 9; }
        else if (squares[i] == "q") { BQ = BQ | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 10; }
        else if (squares[i] == "k") { BK = BK | stoull(binary, nullptr, 2); positionBitboards.pieceList[i] = 11; }
        else{ // Empty square
            positionBitboards.pieceList[i] = -1;
        }
    }

    positionBitboards.position[0] = WP;
    positionBitboards.position[1] = WN;
    positionBitboards.position[2] = WB;
    positionBitboards.position[3] = WR;
    positionBitboards.position[4] = WQ;
    positionBitboards.position[5] = WK;
    positionBitboards.position[6] = BP;
    positionBitboards.position[7] = BN;
    positionBitboards.position[8] = BB;
    positionBitboards.position[9] = BR;
    positionBitboards.position[10] = BQ;
    positionBitboards.position[11] = BK;
    
    positionBitboards.BLACK_PIECES = (BP | BN | BB | BR | BQ | BK);
    positionBitboards.WHITE_PIECES = (WP | WN | WB | WR | WQ | WK);
    positionBitboards.EMPTY_SQUARES = ~(positionBitboards.BLACK_PIECES | positionBitboards.WHITE_PIECES);

    positionBitboards.whitesMove = white;
    if (enPassant != NO_SQ) {
        positionBitboards.enPassantSquare = enPassant;
    }
    for (int i = 0; i < 4; i++) {
        //positionBitboards.castlingRights[i] = castlingPermitions[i];
        if (castlingPermitions[i]) {
            positionBitboards.castlePerms = SETBIT(positionBitboards.castlePerms, i);
        }
    }
    for (int r = 7; r >= 0; r--) {
        for (int f = 0; f < 8; f++) {
            if (arrayRepresentation[r][f] == "K") {
                positionBitboards.kingPos[0] = (7 - r) * 8 + f;
            }
            else if (arrayRepresentation[r][f] == "k") {
                positionBitboards.kingPos[1] = (7 - r) * 8 + f;
            }
        }
    }


    return positionBitboards;
}
