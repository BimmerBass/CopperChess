#include "defs.h"
#include "psqt.h"

/*
INCLUDES THE FUNCTIONS:
	- staticEval(S_BOARD* board)
	- getMaterial(const S_BOARD* pos, bool side)
*/

const int passedPawnValue[8] = { 0, 5, 10, 25, 35, 60, 100, 140 };

const int mirrorRankNum[8] = { 7 , 6 , 5 , 4 , 3 , 2 , 1 , 0 };

int eval::staticEval(const S_BOARD* pos, int depth, int alpha, int beta) {
	int score = 0;

	int mgScore = 0;
	int mgWeight = 0;

	int egScore = 0;
	int egWeight = 0;

	int phase = 0;

	int move = NOMOVE;
	/*if (TT::probePos(pos, depth, alpha, beta, &move, &score)) {
		return score;
	}*/
	if (pos->evaluationCache->probeCache(pos, score) == true) {
		return score;
	}

	bool endgame = ((pos->position[WQ] | pos->position[BQ]) == 0 || (pos->position[WN] | pos->position[WB] | pos->position[WR]
		| pos->position[BN] | pos->position[BB] | pos->position[BR]) == 0) ? true : false;

	int WpawnCnt = countBits(pos->position[WP]);
	int BpawnCnt = countBits(pos->position[BP]);
	// Material and psqt's
	for (int sq = 0; sq < 64; sq++) {
		int fileNum = sq % 8;
		int rankNum = sq / 8;
		if (pos->pieceList[sq] == NO_PIECE) {
			continue;
		}

		else if (pos->pieceList[sq] == WP) {
			/*if (endgame) {
				score += pawnVal + psqt::PawnTableEg[sq];
			}
			else {
				score += pawnVal + psqt::PawnTableMg[sq];
			}*/
			mgScore += pawnVal + psqt::PawnTableMg[sq];
			egScore += pawnVal + psqt::PawnTableEg[sq];
			
			if ((whitePassedPawnMasks[sq] & pos->position[BP]) == 0) { // It is a passed pawn
				mgScore += passedPawnValue[rankNum];
				egScore += passedPawnValue[rankNum];
				// Reward having a rook behind a passed pawn
				mgScore += ((pos->position[WR] & whiteRookSupport[sq]) != 0) ? 25 : 0;
				egScore += ((pos->position[WR] & whiteRookSupport[sq]) != 0) ? 40 : 0; // Rook behind passed pawn is more important in endgame.
				// Perhaps change this static value to something like passedPawnValue[rankNum] / 2
			}

			// Penalize isolated and doubled pawns.
			mgScore -= ((isolatedPawnMasks[fileNum] & (pos->position[WP] ^ ((uint64_t)1 << sq))) == 0) ? 28 : 0;
			mgScore -= (countBits(pos->position[WP] & FileMasks8[fileNum]) >= 2) ? 20 : 0;

			egScore -= ((isolatedPawnMasks[fileNum] & (pos->position[WP] ^ ((uint64_t)1 << sq))) == 0) ? 28 : 0;
			egScore -= (countBits(pos->position[WP] & FileMasks8[fileNum]) >= 2) ? 20 : 0;
		}
		else if (pos->pieceList[sq] == WN) {
			phase += 1;

			mgScore += knightVal + psqt::KnightTable[sq];
			egScore += knightVal + psqt::KnightTable[sq];

			mgScore -= (8 - WpawnCnt) * 5;
			egScore -= (8 - WpawnCnt) * 7;
		}

		else if (pos->pieceList[sq] == WB) {
			phase += 1;

			mgScore += bishopVal + psqt::BishopTable[sq];
			egScore += bishopVal + psqt::BishopTable[sq];
		}

		else if (pos->pieceList[sq] == WR) {
			phase += 2;

			mgScore += rookVal + psqt::RookTable[sq];
			egScore += rookVal + psqt::RookTable[sq];

			mgScore += (8 - WpawnCnt) * 5; // Add value depending on pawn amount
			egScore += (8 - WpawnCnt) * 7;

			// Reward rooks on open files
			mgScore += ((((pos->position[WP] | pos->position[BP]) & FileMasks8[fileNum]) == 0)
			    && (pos->position[WR] & FileMasks8[fileNum]) != 0) ? 35 : 0;

			egScore += ((((pos->position[WP] | pos->position[BP]) & FileMasks8[fileNum]) == 0)
				&& (pos->position[WR] & FileMasks8[fileNum]) != 0) ? 20 : 0;
		}

		else if (pos->pieceList[sq] == WQ) {
			phase += 4;

			mgScore += queenVal + psqt::QueenTable[sq];
			egScore += queenVal + psqt::QueenTable[sq];
		}

		else if (pos->pieceList[sq] == WK) {
			/*if (endgame) {
				score += kingVal + psqt::KingTableEg[sq];
			}
			else {
				score += kingVal + psqt::KingTableMg[sq];
			}*/
			mgScore += kingVal + psqt::KingTableMg[sq];
			egScore += kingVal + psqt::KingTableEg[sq];
		}


		else if (pos->pieceList[sq] == BP) {
			/*if (endgame) {
				score -= (pawnVal + psqt::PawnTableEg[psqt::Mirror64[sq]]);
			}
			else {
				score -= (pawnVal + psqt::PawnTableMg[psqt::Mirror64[sq]]);
			}*/

			mgScore -= (pawnVal + psqt::PawnTableMg[psqt::Mirror64[sq]]);
			egScore -= (pawnVal + psqt::PawnTableEg[psqt::Mirror64[sq]]);

			if ((blackPassedPawnMasks[sq] & pos->position[WP]) == 0) { // The pawn is a passed pawn
				mgScore -= passedPawnValue[mirrorRankNum[rankNum]];
				egScore -= passedPawnValue[mirrorRankNum[rankNum]];

				// Reward having a rook supporting a passed pawn
				mgScore -= ((pos->position[BR] & blackRookSupport[sq]) != 0) ? 25 : 0;
				egScore -= ((pos->position[BR] & blackRookSupport[sq]) != 0) ? 40 : 0;
			}


			// Penalize isolated and doubled pawns
			mgScore += ((isolatedPawnMasks[fileNum] & (pos->position[BP] ^ ((uint64_t)1 << sq))) == 0) ? 28 : 0;
			mgScore += (countBits(pos->position[BP] & FileMasks8[fileNum]) >= 2) ? 20 : 0;

			egScore += ((isolatedPawnMasks[fileNum] & (pos->position[BP] ^ ((uint64_t)1 << sq))) == 0) ? 28 : 0;
			egScore += (countBits(pos->position[BP] & FileMasks8[fileNum]) >= 2) ? 20 : 0;
		}

		else if (pos->pieceList[sq] == BN) {
			phase += 1;
		
			mgScore -= (knightVal + psqt::KnightTable[psqt::Mirror64[sq]]);
			egScore -= (knightVal + psqt::KnightTable[psqt::Mirror64[sq]]);

			mgScore += (8 - BpawnCnt) * 5;
			egScore += (8 - BpawnCnt) * 7;
		}

		else if (pos->pieceList[sq] == BB) {
			phase += 1;

			mgScore -= (bishopVal + psqt::BishopTable[psqt::Mirror64[sq]]);
			egScore -= (bishopVal + psqt::BishopTable[psqt::Mirror64[sq]]);
		}

		else if (pos->pieceList[sq] == BR) {
			phase += 2;
			
			mgScore -= (rookVal + psqt::RookTable[psqt::Mirror64[sq]]);
			egScore -= (rookVal + psqt::RookTable[psqt::Mirror64[sq]]);

			mgScore -= (8 - (BpawnCnt + WpawnCnt)) * 5; // Add value depending on pawn amount.
			egScore -= (8 - (BpawnCnt + WpawnCnt)) * 7;

			mgScore -= ((((pos->position[WP] | pos->position[BP]) & FileMasks8[fileNum]) == 0) &&
			    ((pos->position[BR] & FileMasks8[fileNum]) != 0)) ? 35 : 0;

			egScore -= ((((pos->position[WP] | pos->position[BP]) & FileMasks8[fileNum]) == 0) &&
				((pos->position[BR] & FileMasks8[fileNum]) != 0)) ? 20 : 0;
		}

		else if (pos->pieceList[sq] == BQ) {
			phase += 4;

			mgScore -= (queenVal + psqt::QueenTable[psqt::Mirror64[sq]]);
			egScore -= (queenVal + psqt::QueenTable[psqt::Mirror64[sq]]);
		}

		else if (pos->pieceList[sq] == BK) {
			/*if (endgame) {
				score -= (kingVal + psqt::KingTableEg[psqt::Mirror64[sq]]);
			}
			else {
				score -= (kingVal + psqt::KingTableMg[psqt::Mirror64[sq]]);
			}*/
			mgScore -= (kingVal + psqt::KingTableMg[psqt::Mirror64[sq]]);
			egScore -= (kingVal + psqt::KingTableEg[psqt::Mirror64[sq]]);
		}
	}

	/*
	TAPERED EVAL
	*/
	if (phase > 24) { phase = 24; }
	mgWeight = phase;
	egWeight = 24 - phase;

	score = ((mgScore * mgWeight) + (egScore * egWeight)) / 24;

	/*
	END OF TAPERED EVAL
	*/
	
	// Add or subtract 18 from score depending on who is to move.
	score += (pos->whitesMove == WHITE) ? 18 : -18;
	

	// Penalize having all 8 pawns
	score -= (WpawnCnt == 8) ? 10 : 0;
	score += (BpawnCnt == 8) ? 10 : 0;
	
	// Give bonus for having bishop pair
	score += (countBits(pos->position[WB]) >= 2) ? 50 : 0;
	score -= (countBits(pos->position[BB]) >= 2) ? 50 : 0;

	// Penalize placing bishops in front of the E and D pawns if they are on the second rank.
	if (((pos->position[WP] & RankMasks8[RANK_2]) & FileMasks8[FILE_E]) != 0 && ((pos->position[WB] >> 20) & 1) == 1) { score -= 20; }
	if (((pos->position[WP] & RankMasks8[RANK_2]) & FileMasks8[FILE_D]) != 0 && ((pos->position[WB] >> 19) & 1) == 1) { score -= 20; }

	if (((pos->position[BP] & RankMasks8[RANK_7]) & FileMasks8[FILE_E]) != 0 && ((pos->position[BB] >> 44) & 1) == 1) { score += 20; }
	if (((pos->position[BP] & RankMasks8[RANK_7]) & FileMasks8[FILE_D]) != 0 && ((pos->position[BB] >> 43) & 1) == 1) { score += 20; }

	// Give bonus for having two rooks on the seventh or second rank.
	// The piece-square tables already give bonuses for having one rook there, so this is just to make sure Copper knows that two rooks are even better.
	score += (countBits(pos->position[WR] & RankMasks8[RANK_7]) > 1) ? 30 : 0;
	score -= (countBits(pos->position[BR] & RankMasks8[RANK_2]) > 1) ? 30 : 0;

	// Penalize being in check
	if (pos->inCheck) {
		if (pos->whitesMove == WHITE) {
			score -= 20;
		}
		else {
			score += 20;
		}
	}

	pos->evaluationCache->storeEvaluation(pos, score);

	return score;
}


int eval::getMaterial(const S_BOARD* pos, bool side) {
	int material = 0;
	if (side == WHITE) {
		for (int pce = 0; pce < 6; pce++) {
			material += (pieceVal[pce]) * countBits(pos->position[pce]);
		}
	}
	else {
		for (int pce = 6; pce < 12; pce++) {
			material += ((pieceVal[pce]) * countBits(pos->position[pce]));
		}
	}
	return material;
}