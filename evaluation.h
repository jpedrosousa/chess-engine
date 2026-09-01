#pragma once
#include "board.h"
#include <string>

class Evaluation {
public:
    static int evaluate(const Board& board);

    static bool loadParams(const std::string& path = "tuned_params.txt");

private:
    static int pieceValue(PieceType t);
    static int pstValue(PieceType t, Color c, int square);
};