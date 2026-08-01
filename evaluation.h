#pragma once
#include "board.h"

class Evaluation {
public:
    static int evaluate(const Board& board);

private:
    static int pieceValue(PieceType t);
    static int pstValue(PieceType t, Color c, int square);
};