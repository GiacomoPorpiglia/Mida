
<div align="center">
<img alt="Mida logo" src="./imgs/logo_no_bg.png" style="width:50%;">

<b><i>Strong UCI chess engine written in C++</i></b>

<img alt="GitHub all releases" src="https://img.shields.io/github/downloads/GiacomoPorpiglia/Mida/total?style=for-the-badge">
<a href="./LICENSE"></a><img src="https://img.shields.io/github/license/GiacomoPorpiglia/Mida?style=for-the-badge">
<img src="https://img.shields.io/github/v/release/GiacomoPorpiglia/Mida?style=for-the-badge">
<img src="https://img.shields.io/github/last-commit/GiacomoPorpiglia/Mida?style=for-the-badge">
</div>

# Overview
Mida is a chess engine built entirely in C++. From version 2.0 it uses NNUE for evaluation. </br>


## Engine strength (Elo)


| Version  | [CCRL 40/15](https://computerchess.org.uk/ccrl/4040/) | [CCRL Blitz](https://computerchess.org.uk/ccrl/404/) |
| -------- | ---------- | ---------- |
| 2.3      |    3146    |    3107    |
| 2.2      |    3088    |    3088    |
| 2.1      |    2941    |     /      |
| 2.0      | / (~2600)  | / (~2600)  |
| 1.2.1    |      /     |    2360    |
| 1.2      |      /     |      /     |
| 1.1      |      /     |    2331    |
| 1.0      |      /     |    2233    |



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
More specifically, it was trained for 35 epochs on a small set of Leela and Stockfish data of around 600 million positions. A small part of these were from [DFRC](https://en.wikipedia.org/wiki/Fischer_random_chess) games, but I haven't had the chance to test Mida in this chess variant, so I don't know how it could perform.
<br />

## Search

The search is based on Alpha-beta pruning algorithm, with various pruning and reduction techniques to reduce the number of visited nodes and increase the reached depth:

* Move ordering (MMV/LVA)
* History heuristic
* Zobrist hashing
* Reverse futility pruning
* Null move pruning
* Razoring
* Mate distance pruning
* Late move pruning (LMP)
* Futility pruning
* SEE (static exchange evaluation) used in move ordering as well as pruning for quiet and non-quiet moves
* Late move reduction (LMR)
* Delta pruning
* Singular extensions
* Multicut

<br />

## How to use
To compile the code, just run the commands:

```
 git clone https://github.com/GiacomoPorpiglia/Mida
 cd Mida/src
 make 
./Mida
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
- [rafid-dev](https://github.com/rafid-dev), author of the [Rice engine](https://github.com/rafid-dev/rice), who clarified some doubts about NNUEs.
- [SaccoVinc](https://github.com/SaccoVinc) for creating Mida's logo

Also, thanks to Graham Banks, admin of CCRL, for helping me compile the code properly so that it can execute also on machines without GCC installed, and to all the testers for the useful work they do.

## Donations

Thank you for supporting Mida developement and training through 

[![Paypal](./imgs/paypal.jpg)](https://www.paypal.com/donate/?hosted_button_id=VBC6XDLX4CS62).


# v2.4 Updates

- Bug fix in move generation (en passant pin)


# v2.3 Updates

- Added transposition table reading and writing in quiescence
- Added makefile to compile Mida on all OS
- Many bug fixes



# v2.2 Updates

- Changed History heuristic (now aligned with the implementation of all top engines). This improves the move ordering. Also, history score is now used in LMR to adjust the reduction.

- "Improving cuts": We check if the evaluation has improved compared to the eval from the second last node, and if the score is lower, (meaning the branch is not improving), we can search it with more aggressive prunings (for now only in LMP) because it's probably not a good branch.

- Increased reduction in Null Move Pruning: the reduction is now adjusted also based on the distance of the eval from beta. Also, null move pruning is now applied only if the eval is >= than beta.

- Changes in LMR: the reduction factor (from now will be called R) is now not only the value from the LMR table, but is also adjusted based on the move. In particular:
    - we reduce R by 2 if the move is a killer move.
    - We reduce R by a factor history / 4000, where history is the history score for the move
    - We increase R by 1 if the nod is a non-pv node
    - We increase R by 1 if we are not improving
    - We increase R by 1 if the move is quiet

- Hash table size set to a default value of 64 MB (still have to make it customizable)
- Optimizations in tt entry's size (previouslt 24 bytes, now 16) and added Aging as overwriting condition.

# v2.1 Updates

- Mate distance pruning
- Late move pruning
- Futility pruning
- SEE (static exchange evaluation) for move ordering as well as pruning techniques
- Fixed bug in history moves
- Use of transposition table's evaluation also when we don't return it straight away, meaning we can use the stored eval instead of the NNUE eval of the position. We do this because the tt eval is more accurate, since it comes from a search and not a simple positional evaluation.

# v2.0 Updates

- Introducing NNUE evaluation
- Delta pruning
- Fixed bug in null move pruning

# v1.2 Updates

This version has its main updates in the search function.
- Reverse futility pruning.
- More aggressive null move pruning.

# v1.1 updates

The main updates in v1.1 are:

* 10% increment in computed nodes per second.
* New evaluation function (not in its parameters, but much more readable and easily changable).
* Space evaluation and king on open flank.

