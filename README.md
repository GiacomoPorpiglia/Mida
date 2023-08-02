# Chess Engine
Hey there! This project is a chess engine called "Mida" built entirely in C++. <br />
but first, I want to write an explanation of how a chess engine works for two reasons:
-To keep a record of my work for myself.
-I think a project like this is optimal for an intermidiate/advanced level programmer, and so here you can find some interesting resources if you are interested in doing a similar project.
<br /> <br />
I've been into chess for some years now, and I always wondered how chess engines work, and so I finally took the courage to face this question and build my own engine!

## Representation of the board
You might think that the board in a chess engine is represented as a 2D array, but because of efficiency reasons, this isn't the case. <br />
Instead, chess engines use <b>bitboards</b>, which are 64-bits unsigned integers: this representation is convenient because there are 64 squares on a chess board: therefore, we can represent the occupied squares, attacked squares ecc... by setting to 1 (or "hot") the bits that correspond to an occupied or attacked square, for example. <br />
This implementation is faster because bit-wise operations are very efficient. You can explore further [here](https://www.chessprogramming.org/Bitboards), understanding the full potential of bitboards implementation. 

### What are the main parts of an engine?

There are 3 main parts that constitute an engine: <br />
* <b>Move generation</b> <br />
* <b>Evaluation</b>
* <b>Alpha-Beta pruning</b>

<br /><br />

## Move generation


<br/>
Move genearation consists in <b>generating all the legal moves for a given side and a given position</b>. While this task may seem easy at first, I can assure you will find many obstacles along the way. 

[This is a great article](https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/")  that explains how to generate legal moves efficiently that I strongly recommend.
<br/>

Coding move generation wasn't easy at all for me, and these are the main reasons:

* The first was implementing all the rules such as en passant, especially in some particular cases like when en passant is not allowed due to a king pin.
* The second challenge I faced was optimization: writing a fast move generator is crucial for the engine's speed, and it requires a clever use of bitboards and memory allocation, which for me was a fascinating but also challenging topic.
* The main issue though was, for me, <b>magic bitboards</b>: to put it simple, they are precalcuated bitboard attacks for queens, bishops and rooks (sliding pieces), for every possible position of the piece and of the other pieces on the board (their precalculation is needed to make the move generation faster); the hard part to understand is that, to retrive a magic bitboard, you have to hash the board occupancy, which is the bitboard representing all the occupied squares on the board. Thankfully, I found a great resource on YouTube, which I'll put a link [here](https://www.youtube.com/watch?v=4ohJQ9pCkHI).

My final version of the move generator axplores about 25 million nodes / second on my machine, which is not the best, but I can work with it.

<br />

## Evaluation
The core of a chess engine is the <b>evaluation function</b>. This function, which gets very articulated in engines like Stockfish, has the job of giving a score to the position that estimates how much a given side is better than the other. Generally a positive score means that the side the engine is playing for is better, and a negative one means that the opponent has an advantage.
This function is crucial to the engine reliability, and you'll understand why when talking about Alpha-Beta pruning.
<br />
Intuitively for now, we can understand that if an engine can tell more accurately who has the advantage in a given position, it will be more reliable and will generallt play better finding the best moves, just like happens with human players.
<br />
This function obviously accounts for the material count, by giving a value to each piece type and adding the scores up for each side, but it can have many other parameters: the Stockfish one, for example, is very complicated, and it has been developed with the help of GrandMasters in over a decade.

In my case, in the evaluation function I account for:

* Material count
* Piece-Square tables (more [here](https://www.chessprogramming.org/Piece-Square_Tables))
* Number of attacked squares (mobility)
* Attacks on enemy king ring
* King safety (with [virtual mobility](https://www.chessprogramming.org/King_Safety))
* King pawn shield
* Rook on semi-open or open file
* Bishop on long diagonal
* Number of pawns on squares of the same color of the bishop
* Knight outpost
* Double pawns
* Isolated pawns
* Passed pawns
* Pawn structure
* Threats by pawn push
* Threats by minor pieces/rook
* Hanging pieces

All the correspondent bonuses and penalties are interpolated based on the game phase, which I calculate based on the number of bishops, knights, rooks and queens on the board.
<br />
Also, to help finding checkmates in the endgame, I give a bonus for each unit distance of the king from the corners of the board. This way, if the engine is left with king and rook against king, it will try to push the opponent king to a corner, where it's easier to theckmate.
<br />
Altough this evaluation can seem advanced, it's actually not, for 2 main reasons:

* The parameters I'm accounting are not that many (there's much more to consider in the evaluation of a position)
* The bonuses and penalties are hardcoded, meaning that I decided how much of a bonus to give for each of the parameters above. These parameters would need a lot of testing and tweaking to be more reliable, altough they seem to be doing their job.

<br />

## Alpha-Beta pruning

<br />

To find the best move, a chess engine explores all the possible positions reachable after that move, and does that over and over, going deeper and deeper in the position.
<br />
We know Stockfish can reach a depth of 15 in less than a second, but we also know that, from the starting position, after 15 moves the number of reachable positions are...well...this number: 2,015,099,950,053,364,471,960.
<br />
Obviously, engines don't evaluate every single one of these positions, because it would take centuries: instead, they use an algorithm called Alpha-Beta pruning.
<br />

I will not try to explain the algorithm in much detail. Instead, I'll give a very brief and intuitive explanation, and leave to the reader and its curiosity the rest.
Alpha-Beta pruning is a search-algorithm that decreases the number of nodes to evaluate in the search tree. It works by "pruning" certain branches of the search tree; specifically, it prunes a branch if its root is a move that has proven to be worse than a previously evaluated move.

I'll leave the full explanation to [Sebastian Lague and his awesome video](https://www.youtube.com/watch?v=l-hh51ncgDI), since it really helped me understanding how the algorithm works, and since I think it's much easier to understand it with proper examples, graphics and animations, like the ones provided in the video.

<br />

Obviously, there is MUCH more to pure Alpha-Beta pruning in a chess engine. In fact, my engine also has the following optimizations:

* Move ordering
    * After we generate the moves, we order them, to search first captures, and other types of move, because they are most forcing/forced, and more likely to be the best ones. The sorting of captures is done according to MVV/LVA.
    I also implemented hash move ordering, and it gave the engine a consistent boost in terms of pruning.
* Null move pruning ([detailed explanation here](https://www.chessprogramming.org/Null_Move_Pruning))

* Late move reduction ([detailed explanation here](https://www.chessprogramming.org/Late_Move_Reductions))

* Razoring ([detailed explanation here](https://www.chessprogramming.org/Razoring))

* Zobrist hashing
    * In chess, a position can be reached in different move orders; therefore, we hash the evaluated positions with a method called Zobrist hashing, storing its evaluation, too.
    If we then encounter the same position, we don't have to waste time evaluating it again, because we will just retrieve the evaluation we previously stored. For people who know in detail what I'm talking about, it's an always-replace table.

<br />


## Engine strength

- v1.0: ~2230 ELO in [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/) 
- v1.1: ~2320 ELO in [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/)
- v1.2: +~150 ELO than v1.0, yet to be tested on CCRL.



## How to use
To compile the code, just run the command:
```
g++ *.cpp Board/Board.cpp -O3 -w -o mida_engine.exe
```

The engine is built to work with UCI (Universal Chess Interface), and you can easily find all the commands online.
The most useful are:

* "position startpos" to initialize the engine to the starting position
* "position fen <fen_string>" to load a position from its FEN string
* "go perft <search_depth>" to run a performance test (count how many positions occur at a certain depth starting from a certain position)
* "go depth <search_depth>" to get the best move according to the engine up to a certain depth <search_depth>, starting from a previously loaded position




# v1.1 Updates
The main updates in v1.1 are:

* 10% increment in computed nodes per second.
* New evaluation function (not in its parameters, but much more readable and easily changable).
* Space evaluation and king on open flank.

# v1.2 Updates
This version has its main updates in the search function.
- Reverse futility pruning.
- More aggressive null move pruning.

# v1.2.1 Updates
This was more of a patch. I refactored all the code into proper c++ code.
The only change was a more aggressive late move reduction, reducing by 1 the first 6 moves and the others by depth/3.
The code refactoring made the execuction slightly slower by a ~10%, but I chose to keep this implementation because in my opinion a nicely written code is more important. Also, I will attempt to bring back the original performance, and I hope the new late move reduction can try to compensate for now.

There was also un upgrade in the evaluation function, regarding the evaluatin of an attack, modifying the already existing one inspred by [Loki engine](https://github.com/BimmerBass/Loki)