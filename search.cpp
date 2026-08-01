#include "search.h"
#include "movegen.h"
#include "evaluation.h"
#include <limits>

namespace {
    constexpr int INF = 2'000'000;
    constexpr int MATE_SCORE = 1'000'000;
}

int Search::negamax(const Board& board, int depth, int alpha, int beta, int ply) {
    std::vector<Move> moves = MoveGenerator::generateLegalMoves(board);

    if (moves.empty()) {
        bool inCheck = MoveGenerator::inCheck(board, board.sideToMove);
        if (inCheck) return -(MATE_SCORE - ply); 
        return 0; 
    }

    if (depth == 0) {
        int eval = Evaluation::evaluate(board); 
        return (board.sideToMove == WHITE) ? eval : -eval;
    }

    int best = -INF;
    for (const Move& m : moves) {
        Board next = board.makeMove(m);
        int score = -negamax(next, depth - 1, -beta, -alpha, ply + 1);
        if (score > best) best = score;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break; 
    }
    return best;
}

SearchResult Search::findBestMove(const Board& board, int depth) {
    SearchResult result;
    result.score = -INF;

    int alpha = -INF, beta = INF;
    for (const Move& m : MoveGenerator::generateLegalMoves(board)) {
        Board next = board.makeMove(m);
        int score = -negamax(next, depth - 1, -beta, -alpha, 1);
        if (score > result.score) {
            result.score = score;
            result.bestMove = m;
        }
        if (score > alpha) alpha = score;
    }
    return result;
}