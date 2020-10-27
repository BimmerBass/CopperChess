#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "code_analysis.h"

//#define CLRBIT(bb, sq) ((bb) &= ClearMask[(sq)])
//#define SETBIT(bb, sq) ((bb) |= SetMask[(sq)])
#define FROMSQ(m) (((m) >> (4)) & (63))
#define TOSQ(m) ((m) >> (10))
#define PROMTO(m) (((m) >> (2)) & (3))
#define SPECIAL(m) ((m) & (3))

// Returns x*1MB for transposition table size.
//#define MB(x) (0x100000 * x)
#define MB(x) (x << 20)
#define GB(x) (x << 30)


#define MAXPOSITIONMOVES 256  // Maximum amount of expected moves for a position (more than enough)
#define MAXGAMEMOVES 512 // Wayy more than enough, but just to be on the safe side...
#define NOMOVE 0
#define MAXDEPTH 64

#define INFINITE 30000
#define MATE (INFINITE - MAXDEPTH)

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef uint64_t BitBoard;


/*
GLOBAL VARIABLES
*/

// Hashkeys for the different pieces and squares.
extern BitBoard pieceKeys[13][64]; // 12 arrays as there are 12 piece types. The 13th is for an empty board.
extern BitBoard sideKey;
extern BitBoard castleKeys[16];

// Bitmasks for isolated and passed pawns
extern BitBoard whitePassedPawnMasks[64];
extern BitBoard blackPassedPawnMasks[64];
extern BitBoard isolatedPawnMasks[8]; // One for each file.

// Bitmasks for lines behind passed pawns
extern BitBoard whiteRookSupport[64];
extern BitBoard blackRookSupport[64];

// Bitmasks for setting and clearing bits.
extern BitBoard SetMask[64];
extern BitBoard ClearMask[64];

// Attack bitboards for sliding pieces
extern BitBoard LineAttackRays[4][64];
extern BitBoard DiagonalAttackRays[4][64];

// Array containing all Manhattan distances from sq1 to sq2. Indexed by: ManhattanDistance[sq1][sq2]
extern int ManhattanDistance[64][64];

// Array of values given to captures depending on the pieces in question (MVV-LVA)
extern int MvvLva[12][12];


// Futility pruning margin
const int fMargin[4] = { 0, 200, 300, 500 };


const BitBoard EMPTY = std::stoull("0000000000000000000000000000000000000000000000000000000000000000", nullptr, 2);
const BitBoard UNIVERSE = std::stoull("1111111111111111111111111111111111111111111111111111111111111111", nullptr, 2);

const BitBoard RankMasks8[8] =/*from rank1 to rank8*/
{
	255, 65280, 16711680, 4278190080, 1095216660480, 280375465082880, 71776119061217280, 18374686479671623680
};
const BitBoard FileMasks8[8] =/*from fileA to FileH*/
{
	0x101010101010101L, 0x202020202020202L, 0x404040404040404L, 0x808080808080808L,
	0x1010101010101010L, 0x2020202020202020L, 0x4040404040404040L, 0x8080808080808080L
};

// Constants for countBits function
const uint64_t m1 = 0x5555555555555555;
const uint64_t m2 = 0x3333333333333333;
const uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
const uint64_t h01 = 0x0101010101010101;

// Constants for bitScanForward and bitScanReverse.
const int index64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

// For the popBit function.
const int BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};


/*
STRUCTURES AND DEFINED TYPES
*/

enum square : int {
	A1 = 0, B1 = 1, C1 = 2, D1 = 3, E1 = 4, F1 = 5, G1 = 6, H1 = 7,
	A2 = 8, B2 = 9, C2 = 10, D2 = 11, E2 = 12, F2 = 13, G2 = 14, H2 = 15,
	A3 = 16, B3 = 17, C3 = 18, D3 = 19, E3 = 20, F3 = 21, G3 = 22, H3 = 23,
	A4 = 24, B4 = 25, C4 = 26, D4 = 27, E4 = 28, F4 = 29, G4 = 30, H4 = 31,
	A5 = 32, B5 = 33, C5 = 34, D5 = 35, E5 = 36, F5 = 37, G5 = 38, H5 = 39,
	A6 = 40, B6 = 41, C6 = 42, D6 = 43, E6 = 44, F6 = 45, G6 = 46, H6 = 47,
	A7 = 48, B7 = 49, C7 = 50, D7 = 51, E7 = 52, F7 = 53, G7 = 54, H7 = 55,
	A8 = 56, B8 = 57, C8 = 58, D8 = 59, E8 = 60, F8 = 61, G8 = 62, H8 = 63, NO_SQ = 65
};


