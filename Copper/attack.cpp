#include "defs.h"

/*
INCLUDES THE FUNCTIONS:

- sqAttacked
- positiveLineAttacks
- negativeLineAttacks
- positiveDiagAttacks
- negativeDiagAttacks
- badCapture

*/

bool sqAttacked(int sq, bool side, const S_BOARD* pos) { // Attacked by side == side.
    const BitBoard empty = EMPTY;
    BitBoard OCCUPIED = (pos->position[WP] | pos->position[WN] | pos->position[WB] | pos->position[WR] | pos->position[WQ] | pos->position[WK] | pos->position[BP] | pos->position[BN] | pos->position[BB] | pos->position[BR] | pos->position[BQ] | pos->position[BK]);
    BitBoard sqIndex = (uint64_t)1 << sq;
    // Pawn moves
    if (side == WHITE) {
        if ((((pos->position[WP] & ~FileMasks8[7]) << 9) & SETBIT(empty, sq)) != 0 || (((pos->position[WP] & ~FileMasks8[0]) << 7) & SETBIT(empty, sq)) != 0) {
            return true;
        }
    }
    else {
        if ((((pos->position[BP] & ~FileMasks8[0]) >> 9) & SETBIT(empty, sq)) != 0 || (((pos->position[BP] & ~FileMasks8[7]) >> 7) & SETBIT(empty, sq)) != 0) {
            return true;
        }
    }

    // Knight moves
    BitBoard destinationSquares = 0;
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & (FileMasks8[7] ^ UNIVERSE)) << 17);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & (FileMasks8[0] ^ UNIVERSE)) << 15);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) << 10);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & ((FileMasks8[6] | FileMasks8[7]) ^ UNIVERSE)) >> 6);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & (FileMasks8[7] ^ UNIVERSE)) >> 15);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & (FileMasks8[0] ^ UNIVERSE)) >> 17);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) << 6);
    destinationSquares = destinationSquares | ((((uint64_t)1 << sq) & ((FileMasks8[0] | FileMasks8[1]) ^ UNIVERSE)) >> 10);

    if ((side == WHITE) && (pos->position[WN] & destinationSquares) != 0) {
        return true;
    }
    else if ((side == BLACK) && (pos->position[BN] & destinationSquares) != 0) {
        return true;
    }

    // Bishop and queen moves
    BitBoard BQ_attacks = 0;
    attacks::positiveDiagAttacks(BQ_attacks, OCCUPIED, 0, sq); // North east
    attacks::positiveDiagAttacks(BQ_attacks, OCCUPIED, 1, sq); // North west
    attacks::negativeDiagAttacks(BQ_attacks, OCCUPIED, 2, sq); // South east
    attacks::negativeDiagAttacks(BQ_attacks, OCCUPIED, 3, sq);
    if ((side == WHITE) && ((pos->position[WQ] | pos->position[WB]) & BQ_attacks) != 0) {
        return true;
    }
    else if ((side == BLACK) && ((pos->position[BQ] | pos->position[BB]) & BQ_attacks) != 0) {
        return true;
    }

    // Rook and queen attacks
    BitBoard QR_attacks = 0;
    attacks::positiveLineAttacks(QR_attacks, OCCUPIED, 0, sq); // North
    attacks::positiveLineAttacks(QR_attacks, OCCUPIED, 2, sq); // East
    attacks::negativeLineAttacks(QR_attacks, OCCUPIED, 1, sq); // South
    attacks::negativeLineAttacks(QR_attacks, OCCUPIED, 3, sq); // West
    if ((side == WHITE) && ((pos->position[WR] | pos->position[WQ]) & QR_attacks) != 0) {
        return true;
    }
    else if ((side == BLACK) && ((pos->position[BR] | pos->position[BQ]) & QR_attacks) != 0) {
        return true;
    }

    // King attacks
    BitBoard kingMoves = 0;
    kingMoves = kingMoves | (sqIndex & ~FileMasks8[7]) << 1;
    kingMoves = kingMoves | (sqIndex & ~FileMasks8[0]) >> 1;
    kingMoves = kingMoves | (sqIndex & ~RankMasks8[0]) >> 8;
    kingMoves = kingMoves | (sqIndex & ~RankMasks8[7]) << 8;

    // Diagonal
    kingMoves = kingMoves | (sqIndex & ~(RankMasks8[7] | FileMasks8[7])) << 9;
    kingMoves = kingMoves | (sqIndex & ~(RankMasks8[0] | FileMasks8[7])) >> 7;
    kingMoves = kingMoves | (sqIndex & ~(RankMasks8[0] | FileMasks8[0])) >> 9;
    kingMoves = kingMoves | (sqIndex & ~(RankMasks8[7] | FileMasks8[0])) << 7;
    if ((side == WHITE) && (pos->position[WK] & kingMoves) != 0) {
        return true;
    }
    else if ((side == BLACK) && (pos->position[BK] & kingMoves) != 0) {
        return true;
    }




    return false;
}


inline void attacks::positiveLineAttacks(BitBoard &attackRays, BitBoard OCCUPIED, int direction, int sq){
    BitBoard attacks = LineAttackRays[direction][sq];
    BitBoard blockers = attacks & OCCUPIED;
    if (blockers != 0){
        int blocker = bitScanForward(blockers);
        attacks ^= LineAttackRays[direction][blocker];
    }
    attackRays |= attacks;
}

inline void attacks::negativeLineAttacks(BitBoard &attackRays, BitBoard OCCUPIED, int direction, int sq){
    BitBoard attacks = LineAttackRays[direction][sq];
    BitBoard blockers = attacks & OCCUPIED;
    if (blockers != 0){
        int blocker = bitScanReverse(blockers);
        attacks ^= LineAttackRays[direction][blocker];
    }
    attackRays |= attacks;
}

inline void attacks::positiveDiagAttacks(BitBoard &attackRays, BitBoard OCCUPIED, int direction, int sq) {
    BitBoard attacks = DiagonalAttackRays[direction][sq];
    BitBoard blockers = attacks & OCCUPIED;
    if (blockers != 0) {
        int blocker = bitScanForward(blockers);
        attacks ^= DiagonalAttackRays[direction][blocker];
    }
    attackRays |= attacks;
}

inline void attacks::negativeDiagAttacks(BitBoard &attackRays, BitBoard OCCUPIED, int direction, int sq) {
    BitBoard attacks = DiagonalAttackRays[direction][sq];
    BitBoard blockers = attacks & OCCUPIED;
    if (blockers != 0) {
        int square = bitScanReverse(blockers);
        attacks ^= DiagonalAttackRays[direction][square];
    }
    attackRays |= attacks;
}