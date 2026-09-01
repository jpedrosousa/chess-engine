#include "search.h"
#include "movegen.h"
#include "evaluation.h"
#include "Zobrist.h"
#include <algorithm>
#include <limits>

namespace {
    constexpr int INF = 2'000'000;
    constexpr int MATE_SCORE = 1'000'000;
    // acima desse valor (em modulo), um score representa um mate forcado.
    // usado para ajustar scores de mate ao entrar/sair da tabela de transposicao,
    // ja que "mate em N" depende do ply em que foi encontrado.
    constexpr int MATE_THRESHOLD = MATE_SCORE - 1000;

    // reducao de profundidade usada no null-move pruning
    constexpr int NULL_MOVE_REDUCTION = 2;
    constexpr int NULL_MOVE_MIN_DEPTH = 3;

    int scoreToTT(int score, int ply) {
        if (score >= MATE_THRESHOLD) return score + ply;
        if (score <= -MATE_THRESHOLD) return score - ply;
        return score;
    }

    int scoreFromTT(int score, int ply) {
        if (score >= MATE_THRESHOLD) return score - ply;
        if (score <= -MATE_THRESHOLD) return score + ply;
        return score;
    }

    int pieceOrderValue(PieceType t) {
        switch (t) {
            case PAWN:   return 100;
            case KNIGHT: return 320;
            case BISHOP: return 330;
            case ROOK:   return 500;
            case QUEEN:  return 900;
            case KING:   return 20000;
            default:     return 0;
        }
    }

    PieceType capturedPieceType(const Board& board, const Move& m) {
        if (m.isEnPassant) return PAWN; // a casa de destino esta vazia no en passant
        return board.at(m.to).type;
    }

    // Ordena so capturas por MVV-LVA. Usado na quiescence, onde so existem capturas.
    void orderCapturesOnly(const Board& board, std::vector<Move>& captures) {
        std::stable_sort(captures.begin(), captures.end(), [&](const Move& a, const Move& b) {
            int scoreA = pieceOrderValue(capturedPieceType(board, a)) * 16 - pieceOrderValue(board.at(a.from).type);
            int scoreB = pieceOrderValue(capturedPieceType(board, b)) * 16 - pieceOrderValue(board.at(b.from).type);
            return scoreA > scoreB;
        });
    }

    long long moveOrderScore(const Board& board, const Move& m, const Move* priorityMove,
                              const std::array<Move, 2>& killers, const int (&historyTable)[64][64]) {
        if (priorityMove && m == *priorityMove) return 10'000'000LL;
        if (m.isCapture) {
            return 1'000'000LL + pieceOrderValue(capturedPieceType(board, m)) * 16
                                - pieceOrderValue(board.at(m.from).type);
        }
        if (m == killers[0]) return 900'000LL;
        if (m == killers[1]) return 800'000LL;
        return historyTable[m.from][m.to];
    }

    void orderMoves(const Board& board, std::vector<Move>& moves, const Move* priorityMove,
                     const std::array<Move, 2>& killers, const int (&historyTable)[64][64]) {
        std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return moveOrderScore(board, a, priorityMove, killers, historyTable)
                 > moveOrderScore(board, b, priorityMove, killers, historyTable);
        });
    }

    bool hasNonPawnMaterial(const Board& board, Color side) {
        return board.countPieces(side, KNIGHT) > 0 || board.countPieces(side, BISHOP) > 0
            || board.countPieces(side, ROOK)   > 0 || board.countPieces(side, QUEEN)  > 0;
    }
}