enum rank: int {RANK_1 = 0, RANK_2 = 1, RANK_3 = 2, RANK_4 = 3, RANK_5 = 4, RANK_6 = 5, RANK_7 = 6, RANK_8 = 7, NO_RANK = 8};
enum file: int { FILE_A = 0, FILE_B = 1, FILE_C = 2, FILE_D = 3, FILE_E = 4, FILE_F = 5, FILE_G = 6, FILE_H = 7, NO_FILE = 8};


enum S_SIDE : bool {WHITE = true, BLACK = false};
enum piece :int { WP = 0, WN = 1, WB = 2, WR = 3, WQ = 4, WK = 5, BP = 6, BN = 7, BB = 8, BR = 9, BQ = 10, BK = 11, NO_PIECE = 12 };
enum { WKS = 0, WQS = 1, BKS = 2, BQS = 3 };
enum C_RIGHTS { WKCA = 0, WQCA = 1, BKCA = 2, BQCA = 3 };


// Used in makemove.cpp to parse a promotion move.
const int promoPieces[4][2] = {
	{WN, BN},
	{WB, BB},
	{WR, BR},
	{WQ, BQ}
};

struct S_BOARD;

// Structure to represent a single move. "move" is as the name suggests, the move, and score is the evaluation of the resulting position.
struct S_MOVE {
	int move = 0;
	int score = 0;
};

// List of S_MOVE's. "count" gives the exact amount of moves in the list, such that we don't have to iterate over meaningless data.
struct S_MOVELIST {
	S_MOVE moves[MAXPOSITIONMOVES] = {}; // All moves
	int count = 0; // Amount of moves.
};

enum TT_FLAG { EXACT, LOWER, UPPER };

struct S_TTENTRY{
	BitBoard posKey;
	int move = NOMOVE;
	int score = 0;

	TT_FLAG flag = LOWER;

	int depth = 0; // Depth the position has been searched to.
};

// Transposition table structure
struct S_TABLE{
	S_TTENTRY *tableEntry = nullptr;
	int numEntries = 0;
	
	S_TABLE(uint64_t size, bool gigabytes = false);

	~S_TABLE();
};


// Entry to the evaluation table cache
struct S_EVALENTRY {
	uint64_t posKey;
	int score = 0;
};

// evalcache.cpp
class S_EVALCACHE {
public:
	S_EVALCACHE(uint64_t size, bool gigabytes = false);

	~S_EVALCACHE();

	bool probeCache(const S_BOARD* pos, int& score);
	void storeEvaluation(const S_BOARD* pos, int score);

private:

	S_EVALENTRY* entry = nullptr;
	int numEntries;
};



// Undo-move structure. Contains information that can't be implied directly from a current position.
struct S_UNDO {
	BitBoard bitboards[12] = { 0 };
	int pieces[64] = { 0 };
	S_SIDE side = WHITE;
	bool inCheck = false;
	
	int castlingPerms = 0;
	int enPassantSq = NO_SQ;
	int fiftyMove = 0;

	BitBoard key = 0;
	
};

// Contains all undo-move structures, to make it possible to undo many moves.
struct S_HISTORY {
	// History
	S_UNDO history[MAXGAMEMOVES] = {};
	int moveCount = 0; // In plies. (E.g. 1. e4, e5 would give moveCount = 2).
};

