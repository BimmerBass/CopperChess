#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
//#undef NDEBUG
#include <assert.h>





#if (defined(_MSC_VER) || defined(__INTEL_COMPILER))
#include <xmmintrin.h> // Used for _mm_prefetch
#include <nmmintrin.h> // Used for countBits
#endif


// The verbose option is for disabling search output during SPSA tuning.
//#define COPPER_VERBOSE 1


#define FROMSQ(m) (((m) >> (4)) & (63))
#define TOSQ(m) ((m) >> (10))
#define PROMTO(m) (((m) >> (2)) & (3))
#define SPECIAL(m) ((m) & (3))

// The below conversions are used to calculate the size of the various hash tables.
#define KB(x) (x << 10)
#define MB(x) (x << 20)
#define GB(x) (x << 30)


// Here we define the minimum and maximum allowable hash size in megabytes
#define MIN_HASH 1
#define MAX_HASH 1000


#define MAXPOSITIONMOVES 256  // Maximum amount of expected moves for a position (more than enough)
#define MAXGAMEMOVES 512 // Wayy more than enough, but just to be on the safe side...
#define NOMOVE 0
#define MAXDEPTH 64

#define LAZY_EVAL false // Lazy evaluation, used in evaluation.h and evaluation.cpp

#define INF 30000
#define MATE (INF - MAXDEPTH)

#define VALUE_NONE (INF + 2)

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
// The passed pawns masks can actually be used by the other side to see if their pawn is a backwards pawn.
extern BitBoard whitePassedPawnMasks[64];
extern BitBoard blackPassedPawnMasks[64];
extern BitBoard isolatedPawnMasks[8]; // One for each file.

// Bitmasks for lines behind passed pawns
extern BitBoard whiteRookSupport[64];
extern BitBoard blackRookSupport[64];

// Bitmasks for white and black outposts
extern BitBoard whiteOutpostMasks[64];
extern BitBoard blackOutpostMasks[64];

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

// Array containing reductions depending on amount of moves tried in position. Initialized using Stockfish's implementation
extern int Reductions[MAXPOSITIONMOVES];

// Futility pruning margin
const int fMargin[4] = { 0, 200, 300, 500 };


constexpr uint64_t KING_SIDE = 17361641481138401520;
constexpr uint64_t QUEEN_SIDE = 1085102592571150095;
constexpr uint64_t DARK_SQUARES = 12273903644374837845;


const uint64_t EMPTY = std::stoull("0000000000000000000000000000000000000000000000000000000000000000", nullptr, 2);
const uint64_t UNIVERSE = std::stoull("1111111111111111111111111111111111111111111111111111111111111111", nullptr, 2);

constexpr uint64_t RankMasks8[8] =/*from rank1 to rank8*/
{
	255, 65280, 16711680, 4278190080, 1095216660480, 280375465082880, 71776119061217280, 18374686479671623680
};
constexpr uint64_t FileMasks8[8] =/*from fileA to FileH*/
{
	0x101010101010101L, 0x202020202020202L, 0x404040404040404L, 0x808080808080808L,
	0x1010101010101010L, 0x2020202020202020L, 0x4040404040404040L, 0x8080808080808080L
};

// Indexed by 7 + rank - file
// The diagonals start at the h1-h1 and end in the a8-a8
constexpr uint64_t diagonalMasks[15] = { 128, 32832, 8405024, 2151686160, 550831656968, 141012904183812, 36099303471055874,
	9241421688590303745, 4620710844295151872, 2310355422147575808, 1155177711073755136, 577588855528488960,
	288794425616760832, 144396663052566528, 72057594037927936 };

// Indexed by: rank + file
// Anti-diagonals go from a1-a1 to h8-h8
constexpr uint64_t antidiagonalMasks[15] = { 1, 258, 66052, 16909320, 4328785936, 1108169199648, 283691315109952,
	72624976668147840, 145249953336295424, 290499906672525312, 580999813328273408,
	1161999622361579520, 2323998145211531264, 4647714815446351872, 9223372036854775808 };

// Constants for countBits function
constexpr uint64_t m1 = 0x5555555555555555;
constexpr uint64_t m2 = 0x3333333333333333;
constexpr uint64_t m4 = 0x0f0f0f0f0f0f0f0f;
constexpr uint64_t h01 = 0x0101010101010101;

