#pragma once
#include "board.h"
#include "move.h"

struct SearchResult {
    Move bestMove;
    int score = 0;
};

class Search {
public:
    
    static SearchResult findBestMove(const Board& board, int depth);

private:
   
    static int negamax(const Board& board, int depth, int alpha, int beta, int ply);
};