// Primary board structure. Contains bitboards and other useful information. Pass this by pointer or reference whenever possible, as it is more efficient.
struct S_BOARD {
	// General position
	BitBoard position[12] = { 0 };
	int pieceList[64] = { 0 }; // List of integer values representing pieces occupying the square.
	S_SIDE whitesMove;
	bool inCheck = false;

	BitBoard posKey = 0;

	// Special moves
	int castlePerms = 0;
	int enPassantSquare = NO_SQ;

	// Misc
	int kingPos[2] = {0};
	BitBoard EMPTY_SQUARES = 0;
	BitBoard WHITE_PIECES = 0;
	BitBoard BLACK_PIECES = 0;
	int fiftyMove = 0;
	
	// Create a transposition table with default size of 2GB.
	S_TABLE *transpositionTable = new S_TABLE(1, true);
	S_EVALCACHE* evaluationCache = new S_EVALCACHE(500); // Allocate 500MB for static evaluations.
	
	bool is_checkmate = false;
	bool is_stalemate = false;
	
	// Searching
	int pvArray[MAXDEPTH] = {}; // Array of principal variation up to max depth.

	// In the ruy lopez closed variation, killer moves provide around 25%-30% fewer nodes searched.
	int killerMoves[MAXDEPTH][2] = { {} }; // We'll store two killer moves per ply.

	int historyHeuristic[12][64] = { {} }; // historyHeuristic[fromSq][toSq]

	int ply = 0; // Amount of moves deep in search function
	S_HISTORY history;
	
};

struct S_SEARCHINFO{
	long long starttime = 0;
	long long stoptime = 0;
	int depth = MAXDEPTH;
	int depthset = MAXDEPTH;
	bool timeset = false;
	int movestogo = 0;
	bool infinite = -1;

	long nodes = 0;

	bool quit = false;
	bool stopped = false;

	float fh = 0;
	float fhf = 0;
};


/*
INLINE FUNCTIONS
*/

#define RAND_64     ((BitBoard)std::rand() | \
					(BitBoard)std::rand() << 15 | \
					(BitBoard)std::rand() << 30 | \
					(BitBoard)std::rand() << 45 | \
					((BitBoard)std::rand() & 0xf) << 60 )

// Used in evaluation.cpp
inline int countBits(uint64_t x) { // Count amount of turned on bits in a uint64_t number
	x -= (x >> 1) & m1;
	x = (x & m2) + ((x >> 2) & m2);
	x = (x + (x >> 4)) & m4;
	return (x * h01) >> 56;
}


inline int bitScanReverse(BitBoard bb) { // Find the MS1B
	const BitBoard debruijn64 = 0x03f79d71b4cb0a89;
	ASSERT(bb != 0);
	bb |= bb >> 1;
	bb |= bb >> 2;
	bb |= bb >> 4;
	bb |= bb >> 8;
	bb |= bb >> 16;
	bb |= bb >> 32;
	return index64[(bb * debruijn64) >> 58];
}


inline int bitScanForward(BitBoard bb) { // Find the LS1B
	const BitBoard debruijn64 = 0x03f79d71b4cb0a89;
	ASSERT(bb != 0);
	return index64[((bb ^ (bb - 1)) * debruijn64) >> 58];
}


// Provides a 23% speedup in generatePosKey. Might implement in movegen too...
inline int PopBit(BitBoard* bb) {
	BitBoard b = *bb ^ (*bb - 1);
	unsigned int fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
	*bb &= (*bb - 1);
	return BitTable[(fold * 0x783a9b23) >> 26];
}

// Set/Clear a bit in uint64_t bb at index sq.
inline BitBoard SETBIT(BitBoard bb, int sq) {
	return (uint64_t)(bb | SetMask[sq]);
}
inline BitBoard CLRBIT(BitBoard bb, int sq) {
	return (uint64_t)(bb & ClearMask[sq]);
}

// Find the max/min of two numbers. Will be used when tempered evaluation is included.
template <class T> const T& max(const T& a, const T& b) {
	return (a < b) ? b : a;     // or: return comp(a,b)?b:a; for version (2)
}

template <class T> const T& min(const T& a, const T& b) {
	return !(b < a) ? a : b;     // or: return !comp(b,a)?a:b; for version (2)
}

