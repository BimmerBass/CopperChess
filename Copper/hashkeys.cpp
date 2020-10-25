#include "defs.h"

/*
INCLUDES THE FUNCTIONS:
    - generatePosKey
*/

BitBoard generatePosKey(const S_BOARD* pos) {
    BitBoard finalKey = 0;

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
