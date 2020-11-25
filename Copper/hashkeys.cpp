#include "defs.h"

/*
INCLUDES THE FUNCTIONS:
    - generatePosKey
    - generatePawnHash
*/

uint64_t generatePawnHash(const S_BOARD* pos) {
    uint64_t finalKey = 0;

    BitBoard pawnBoard = pos->position[WP];
    if (pawnBoard != 0) {
        while (pawnBoard != 0) {
            finalKey ^= pieceKeys[WP][PopBit(&pawnBoard)];
        }
    }

    pawnBoard = pos->position[BP];
    if (pawnBoard != 0) {
        while (pawnBoard != 0) {
            finalKey ^= pieceKeys[BP][PopBit(&pawnBoard)];
        }
    }

    return finalKey;
}

uint64_t generatePosKey(const S_BOARD* pos) {
    uint64_t finalKey = 0;

    for (int pce_t = 0; pce_t < 12; pce_t++) {
        BitBoard brd = pos->position[pce_t];
        if (brd == 0) {
            continue;
        }
        
        while (brd != 0) {
            finalKey ^= pieceKeys[pce_t][PopBit(&brd)];
        }
    }

    if (pos->whitesMove == WHITE) {
        finalKey ^= sideKey;
    }

    if (pos->enPassantSquare != NO_SQ) {
        ASSERT(pos->enPassantSquare >= 0 && pos->enPassantSquare <= 63);
        finalKey ^= pieceKeys[NO_PIECE][pos->enPassantSquare];
    }

    ASSERT(castleNum >= 0 && castleNum <= 15);

    finalKey ^= castleKeys[pos->castlePerms];

    return finalKey;
}