// Generating bitboard from king square in all sliding piece directions.
inline BitBoard genKingRays(int sq) {
	BitBoard kingRays = 0;
	for (int i = 0; i < 4; i++) {
		kingRays |= DiagonalAttackRays[i][sq];
		kingRays |= LineAttackRays[i][sq];
	}
	return kingRays;
}


// Used in bitboards.cpp and board.cpp
inline std::vector<std::string> genSquareList(std::string boardArray[8][8]) {
	std::vector<std::string> squares;
	for (int r = 7; r >= 0; r--) {
		for (int c = 0; c < 8; c++) {
			squares.push_back(boardArray[r][c]);
		}
	}
	return squares;
}

// Used in bitboards.cpp and board.cpp
inline std::string index_to_square(int index) {
	std::string FILES[8] = { "a", "b", "c", "d", "e", "f", "g", "h" };
	int rank = index / 8 + 1; std::string file = FILES[index % 8];
	return file + std::to_string(rank);
}

// Just converts a string to uppercase.
inline std::string to_upper(std::string str) {
	std::string out = "";
	for (int i = 0; i < str.length(); i++) {
		out += toupper(str[i]);
	}
	return out;
}


inline long long getTimeMs() {
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch());
	return ms.count();
}

/*
MOVE REPRESENTATION IS INSPIRED BY STOCKFISH
A move can be stored in a 16 bit unsigned integer.
bit 0..5 -> destination square (0 to 63)
bit 6..11 -> origin square (0 to 63)
bit 12..13 -> promotion piece ((0) knight = 00, (1) bishop = 01, (2) rook = 10, (3) queen = 11)
bit 14..15 -> special move flag ((0) promotion = 00, (1) en-passant = 01, (2) castling = 10, (3) neither = 11)
NOTE: en passant flag is not risen by a move that gives their opponent the ability to do such a move, but a move where it can be done.
*/
inline unsigned int newMove(int toSq, int frSq, int toPiece, int spcFlags) {
	unsigned int out = ((toSq << 10) | (frSq << 4) | (toPiece << 2) | spcFlags);
	return out;
}




/*
FUNCTIONS AND CLASSES
*/

// attack.cpp
bool badCapture(const S_BOARD* pos, int move);
bool sqAttacked(int sq, bool side, const S_BOARD* pos);

namespace attacks {
	void positiveLineAttacks(BitBoard& attackRays, BitBoard OCCUPIED, int direction, int sq);
	void negativeLineAttacks(BitBoard& attackRays, BitBoard OCCUPIED, int direction, int sq);
	void positiveDiagAttacks(BitBoard& attackRays, BitBoard OCCUPIED, int direction, int sq);
	void negativeDiagAttacks(BitBoard& attackRays, BitBoard OCCUPIED, int direction, int sq);
}


// bitboards.cpp and board.cpp
namespace BoardRep {

	S_BOARD* arrayToBitboards(std::string arrayRepresentation[8][8], S_SIDE white, bool castlingPermitions[4], int enPassant = NO_SQ);

	void displayBoardState(S_BOARD board); // print out position from 12 bitboards
	void parseFen(const char* fen, S_BOARD& pos);

	void clearBoard(S_BOARD* pos);
};

// evaluation.cpp
namespace eval {
	int staticEval(const S_BOARD* pos, int depth, int alpha, int beta);

	int getMaterial(const S_BOARD* pos, bool side);

	/*
	The piece values are inspired by the ones found on chessprogramming.org in the sense that they meet the same requirements:

	1. B > N > 3P
	2. B + N > R + P
	3. 2R > Q
	4. R > B + 2P > N + 2P
	*/
	enum mgValues : int {
		pawnValMg = 100,
		knightValMg = 320,
		bishopValMg = 350,
		rookValMg = 560,
		queenValMg = 1050,
		kingValMg = 20000
	};

	enum egValues : int {
		pawnValEg = 200,
		knightValEg = 560,
		bishopValEg = 600,
		rookValEg = 1200,
		queenValEg = 2300,
		kingValEg = 20000
	};

