#include "defs.h"

#define NAME "CopperChess3.0"
constexpr int INPUTBUFFER = 400 * 6;

/*
DISCLAIMER:
All of the uci implementation is taken nearly or completely directly from the vice chess engine bu BlueFever software.
*/


void ParseGo(char* line, S_SEARCHINFO* info, S_BOARD* pos) {
	int depth = -1, movestogo = 30, movetime = -1;
	long long time = -1, inc = 0;
	char* ptr = nullptr;
	info->timeset = false;

	if ((ptr = strstr(line, "infinite"))) {
		;
	}

	if ((ptr = strstr(line, "binc")) && pos->whitesMove == BLACK) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "winc")) && pos->whitesMove == WHITE) {
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "wtime")) && pos->whitesMove == WHITE) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "btime")) && pos->whitesMove == BLACK) {
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "movestogo"))) {
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line, "movetime"))) {
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line, "depth"))) {
		depth = atoi(ptr + 6);
	}

	if (movetime != -1) {
		time = movetime;
		movestogo = 1;
	}

	info->starttime = getTimeMs();
	info->depth = depth;

	if (time != -1) {
		info->timeset = true;
		time /= movestogo;
		
		if (time > 50) {
			time -= 50;
		}
		else {
			time = time;
		}
		info->stoptime = info->starttime + time + inc;
	}
	
	if (depth == -1) {
		info->depth = MAXDEPTH;
	}

#if defined(COPPER_VERBOSE)
	std::cout << "time: " << time << " start: " << info->starttime << " stop: "
		<< info->stoptime << " depth: " << info->depth << " timeset: " << info->timeset << "\n";
	Search::searchPosition(pos, info);
#endif
}



void ParsePosition(char* lineIn, S_BOARD* pos) {
	lineIn += 9;
	char* ptrChar = lineIn;

	if (strncmp(lineIn, "startpos", 8) == 0) {
		BoardRep::parseFen(START_FEN, *pos);
	}
	else {
		ptrChar = strstr(lineIn, "fen");
		if (ptrChar == NULL){
			BoardRep::parseFen(START_FEN, *pos);
		}
		else {
			ptrChar += 4;
			BoardRep::parseFen(ptrChar, *pos);
		}
	}

	ptrChar = strstr(lineIn, "moves");
	int move = NOMOVE;

	if (ptrChar != NULL) {
		ptrChar += 6;
		while (*ptrChar) {
			std::string moveStr = "";
			for (int i = 0; i < 5; i++) {
				moveStr += *(ptrChar + i);
			}

			move = parseMove(moveStr, pos);
			if (move == NOMOVE) { break; }
			MoveGeneration::makeMove(*pos, move);
			pos->ply = 0;
			while (*ptrChar && *ptrChar != ' ') { ptrChar++; }
			ptrChar++;
		}
	}

	BoardRep::displayBoardState(*pos);
}


void ParsePerft(char* lineIn, S_BOARD* pos) {
	lineIn += 6;

	char* ptrChar = lineIn;

	if (strncmp(lineIn, "startpos", 8) == 0) {
		BoardRep::parseFen(START_FEN, *pos);
	}
	else {
		ptrChar = strstr(lineIn, "fen");

		if (ptrChar == NULL) {
			BoardRep::parseFen(START_FEN, *pos);
		}
		else {
			ptrChar += 4;
			BoardRep::parseFen(ptrChar, *pos);
		}
	}

	ptrChar = strstr(lineIn, "moves");
	int move = NOMOVE;

	if (ptrChar != NULL) {
		ptrChar += 6;
		while (*ptrChar) {
			std::string moveStr = "";
			for (int i = 0; i < 5; i++) {
				moveStr += *(ptrChar + i);
			}

			move = parseMove(moveStr, pos);
			if (move == NOMOVE) { break; }
			MoveGeneration::makeMove(*pos, move);
			pos->ply = 0;
			while (*ptrChar && *ptrChar != ' ') { ptrChar++; }
			ptrChar++;
		}
	}

	// After having parsed the fen, we will parse the depth at which we need to run perft.
	ptrChar = strstr(lineIn, "depth");

	std::string depth_string = "";

	if (ptrChar != NULL) {

		ptrChar += 6;

		while (*ptrChar) {
			depth_string += *ptrChar;
			ptrChar++;
		}

		int depth = std::stoi(depth_string);

		assert(depth > 0);

		MoveGeneration::perftTest(depth, pos);
	}

}


void Uci_Loop() {

	setvbuf(stdout, NULL, _IOLBF, sizeof(NULL));
	setvbuf(stdin, NULL, _IOLBF, sizeof(NULL));

	S_BOARD* pos = new S_BOARD;
	S_SEARCHINFO* info = new S_SEARCHINFO;

	char line[INPUTBUFFER];

	printf("id name %s\n", NAME);
	printf("id author BimmerBass\n");
	printf("option name Book type check default true\n");
	printf("option name Hash type spin default 200 min %d max %d\n", MIN_HASH, MAX_HASH); // The default hash size is 200MB with minimum 1MB and maximum 1GB
	printf("uciok\n");

	int MB = 200;

	while (true) {
		memset(&line[0], 0, sizeof(line));
		fflush(stdout);
		if (!fgets(line, INPUTBUFFER, stdin)) {
			continue;
		}
		if (line[0] == '\n') {
			continue;
		}

		if (!strncmp(line, "isready", 7)) {
			printf("readyok\n");
			continue;
		}
		else if (!strncmp(line, "position", 8)) {
			ParsePosition(line, pos);
		}
		else if (!strncmp(line, "ucinewgame", 10)) {
			char newLine[] = "position startpos\n";
			ParsePosition(newLine, pos);
		}
		else if (!strncmp(line, "go", 2)) {
			ParseGo(line, info, pos);
		}
		else if (!strncmp(line, "quit", 4)) {
			info->quit = true;
			break;
		}
		else if (!strncmp(line, "uci", 3)) {
			printf("id name %s\n", NAME);
			printf("id author BimmerBass\n");
			printf("uciok\n");
		}
		else if (!strncmp(line, "setoption name Hash value ", 26)) {
#if (defined(_WIN32) || defined(_WIN64))
			sscanf_s(line, "%*s %*s %*s %*s %d", &MB, MB);
#else
			sscanf(line, "%*s %*s %*s %*s %d", &MB);
#endif
			
			// Here we are just making sure that the input doesn't exceed the set limits
			if (MB < MIN_HASH) { MB = MIN_HASH; }
			else if (MB > MAX_HASH) { MB = MAX_HASH; }

			pos->transpositionTable->resize(uint64_t(MB));

		}

		else if (!strncmp(line, "setoption name Book value ", 26)) {
			char* ptrTrue = nullptr;
			ptrTrue = strstr(line, "true");

			if (ptrTrue != nullptr) { // Enable book option.
				engineOptions->use_book = true;
			}
			else {
				engineOptions->use_book = false;
			}

		}


		else if (!strncmp(line, "perft", 5)) {
			ParsePerft(line, pos);
		}

		else if (!strncmp(line, "evaltest", 8)) {
			eval_balance();
		}

		if (info->quit) {
			break;
		}
	}
}