int Search::quiescence(const Board& board, int alpha, int beta, int ply) {
    int standPat = Evaluation::evaluate(board);
    standPat = (board.sideToMove == WHITE) ? standPat : -standPat;

    if (ply > 64) return standPat;

    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    std::vector<Move> allMoves = MoveGenerator::generateLegalMoves(board);
    std::vector<Move> captures;
    captures.reserve(allMoves.size());
    for (const Move& m : allMoves) {
        if (m.isCapture) captures.push_back(m);
    }
    orderCapturesOnly(board, captures);

    for (const Move& m : captures) {
        Board next = board.makeMove(m);
        int score = -quiescence(next, -beta, -alpha, ply + 1);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

int Search::negamax(const Board& board, int depth, int alpha, int beta, int ply,
                     SearchState& st, bool allowNull) {
    int origAlpha = alpha;
    uint64_t key = Zobrist::computeHash(board);

    Move ttMove; 
    if (depth > 0) {
        int ttScore;
        if (st.tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
            return scoreFromTT(ttScore, ply);
        }
    }

    bool inCheckNow = MoveGenerator::inCheck(board, board.sideToMove);

    std::vector<Move> moves = MoveGenerator::generateLegalMoves(board);
    if (moves.empty()) {
        if (inCheckNow) return -(MATE_SCORE - ply);
        return 0;
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta, ply);
    }


    if (allowNull && depth >= NULL_MOVE_MIN_DEPTH && !inCheckNow
        && hasNonPawnMaterial(board, board.sideToMove)) {
        Board nullBoard = board.makeNullMove();
        int reducedDepth = std::max(depth - 1 - NULL_MOVE_REDUCTION, 0);
        int nullScore = -negamax(nullBoard, reducedDepth, -beta, -beta + 1, ply + 1, st, false);
        if (nullScore >= beta) {
            return beta;
        }
    }

    if (ply >= static_cast<int>(st.killers.size())) st.killers.resize(ply + 8);
    std::array<Move, 2> killersHere = st.killers[ply];

    orderMoves(board, moves, (ttMove.from != -1) ? &ttMove : nullptr,
               killersHere, st.history[board.sideToMove]);

    int best = -INF;
    Move bestMoveHere = moves.front();
    for (const Move& m : moves) {
        Board next = board.makeMove(m);
        int score = -negamax(next, depth - 1, -beta, -alpha, ply + 1, st, true);
        if (score > best) {
            best = score;
            bestMoveHere = m;
        }
        if (best > alpha) alpha = best;
        if (alpha >= beta) {
            if (!m.isCapture) {
                if (!(st.killers[ply][0] == m)) {
                    st.killers[ply][1] = st.killers[ply][0];
                    st.killers[ply][0] = m;
                }
                st.history[board.sideToMove][m.from][m.to] += depth * depth;
            }
            break;
        }
    }

    TTFlag flag;
    if (best <= origAlpha) flag = TTFlag::UPPERBOUND;
    else if (best >= beta) flag = TTFlag::LOWERBOUND;
    else flag = TTFlag::EXACT;
    st.tt.store(key, depth, scoreToTT(best, ply), flag, bestMoveHere);

    return best;
}

SearchResult Search::findBestMove(const Board& board, int maxDepth) {
    SearchState st(32);
    st.killers.resize(maxDepth + 8);

    SearchResult result;
    result.score = -INF;

    Move previousBest;
    bool havePrevious = false;

    for (int depth = 1; depth <= maxDepth; depth++) {
        std::vector<Move> moves = MoveGenerator::generateLegalMoves(board);
        if (moves.empty()) break;

        orderMoves(board, moves, havePrevious ? &previousBest : nullptr,
                   st.killers[0], st.history[board.sideToMove]);

        SearchResult iterResult;
        iterResult.score = -INF;
        iterResult.bestMove = moves.front();
        int alpha = -INF, beta = INF;

        for (const Move& m : moves) {
            Board next = board.makeMove(m);
            int score = -negamax(next, depth - 1, -beta, -alpha, 1, st, true);
            if (score > iterResult.score) {
                iterResult.score = score;
                iterResult.bestMove = m;
            }
            if (score > alpha) alpha = score;
        }

        iterResult.depthReached = depth;
        result = iterResult;
        previousBest = result.bestMove;
        havePrevious = true;
    }

    return result;
}