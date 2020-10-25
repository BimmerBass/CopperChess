
# Copper Chess engine
![Icon](https://github.com/BimmerBass/CopperChess/blob/master/Copper/icon.ico)

A (semi)[1] UCI-compliant chess engine rated at least 2100, written in C++11 by Niels Abildskov (BimmerBass).

At the moment Copper is a command-line engine only, and can only be used graphically with a chess GUI such as Arena og Lucas Chess.

Due to issues with the UCI-protocol, Copper unfortunately only runs on windows at the moment.

#### Strength
The engine still has bugs (described below) and weaknesses, and have therefore not been tested thoroughly yet. Despite of this, it easily beats Stockfish rated 2000 on lichess.org.


#### Special thanks to
- The [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page) which has been used extensively throughout the creation of Copper Chess engine.
- BlueFeverSoft, the creator of the [Vice Chess Engine](https://github.com/bluefeversoft/Vice_Chess_Engine). Copper uses a lot of his implementation and has directly copied some of the code snippets.


#### Implementation details
- Bitboards for board representation.
    - Ray-lookup tables for sliding pieces.
    - Legal-move generation function[2].
    - **Perft @ depth 5 speed is around: 1 second.** (from starting position)
- Relatively simple static evaluation function
    - Has the following piece-values in centipawns:
        - Pawn: 100
        - Knight: 310
        - Bishop: 350
        - Rook: 520
        - Queen: 900
        - King: 20000
        - These values can be tweaked to adjust how bold the engine's playstyle is.
    - Piece-square tables. For the piece types where it is relevant, there are tables for both the middlegame and endgame.
    - Pawn structure.
        - Having all eight pawns is slightly penalized as they tend to clutter the board.
        - Isolated and doubled pawns are penalized.
        - Passed pawns are rewarded.
    - Reward for the side to move (18 cp)
    - Having the bishop pair is rewarded.
    - Rooks and knights recieve bonuses and penalties respectively, depending on the number of pawns present on the board.
    - If the e- or d-pawns are still on the second rank, bishops are penalized for occupying e3 and d3 respectively.
    - Being in check is penalized.
    - Having rooks behind passed pawns are rewarded.
    - Having two rooks on the seventh rank as white or on the second as black are also rewarded.
- **Search function**: The search includes the following methods and tables
    - Iterative deepening with Alpha-Beta Pruning.
    - Aspiration windows for narrowed search.
    - History heuristics
    - Killer moves
    - Quiescence search
        - Delta pruning
    - 2GB Transposition table. (Will be reduced soon)
    - MvvLva (Most-valuable-victim Least-valuable-attacker.)
    - Late move reductions
    - Futility pruning
    - Null move pruning
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

##### Notes
1. The engine is only semi-UCI compliant since the protocol hasn't been completely implemented, and it has some bugs. (works fine most of the time)
2. Although it is inefficient to check all pseudo-legal moves generated, i have decided to keep this implementation as it is more intuitive and the algorithm for doing this is still quite efficient. It works by first seeing if the side to move is in check. If this is true, it tries all moves and see if they still leave the side in check. If it isn't, it creates a bitmask for the king square that has all the squares a queen would be able to move to, and then it checks the moves for pieces that start on these squares. For king moves it checks to see if the destination square is attacked, and for en-passant it just makes the move and sees if the side to move is in check.
3. Self-play training will be accomplished by creating around 15.000 positions from self-play (first six moves will be randomized so the network won't overfit). Then, a batch of around 2.000 positions will be selected randomly, their evaluations will be compared to the actual outcomes from the games, and the error function will be computed by taking the squared sum of there differences. Backpropagation will be used to adjust the network.