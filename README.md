
<div align="center">

<b><i>Strong UCI chess engine written in C++</i></b>

<img alt="GitHub all releases" src="https://img.shields.io/github/downloads/GiacomoPorpiglia/Mida/total?style=for-the-badge">
<a href="./LICENSE"></a><img src="https://img.shields.io/github/license/GiacomoPorpiglia/Mida?style=for-the-badge">
<img src="https://img.shields.io/github/v/release/GiacomoPorpiglia/Mida?style=for-the-badge">
<img src="https://img.shields.io/github/last-commit/GiacomoPorpiglia/Mida?style=for-the-badge">
</div>

# Overview
Mida is a chess engine built entirely in C++. From version 2.0 it uses NNUE for evaluation. </br>


## Engine strength


- v2.1: &nbsp;&nbsp;&nbsp;&nbsp; 2930 ELO on [CCRL 40/15](https://ccrl.chessdom.com/ccrl/4040/) 
- v2.0: &nbsp;&nbsp;&nbsp;&nbsp;   ~2600 ELO (Not tested)
- v1.2.1:&nbsp;&nbsp; 2360 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/) 
- v1.1: &nbsp;&nbsp;&nbsp;&nbsp; 2331 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/)
- v1.0:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;   2233 ELO on [CCRL blitz](https://ccrl.chessdom.com/ccrl/404/) 



## Representation of the board
To represent various board states, most chess engines, including Mida, use <b>bitboards</b>, which are 64-bits unsigned integers: this representation is convenient because there are 64 squares on a chess board: therefore, we can represent the occupied squares, attacked squares ecc... by setting to 1 (or "hot") the bits that correspond to an occupied or attacked square, for example. <br />
This implementation is faster because bit-wise operations are very efficient. You can explore further [here](https://www.chessprogramming.org/Bitboards), understanding the full potential of bitboards implementation. 

## Move generation
Move genearation consists in <b>generating all the legal moves for a given side and a given position</b>. While this task may seem easy at first, I can assure you will find many obstacles along the way.
To do this, I followed [this article](https://peterellisjones.com/posts/generating-legal-chess-moves-efficiently/) that I strongly recommend.
<br/>

## Evaluation
From version 2.0, Mida uses <b>Neural Network</b> evaluation, with [HalfKP](https://www.chessprogramming.org/Stockfish_NNUE#HalfKP) structure. 

The network was very briefly trained, because for now that's not what I want to focus on. Still, it does the job.
<br />

## Search
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

To add:
* "Improving cuts": I don't know if that's the proper name of the technique or if it even has a name. 
Basically, the idea is to see if by going down a branch we are finding better positions (so, if we are improving), and if we are, we want to search that branch deeper. On the contrary, if we are not improving, chances are that's not going to be a good branch, and so we can prune and reduce it more aggressively.

<br />

## How to use
To compile the code, just run the commands:
```
 cd src
 g++ -static *.cpp -O3 -w -o ../mida_engine.exe
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

Finally, thanks to Graham Banks, admin of CCRL, for helping me compile the code properly so that it can execute also on machines without GCC installed.

## Donations

Thank you for supporting Mida developement and training through 

[![Paypal](https://raw.githubusercontent.com/GiacomoPorpiglia/Mida/master/paypal1.jpg)](https://www.paypal.com/donate/?hosted_button_id=VBC6XDLX4CS62).


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
- Use of transposition table's evaluation also when we don't return it straight away, meaning we can use the stored eval instead of the NNUE eval of the position. We do this because the tt eval is more accurate, since it comes from a search and not a simple positional evaluation.