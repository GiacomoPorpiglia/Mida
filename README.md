# Overview
Mida is a chess engine built entirely in C++. From version 2.0 it uses NNUE for evaluation. </br>

## Representation of the board
To represent various board states, most chess engines, including Mida, use <b>bitboards</b>, which are 64-bits unsigned integers: this representation is convenient because there are 64 squares on a chess board: therefore, we can represent the occupied squares, attacked squares ecc... by setting to 1 (or "hot") the bits that correspond to an occupied or attacked square, for example. <br />
This implementation is faster because bit-wise operations are very efficient. You can explore further [here](https://www.chessprogramming.org/Bitboards), understanding the full potential of bitboards implementation. 

## Move generation
Move genearation consists in <b>generating all the legal moves for a given side and a given position</b>. While this task may seem easy at first, I can assure you will find many obstacles along the way.
To do this, I followed [this article](https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/") that I strongly recommend.
<br/>

## Evaluation
From version 2.0, Mida uses <b>Neural Network</b> evaluation, with [HalfKP](https://www.chessprogramming.org/Stockfish_NNUE#HalfKP) structure. 

The network was very briefly trained, because for now that's not what I want to focus on. Still, it does the job.
<br />

## Search
<br />
The search is based on Alpha-beta pruning algorithm, with various techniques (like prunings and reductions) to reduce the number of visited nodes:

* Move ordering (MMV/LVA)
* Zobrist hashing
* Reverse futility pruning
* Null move pruning
* Razoring
* Mate distance pruning
* Late move pruning
* Futility pruning
* SEE (static exchange evaluation) used in move ordering as well as pruning for quiet and non-quiet moves
* Late move reduction
* Delta pruning

<br />

## Engine strength

- v1.0: ~2230 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/) 
- v1.1: ~2325 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/)
- v1.2: Not tested on CCRL ~2355 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/) 
- v2.0: Not tested on CCRL
- v2.1 Not tested on CCRL. Should be in the 2800-2900 ELO range



## How to use
To compile the code, just run the command:
```
g++ *.cpp -O3 -w -o mida_engine.exe
```

The engine is built to work with UCI (Universal Chess Interface), and you can easily find all the commands online.
The most useful are:

* "position startpos" to initialize the engine to the starting position
* "position fen <fen_string>" to load a position from its FEN string
* "go perft <search_depth>" to run a performance test (count how many positions occur at a certain depth starting from a certain position)
* "go depth <search_depth>" to get the best move according to the engine up to a certain depth <search_depth>, starting from a previously loaded position

## Special thanks
For this project I used a lot of awesome resources:
- [Chess programming Wiki](https://www.chessprogramming.org/Main_Page)
- [Chess programming Youtube Series by Maksim Korzh](https://www.youtube.com/watch?v=QUNP-UjujBM&list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs)
- [dshawul](https://github.com/dshawul) and his [NNUE-probe library](https://github.com/dshawul/nnue-probe)

I also want to thank [rafid-dev](https://github.com/rafid-dev), author of the [Rice engine](https://github.com/rafid-dev/rice), who clarified some doubts about NNUEs.


# v1.1 updates
The main updates in v1.1 are:

* 10% increment in computed nodes per second.
* New evaluation function (not in its parameters, but much more readable and easily changable).
* Space evaluation and king on open flank.

# v1.2 Updates
This version has its main updates in the search function.
- Reverse futility pruning.
- More aggressive null move pruning.

# v2.0 Updates
- Introducing NNUE evaluation
- Delta pruning
- Fixed bug in null move pruning


# v2.1 Updates
- Mate distance pruning
- Late move pruning
- Futility pruning
- SEE (static exchange evaluation) for move ordering as well as pruning techniques
- Fixed bug in history moves 