	static int pieceValMg[13] = { pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg,
	pawnValMg, knightValMg, bishopValMg, rookValMg, queenValMg, kingValMg, 0 };
}


// hashkeys.cpp
BitBoard generatePosKey(const S_BOARD* pos);


// init.cpp
void initBishopAttacks();
void initRookAttacks();
void initMvvLva();
void initBitMasks(BitBoard(&set)[64], BitBoard(&clear)[64]);
void initHashKeys();
void initPawnMasks();
void initAll(BitBoard(&set)[64], BitBoard(&clear)[64]);
void initRookSupportMasks();
void initManhattanDistances();


// makemove.cpp, movegen.cpp and utils.cpp
extern inline bool moveExists(S_BOARD* pos, const int move);

extern void makeNullMove(S_BOARD* pos);
extern void undoNullMove(S_BOARD* pos, int ep);

namespace MoveGeneration {

	void makeMove(S_BOARD& board, int move);

	void undoMove(S_BOARD& board);

	// We'll try optimizing this by passing the board as a pointer rather than by value.
	// Avg. perft(3) speed was around 15-20 milliseconds before. This is now 7 milliseconds => at least 50% speedup
	void validMoves(S_BOARD* board, S_MOVELIST& legalMoves);

	void perft(int depth, S_BOARD* board);
	void perftTest(int depth, S_BOARD* board);

	//float alphaBeta(S_BOARD* pos, int depth, float alpha, float beta, bool isMaximizing);

	void pseudoLegalMoves(S_BOARD* boardState, S_MOVELIST& pseudoLegal);

	void pawnMoves(S_BOARD* board, S_MOVELIST& s_moves);

	void knightMoves(S_BOARD* board, S_MOVELIST& s_moves);

	void bishopMoves(S_BOARD* board, S_MOVELIST& s_moves, bool queen = false);

	void rookMoves(S_BOARD* board, S_MOVELIST& s_moves, bool queen = false);

	void queenMoves(S_BOARD* board, S_MOVELIST& s_moves);

	void kingMoves(S_BOARD* board, S_MOVELIST& s_moves);
}


// search.cpp
bool isRepetition(const S_BOARD* pos);

namespace Search{
void pickNextMove(int index, S_MOVELIST* legalMoves);

int alphabeta(S_BOARD* pos, S_SEARCHINFO *info, int depth, int alpha, int beta, bool doNull);

int searchRoot(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta);

void searchPosition(S_BOARD* pos, S_SEARCHINFO *info);

void clearForSearch(S_BOARD* pos, S_SEARCHINFO *info);

int Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info);

void CheckUp(S_SEARCHINFO* info);

int search_widen(S_BOARD* pos, S_SEARCHINFO* info, int depth, int value); // Taken from https://www.chessprogramming.org/CPW-Engine_search

int MTDF(S_BOARD* pos, S_SEARCHINFO* info, int estimate, int depth);
}


// ui.cpp
std::string printMove(int move);
int parseMove(std::string userInput, S_BOARD* board);
void printMovelist(S_BOARD* board);


// transposition.cpp
namespace TT{

	//void initTable();
	
	void clearTable(S_TABLE* table);

	// Check to see if there has already been found a move for the given position, and return if yes.
	int probePos(const S_BOARD* pos, int depth, int alpha, int beta, int *move, int *score);

	// Insert a position and move into the table
	void storeEntry(S_BOARD* pos, int move, int depth, TT_FLAG flg, int score);

	// Extract the principal variation and return depth at which it exists.
	int getPvLine(S_BOARD* pos, int depth);

	int probePvMove(const S_BOARD* pos);
}

// uci.cpp
void ParseGo(char* line, S_SEARCHINFO* info, S_BOARD* pos);
void ParsePosition(char* lineIn, S_BOARD* pos);
void Uci_Loop();


// misc.cpp
void ReadInput(S_SEARCHINFO* info);
int InputWaiting();


// polybook.cpp
int getBookMove(S_BOARD* pos);
void cleanPolyBook();
void initPolyBook();