// Constants for bitScanForward and bitScanReverse.
constexpr int index64[64] = {
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
constexpr int BitTable[64] = {
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

enum TT_FLAG { EXACT, LOWER, UPPER, NO_FLAG };

struct S_TTENTRY{
	uint64_t posKey;
	int move = NOMOVE;
	int score = 0;

	TT_FLAG flag = NO_FLAG;

	int depth = 0; // Depth the position has been searched to.
};

// Transposition table structure
struct S_TABLE{
	S_TTENTRY *tableEntry = nullptr;
	int numEntries = 0;
	
	S_TABLE(uint64_t size, bool gigabytes = false);

	void resize(uint64_t mb_size);

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
	void storeEvaluation(const S_BOARD* pos, int& score);

	void clearCache();

	void prefetch_cache(const S_BOARD* pos);
private:

	S_EVALENTRY* entry = nullptr;
	int numEntries;
};


// Entry to the pawn hash table.
struct PawnHashEntry {
	uint64_t pawnKey;
	int eval;
};

// The pawn hash table.
class PawnHashTable {
public:
	PawnHashTable(uint64_t size);

	~PawnHashTable();

	bool probe_pawn_hash(const S_BOARD* pos, int* ev);

	void store_pawn_eval(const S_BOARD* pos, int* ev);

	void clear_hash();

	void prefetch_cache(const S_BOARD* pos);

private:
	PawnHashEntry* entries = nullptr;

	int numEntries = 0;
};


// Undo-move structure. Contains information that can't be implied directly from a current position.
struct S_UNDO {
	BitBoard bitboards[12] = { 0 };
	//int pieces[64] = { 0 };
	S_SIDE side = WHITE;
	bool inCheck = false;
	
	int castlingPerms = 0;
	int enPassantSq = NO_SQ;
	int fiftyMove = 0;

	bool has_castled[2] = {};

	uint64_t key = 0;
	uint64_t pawnHash = 0;
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

	uint64_t posKey = 0;
	uint64_t pawnKey = 0; // Used for indexing the pawn hash table.

	// Special moves
	int castlePerms = 0;
	bool has_castled[2] = {}; // First index is for white, second for black.

	int enPassantSquare = NO_SQ;

	// Misc
	int kingPos[2] = {0};
	BitBoard EMPTY_SQUARES = 0;
	BitBoard WHITE_PIECES = 0;
	BitBoard BLACK_PIECES = 0;
	int fiftyMove = 0;
	
	// Create a transposition table with default size of 200MB.
	S_TABLE *transpositionTable = new S_TABLE(32);
	S_EVALCACHE* evaluationCache = new S_EVALCACHE(50); // Allocate 50MB for static evaluations.

	/*
	PAWN HASH TABLES
		- Since we have separate evaluations for pawn structures in endgame and middlegame, we need two different pawn hash tables.
		They will have a size of 2MB each.
	*/
	PawnHashTable* pawn_table_mg = new PawnHashTable(1);
	PawnHashTable* pawn_table_eg = new PawnHashTable(1);

	bool is_checkmate = false;
	bool is_stalemate = false;
	
	// Searching
	int pvArray[MAXDEPTH] = {}; // Array of principal variation up to max depth.

	// In the ruy lopez closed variation, killer moves provide around 25%-30% fewer nodes searched.
	int killerMoves[MAXDEPTH][2] = { {} }; // We'll store two killer moves per ply.

	int historyHeuristic[12][64] = { {} }; // historyHeuristic[pieceMoved][toSq]

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


// All the options that the GUI can change. This includes the use of opening books and the tt size.
struct S_OPTIONS {
	bool use_book = true;
	int tt_size = 200; // MB
};

extern S_OPTIONS* engineOptions;


/*
INLINE FUNCTIONS
*/

// Not really random, but useful for generation of hashkeys
#define RAND_64     ((BitBoard)std::rand() | \
					(BitBoard)std::rand() << 15 | \
					(BitBoard)std::rand() << 30 | \
					(BitBoard)std::rand() << 45 | \
					((BitBoard)std::rand() & 0xf) << 60 )

// Used for debugging bitboards in copperMain.cpp
inline void printBitboard(uint64_t bb) {
	for (int rank = 7; rank >= 0; rank--) {

		for (int file = 0; file < 8; file++) {
			if (((bb >> (8 * rank + file)) & 1) == 1) {
				std::cout << "X";
			}
			else {
				std::cout << "-";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n\n";
}


// Used in evaluation.cpp
inline bool squaresOnSameDiagonalOrAntiDiagonal(int sq1, int sq2) {
	if (((sq2 / 8) - (sq1 / 8)) == ((sq2 % 8) - (sq1 % 8)) || (((sq2 / 8) - (sq1 / 8)) + ((sq2 % 8) - (sq1 % 8))) == 0) {
		return true;
	}
	return false;
}

// Used in evaluation.cpp
inline int countBits(uint64_t x) { // Count amount of turned on bits in a uint64_t number
#if (defined(__INTEL_COMPILER) || defined(_MSC_VER))
	return _mm_popcnt_u64(x);
#else
	x -= (x >> 1) & m1;
	x = (x & m2) + ((x >> 2) & m2);
	x = (x + (x >> 4)) & m4;
	return (x * h01) >> 56;
#endif
}


// Taken from stockfish. Used in the evaluation function and psqt.h
enum Score :int { SCORE_ZERO };
constexpr Score make_score(int mg, int eg) {
	return Score((int)((unsigned int)eg << 16) + mg);
}

inline int eg_value(Score s) {
	union { uint16_t u; int16_t s; } eg = { uint16_t(unsigned(s + 0x8000) >> 16) };
	return int(eg.s);
}

inline int mg_value(Score s) {
	union { uint16_t u; int16_t s; } mg = { uint16_t(unsigned(s)) };
	return int(mg.s);
}


inline int bitScanReverse(BitBoard bb) { // Find the MS1B
	const BitBoard debruijn64 = 0x03f79d71b4cb0a89;
	assert(bb != 0);
	bb |= bb >> 1;
	bb |= bb >> 2;
	bb |= bb >> 4;
	bb |= bb >> 8;
	bb |= bb >> 16;
	bb |= bb >> 32;
	return index64[(bb * debruijn64) >> 58];
}


inline int bitScanForward(BitBoard bb) { // Find the LS1B
	assert(bb != 0);
	const BitBoard debruijn64 = 0x03f79d71b4cb0a89;
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

// Used to pick a more random book move by seeding the time.
inline uint64_t getRand() {
	std::srand(std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now().time_since_epoch()).count());
	return ((BitBoard)std::rand()
		| (BitBoard)std::rand() << 15
		| (BitBoard)std::rand() << 30
		| (BitBoard)std::rand() << 45
		| ((BitBoard)std::rand() & 0xf) << 60);
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

	void displayBoardState(S_BOARD &board); // print out position from 12 bitboards
	void parseFen(const char* fen, S_BOARD& pos);


	void mirrorBoard(S_BOARD* pos);
	void clearBoard(S_BOARD* pos);
};


// hashkeys.cpp
uint64_t generatePosKey(const S_BOARD* pos);
uint64_t generatePawnHash(const S_BOARD* pos); // Will be used for the future pawn hash table.


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
void initOutpostMasks();
void initReductions();
void initPhaseMaterial();


// makemove.cpp, movegen.cpp and utils.cpp
void run_wac();
void eval_balance();



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

inline bool moveExists(S_BOARD* pos, const int move) {
	S_MOVELIST list;
	MoveGeneration::validMoves(pos, list);

	for (int i = 0; i < list.count; i++) {
		if (list.moves[i].move == move) {
			return true;
		}
	}
	return false;
}



// search.cpp
bool isRepetition(const S_BOARD* pos);

namespace Search{
void pickNextMove(int index, S_MOVELIST* legalMoves);

int alphabeta(S_BOARD* pos, S_SEARCHINFO *info, int depth, int alpha, int beta, bool doNull, bool is_pv);

int searchRoot(S_BOARD* pos, S_SEARCHINFO* info, int depth, int alpha, int beta);

int searchPosition(S_BOARD* pos, S_SEARCHINFO *info);

void clearForSearch(S_BOARD* pos, S_SEARCHINFO *info);

int Quiescence(int alpha, int beta, S_BOARD* pos, S_SEARCHINFO* info);

void CheckUp(S_SEARCHINFO* info);

int search_widen(S_BOARD* pos, S_SEARCHINFO* info, int depth, int estimate);

int reduction(bool improving, int depth, int moveCount);

int futility_margin(int d, bool i);

int lmp_limit(int d, bool i);

int contempt_factor(const S_BOARD* pos);

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
	int probePos(const S_BOARD* pos, int depth, int alpha, int beta, int* move, int* score);

	S_TTENTRY* extract_entry(const S_BOARD* pos, bool& ttHit);

	// Insert a position and move into the table
	void storeEntry(S_BOARD* pos, int move, int depth, TT_FLAG flg, int score);

	// Extract the principal variation and return depth at which it exists.
	int getPvLine(S_BOARD* pos, int depth);

	int probePvMove(const S_BOARD* pos);
}

// uci.cpp
void ParseGo(char* line, S_SEARCHINFO* info, S_BOARD* pos);
void ParsePosition(char* lineIn, S_BOARD* pos);
void ParsePerft(char* line, S_BOARD* pos);
void Uci_Loop();



// misc.cpp
void ReadInput(S_SEARCHINFO* info);
int InputWaiting();

void prefetch(void* addr);


// polybook.cpp
int getBookMove(S_BOARD* pos);
void cleanPolyBook();
void initPolyBook();

