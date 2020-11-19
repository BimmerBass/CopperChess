
# Copper Chess engine
![Icon](https://github.com/BimmerBass/CopperChess/blob/master/Copper/icon.ico)

A (semi)[1] UCI-compliant chess engine rated at least 2100, written in C++11 by Niels Abildskov (BimmerBass).

At the moment Copper is a command-line engine only, and can only be used graphically with a chess GUI such as Arena og Lucas Chess.

Due to issues with the UCI-protocol, Copper unfortunately only runs on windows at the moment.

#### Strength
The engine still has bugs and weaknesses, and have therefore not been tested thoroughly yet. Despite of this, it easily beats Stockfish rated 2000 on lichess.org.


#### Special thanks to
- The [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Copper Chess engine.
- BlueFeverSoft, the creator of the [Vice Chess Engine](https://github.com/bluefeversoft/Vice_Chess_Engine). Copper uses a lot of his implementation and has directly copied some of the code snippets.


#### Implementation details
- Bitboards for board representation.
    - Ray-lookup tables for sliding pieces.
    - Legal-move generation function[2].
    - **Perft @ depth 5 speed is around: 1 second.** (from starting position)
- Static evaluation function.
    - It is being re-written at the moment.
    - Has the following piece-values in centipawns:
        - Pawn: 100
        - Knight: 320
        - Bishop: 350
        - Rook: 560
        - Queen: 1050
        - King: 20000
        - These values can be tweaked to adjust how bold the engine's playstyle is.
        - I am currently working on giving different piece-values in the endgame as pawns for example become more valuable.
    - Piece-square tables. I am working on adjusting the endgame-specific tables (at the moment, many are the same as middlegame psqt's)
    - King evaluation:
        - Bonus for castling
        - Penalty for being on other ranks than the back rank in middlegame.
        - Bonus for centralization in endgame. Either using Manhattan center distance or just having a hardcoded piece-square table.
    - Queen evaluation:
        - Penalty for early development (only applicable in middlegame). Will perhaps be determined based on amount of pieces on the back rank.
        - Bonus for decreased manhattan distance to opponent king. This bonus will be increased in the endgame.
        - Bonus for being on the same file, rank or diagonal as the enemy king.
    - Rook evaluation:
        - Small bonus for being on the same file (perhaps also rank) as the enemy queen, and bigger bonus for being on the same file as enemy king.
        - Bonus for being doubled.
        - Bonus for being on the seventh rank.
        - Bonus for having to rooks on the seventh rank (also known as pigs on the seventh).
        - Bonus for being on an open or semi open file.
        - Bonus for being on the E and D file in the middlegame.
        - Inversely proportional bonus to the amount of own pawns on the board.
        - Bonus for defending one of our own passed pawns.
        - Extra bonus for being behind an enemy passed pawn.
    - Bishop evaluation:
        - Bonus if we have the bishop pair.
        - Bonus inversely proportional to amount of pawns on the diagonals occupied.
        - Bonus if there are pawns on both sides (queenside and kingside) of the board in the endgame.
        - Bonus for being on the same square color as the enemy king.
        - Bonus for early development.
        - Penalty for being developed such that it blocks either the E2 or D2 pawn.
        - Bonus for being on the same diagonal as the enemy king.
    - Knight evaluation:
        - Bonus for centralization.
        - Penalty each time a pawn of the same color gets removed from the board.
        - Penalty in the endgame if there are pawns on both sides of the board.
        - Bonus for being on an outpost (we can determine this using the passed pawn bitmasks, we just need to remove the middle file). Extra bonus if defended by a pawn.
    - Pawn evaluation (pawn structure evaluation):
        - I will be implementing a pawn hash table as pawn structure evaluation is quite slow, but i have not found an efficient way to do so yet.
        - Bonus if defended by another pawn.
        - Bonus if it is a passed pawn.
        - Penalty for being advanced if it is in front of the castled king.
        - Penalty for being doubled.
        - Penalty if isolated. This should be smaller than the bonus for being passed as it would otherwise deincentivize having passed pawns.
        - Extra bonus for connected passed pawns.
        - Bonus for central advancement in the middlegame.
        - Bonus for being advanced on the edge in the endgame.
    - Small bonus for tempo (being the side to move).
    - A tapered evaluation will be used to transition into a more endgame-specific evaluation as pieces dissapear from the board.
- **Automated tuning**: To increase playing strength, we will implement an algorithm to tune the evaluation - and perhaps also search - parameters by decreasing an error function obtained by self-play games with ultra-short time controls (e.g. 1 + 0.08s).
- **Search function**: The search includes the following methods and tables
    - Iterative deepening with Alpha-Beta Pruning.
    - Aspiration windows for narrowed search.
    - History heuristics
    - Killer moves
    - Quiescence search
        - Delta pruning
        - Bad capture pruning.
    - 200MB Transposition table, with a (soon to be) depth-based replacement strategy. (Will be reduced when a function that measures the percentage of space used in the transposition table is implemented)
    - MvvLva (Most-valuable-victim Least-valuable-attacker.)
    - Late move reductions
    - Futility pruning
    - Razoring.
    - Null move pruning
    - Eval-/Static null move pruning.
    - Mate distance pruning. If we have found a forced checkmate, we don't want to examine longer mate sequences than that one.
    - Principal variation search in the root node.
    - A 50MB evaluation cache that stores previously calculated static evaluations. At the moment, the replacement strategy is replace-all, but i think an age-strategy would be better in the future.

Copper achieves an overall move ordering of around 89-90%.

#### TO-DO's
1. MTD(f) will be tried instead of aspiration windows, and if it has better performance, it'll be implemented.
2. The array-lookup generation of sliding piece attacks will be replaced with magic bitboards for performance gains.
3. Further pruning techniques will be added (especially to quiescence search) to get an average search depth of 15 plies in the middlegame.
4. The evaluation function will be improved and optimized for speed.
5. Chess960 support.
6. Some day, I would like to create a convolutional neural network, and train it with self-play[3] to get a better evaluation function.

##### Notes and bugs
1. The engine is only semi-UCI compliant since the protocol hasn't been completely implemented. It works fine for playing games, but some info to the GUI is missing.
2. Although it is inefficient to check all pseudo-legal moves generated, i have decided to keep this implementation as it is more intuitive and the algorithm for doing this is still quite efficient. It works by first seeing if the side to move is in check. If this is true, it tries all moves and see if they still leave the side in check. If it isn't, it creates a bitmask for the king square that has all the squares a queen would be able to move to, and then it checks the moves for pieces that start on these squares. For king moves it checks to see if the destination square is attacked, and for en-passant it just makes the move and sees if the side to move is in check.
3. Self-play training will be accomplished by creating around 15.000 positions from self-play (first six moves will be randomized so the network won't overfit). Then, a batch of around 2.000 positions will be selected randomly, their evaluations will be compared to the actual outcomes from the games, and the error function will be computed by taking the squared sum of there differences. Backpropagation will be used to adjust the network.