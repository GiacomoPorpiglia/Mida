#ifndef PERFT_H
#define PERFT_H

#include <stdint.h>
#include <iostream>
#include "constants.h"
#include "board_declaration.h"

int perft(int depth, int maxDepth)
{
    int counter = 0;
    if (depth == 0)
        return 1;
    movesList moveList;
    if (board.colorToMove == 1)
        moveList = board.calculateWhiteMoves();
    else
        moveList = board.calculateBlackMoves();

    MOVE move;
    for (int count = 0; count < moveList.count; count++)
    {
        move = moveList.moves[count];
        int tempCounter = 0;
        if (depth == maxDepth)
        {
            cout << coordFromPosition[getSquareFrom(move)] << coordFromPosition[getSquareTo(move)] << " ";
        }

        int oldPieceType = board.allPieces[getSquareFrom(move)];
        int capturedPieceType = board.allPieces[getSquareTo(move)];

        uint16_t oldSpecs = board.boardSpecs;

        // play move
        playMove(move, capturedPieceType);

        int increment = perft(depth - 1, maxDepth);

        unplayMove(move, oldPieceType, oldSpecs, capturedPieceType);

        counter += increment;
        tempCounter += increment;
        if (depth == maxDepth)
            cout << tempCounter << "\n";
    }
    return counter;
}

#endif