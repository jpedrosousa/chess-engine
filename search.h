#pragma once
#include "board.h"
#include "move.h"
#include "tt.h"
#include <array>
#include <vector>

struct SearchResult {
    Move bestMove;
    int score = 0;
    int depthReached = 0; 
};

class Search {
public:
    static SearchResult findBestMove(const Board& board, int maxDepth);

private:
    struct SearchState {
        TranspositionTable tt;
        std::vector<std::array<Move, 2>> killers; // killer moves por ply
        int history[2][64][64] = {};              // [cor][from][to]

        explicit SearchState(size_t ttSizeMB) : tt(ttSizeMB) {}
    };

    static int negamax(const Board& board, int depth, int alpha, int beta, int ply,
                        SearchState& st, bool allowNull);
    static int quiescence(const Board& board, int alpha, int beta, int ply);
};