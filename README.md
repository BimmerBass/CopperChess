
# Copper Chess engine
![Icon](https://github.com/BimmerBass/CopperChess/blob/master/Copper/icon.ico)

A (semi)[1] UCI-compliant chess engine rated at least 2100, written in C++11 by Niels Abildskov (BimmerBass).

At the moment Copper is a command-line engine only, and can only be used graphically with a chess GUI such as Arena og Lucas Chess.

Due to issues with the UCI-protocol, Copper unfortunately only runs on windows at the moment.

#### Strength
The engine still has bugs[4][5][6] (described below) and weaknesses, and have therefore not been tested thoroughly yet. Despite of this, it easily beats Stockfish rated 2000 on lichess.org.


#### Special thanks to
- The [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Copper Chess engine.
- BlueFeverSoft, the creator of the [Vice Chess Engine](https://github.com/bluefeversoft/Vice_Chess_Engine). Copper uses a lot of his implementation and has directly copied some of the code snippets.


#### Implementation details
- Bitboards for board representation.
    - Ray-lookup tables for sliding pieces.
    - Legal-move generation function[2].
    - **Perft @ depth 5 speed is around: 1 second.** (from starting position)
- Static evaluation function
    - It is being re-written at the moment.
    - Has the following piece-values in centipawns:
        - Pawn: 100
        - Knight: 310
        - Bishop: 350
        - Rook: 520
        - Queen: 900
        - King: 20000
        - These values can be tweaked to adjust how bold the engine's playstyle is.
        - I am currently working on giving different piece-values in the endgame as pwns for example become more valuable.
    - Piece-square tables. I am working on adjusting the endgame-specific tables (at the moment, many are the same as middlegame psqt's)
    - King evaluation:
        - Bonus for castling
        - Penalty for being on other ranks than the back rank in middlegame.
        - Bonus for manhattan distance to enemy king in endgame (should increase towards the late endgame)
        - Bonus for centralization in endgame. Either using Manhattan center distance or just having a hardcoded piece-square table.
    - Queen evaluation:
        - Penalty for early development (only applicable in middlegame). Will perhaps be determined based on amount of pieces on the back rank.
        - Bonus for decreased manhattan distance to opponent king. This bonus will be increased in the endgame.
        - Bonus for being on the same file, rank or diagonal as the enemy king.
    - Rook evaluation:
        - Small bonus for being on the same file (perhaps also rank) as the enemy queen, and bigger bonus for being on the same file as enemy king.
        - Bonus for having connected rooks on the back rank in the middlegame.
        - Bonus for being doubled.
        - Bonus for being on the seventh rank.
        - Bonus for having to rooks on the seventh rank (also known as pigs on the seventh).
        - Bonus for being on an open or semi open file. Could be done for white like this: rookVal += 40 - 13.3 * pawnCnt(file)
        - Bonus for being on the E and D file in the middlegame.
        - Inversely proportional bonus to the amount of own pawns on the board.
        - Bonus for defending one of our own passed pawns.
        - Extra bonus for being behind an enemy passed pawn.
    - Bishop evaluation:
        - Bonus if we have the bishop pair.
        - Bonus inversely proportional to amount of pawns on the diagonals occupied.
        - Bonus for being on our side of the board in the middlegame. (e.g. for white: being on the 1st, 2nd, 3rd or 4th rank).
        - Bonus if there are pawns on both sides (queenside and kingside) of the board in the endgame.
        - Bonus for being on the other side of the enemy king (for example, a bishop on B2 is better than one on G2 if black has castled kingside).
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
- **Search function**: The search includes the following methods and tables
    - Iterative deepening with Alpha-Beta Pruning.
    - Aspiration windows for narrowed search.
    - History heuristics
    - Killer moves
    - Quiescence search
        - Delta pruning
    - 1GB Transposition table. (Will be reduced when a function that measures the percentage of space used in the transposition table is implemented)
    - MvvLva (Most-valuable-victim Least-valuable-attacker.)
    - Late move reductions
    - Futility pruning
    - Null move pruning
    - Eval-/Static null move pruning.
    - Mate distance pruning. If we have found a forced checkmate, we don't want to examine longer mate sequences than that one.
    - Principal variation search in the root node.
    - A 500MB evaluation cache that stores previously calculated static evaluations.

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
4. The engine sometimes makes inaccuracies in the endgame, but this will most likely be fixed using a tapered eval and more endgame-specific heuristics. **As stated, the tapered eval has been added, but there is still not enough endgame knowledge.**
5. **This is most likely fixed now, but I will need to perform more tests to be 100% sure.** If the game is really long (usually over 100 plies), Copper doesn't receive the entire UCI-command causing it to evaluate another position and therefore loose by making illegal moves. This will probably be fixed by increasing the input buffer size. **It also had problems with parsing promotion moves sent from the GUI. Since promotions often happen in the endgame, i think this was the cause for the illegal moves.**