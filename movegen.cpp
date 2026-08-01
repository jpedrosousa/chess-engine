#include "movegen.h"

namespace {
    inline int fileOf(int sq) { return sq % 8; }
    inline int rankOf(int sq) { return sq / 8; }
    inline bool onBoard(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }
    inline int sqOf(int f, int r) { return r * 8 + f; }

    constexpr int A1 = 0, H1 = 7, A8 = 56, H8 = 63;
    constexpr int E1 = 4, F1 = 5, D1 = 3, B1 = 1, C1 = 2;
    constexpr int E8 = 60, F8 = 61, D8 = 59, B8 = 57, C8 = 58;

    void addPromotions(std::vector<Move>& out, int from, int to, bool capture) {
        for (PieceType promo : {QUEEN, ROOK, BISHOP, KNIGHT}) {
            Move m;
            m.from = from;
            m.to = to;
            m.promotion = promo;
            m.isCapture = capture;
            out.push_back(m);
        }
    }
}

void MoveGenerator::generatePawnMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out) {
    int f = fileOf(sq), r = rankOf(sq);
    int dir = (p.color == WHITE) ? 1 : -1;
    int startRank = (p.color == WHITE) ? 1 : 6;
    int promoRank = (p.color == WHITE) ? 7 : 0;

    int oneF = f, oneR = r + dir;
    if (onBoard(oneF, oneR) && board.at(oneF, oneR).isEmpty()) {
        int to = sqOf(oneF, oneR);
        if (oneR == promoRank) {
            addPromotions(out, sq, to, false);
        } else {
            Move m; m.from = sq; m.to = to;
            out.push_back(m);

           
            int twoR = r + 2 * dir;
            if (r == startRank && board.at(f, twoR).isEmpty()) {
                Move m2; m2.from = sq; m2.to = sqOf(f, twoR); m2.isDoublePawnPush = true;
                out.push_back(m2);
            }
        }
    }

    for (int df : {-1, 1}) {
        int cf = f + df, cr = r + dir;
        if (!onBoard(cf, cr)) continue;
        int to = sqOf(cf, cr);

        if (to == board.enPassantSquare) {
            Move m; m.from = sq; m.to = to; m.isEnPassant = true; m.isCapture = true;
            out.push_back(m);
            continue;
        }

        const Piece& target = board.at(cf, cr);
        if (!target.isEmpty() && target.color != p.color) {
            if (cr == promoRank) {
                addPromotions(out, sq, to, true);
            } else {
                Move m; m.from = sq; m.to = to; m.isCapture = true;
                out.push_back(m);
            }
        }
    }
}

void MoveGenerator::generateKnightMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out) {
    static const int offsets[8][2] = {
        {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
    };
    int f = fileOf(sq), r = rankOf(sq);
    for (auto& off : offsets) {
        int nf = f + off[0], nr = r + off[1];
        if (!onBoard(nf, nr)) continue;
        const Piece& target = board.at(nf, nr);
        if (target.isEmpty() || target.color != p.color) {
            Move m; m.from = sq; m.to = sqOf(nf, nr); m.isCapture = !target.isEmpty();
            out.push_back(m);
        }
    }
}

void MoveGenerator::generateSlidingMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out) {
    static const int rookDirs[4][2]   = {{1,0},{-1,0},{0,1},{0,-1}};
    static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};

    const int (*dirs)[2] = nullptr;
    int nDirs = 0;
    if (p.type == ROOK)   { dirs = rookDirs; nDirs = 4; }
    if (p.type == BISHOP) { dirs = bishopDirs; nDirs = 4; }

    int f = fileOf(sq), r = rankOf(sq);

    auto castDirs = [&](const int dirsArr[][2], int count) {
        for (int i = 0; i < count; i++) {
            int nf = f + dirsArr[i][0], nr = r + dirsArr[i][1];
            while (onBoard(nf, nr)) {
                const Piece& target = board.at(nf, nr);
                if (target.isEmpty()) {
                    Move m; m.from = sq; m.to = sqOf(nf, nr);
                    out.push_back(m);
                } else {
                    if (target.color != p.color) {
                        Move m; m.from = sq; m.to = sqOf(nf, nr); m.isCapture = true;
                        out.push_back(m);
                    }
                    break; 
                }
                nf += dirsArr[i][0];
                nr += dirsArr[i][1];
            }
        }
    };

    if (p.type == QUEEN) {
        castDirs(rookDirs, 4);
        castDirs(bishopDirs, 4);
    } else if (dirs) {
        castDirs(dirs, nDirs);
    }
}

void MoveGenerator::generateKingMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out) {
    static const int offsets[8][2] = {
        {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}
    };
    int f = fileOf(sq), r = rankOf(sq);
    for (auto& off : offsets) {
        int nf = f + off[0], nr = r + off[1];
        if (!onBoard(nf, nr)) continue;
        const Piece& target = board.at(nf, nr);
        if (target.isEmpty() || target.color != p.color) {
            Move m; m.from = sq; m.to = sqOf(nf, nr); m.isCapture = !target.isEmpty();
            out.push_back(m);
        }
    }

    Color opp = (p.color == WHITE) ? BLACK : WHITE;
    if (p.color == WHITE && sq == E1) {
        if (board.whiteCanCastleKingside &&
            board.at(F1).isEmpty() && board.at(6 /*g1*/).isEmpty() &&
            !isSquareAttacked(board, E1, opp) && !isSquareAttacked(board, F1, opp) && !isSquareAttacked(board, 6, opp)) {
            Move m; m.from = E1; m.to = 6; m.isCastleKingside = true;
            out.push_back(m);
        }
        if (board.whiteCanCastleQueenside &&
            board.at(D1).isEmpty() && board.at(C1).isEmpty() && board.at(B1).isEmpty() &&
            !isSquareAttacked(board, E1, opp) && !isSquareAttacked(board, D1, opp) && !isSquareAttacked(board, C1, opp)) {
            Move m; m.from = E1; m.to = C1; m.isCastleQueenside = true;
            out.push_back(m);
        }
    } else if (p.color == BLACK && sq == E8) {
        if (board.blackCanCastleKingside &&
            board.at(F8).isEmpty() && board.at(62 /*g8*/).isEmpty() &&
            !isSquareAttacked(board, E8, opp) && !isSquareAttacked(board, F8, opp) && !isSquareAttacked(board, 62, opp)) {
            Move m; m.from = E8; m.to = 62; m.isCastleKingside = true;
            out.push_back(m);
        }
        if (board.blackCanCastleQueenside &&
            board.at(D8).isEmpty() && board.at(C8).isEmpty() && board.at(B8).isEmpty() &&
            !isSquareAttacked(board, E8, opp) && !isSquareAttacked(board, D8, opp) && !isSquareAttacked(board, C8, opp)) {
            Move m; m.from = E8; m.to = C8; m.isCastleQueenside = true;
            out.push_back(m);
        }
    }
}

std::vector<Move> MoveGenerator::generatePseudoLegalMoves(const Board& board) {
    std::vector<Move> moves;
    moves.reserve(48);
    Color us = board.sideToMove;

    for (int sq = 0; sq < 64; sq++) {
        const Piece& p = board.at(sq);
        if (p.isEmpty() || p.color != us) continue;

        switch (p.type) {
            case PAWN:   generatePawnMoves(board, sq, p, moves); break;
            case KNIGHT: generateKnightMoves(board, sq, p, moves); break;
            case BISHOP:
            case ROOK:
            case QUEEN:  generateSlidingMoves(board, sq, p, moves); break;
            case KING:   generateKingMoves(board, sq, p, moves); break;
            default: break;
        }
    }
    return moves;
}

bool MoveGenerator::isSquareAttacked(const Board& board, int square, Color bySide) {
    int f = fileOf(square), r = rankOf(square);

    int pawnDir = (bySide == WHITE) ? -1 : 1; 
    for (int df : {-1, 1}) {
        int af = f + df, ar = r + pawnDir;
        if (onBoard(af, ar)) {
            const Piece& p = board.at(af, ar);
            if (p.type == PAWN && p.color == bySide) return true;
        }
    }

    static const int knightOffsets[8][2] = {
        {1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}
    };
    for (auto& off : knightOffsets) {
        int af = f + off[0], ar = r + off[1];
        if (onBoard(af, ar)) {
            const Piece& p = board.at(af, ar);
            if (p.type == KNIGHT && p.color == bySide) return true;
        }
    }

    static const int kingOffsets[8][2] = {
        {1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}
    };
    for (auto& off : kingOffsets) {
        int af = f + off[0], ar = r + off[1];
        if (onBoard(af, ar)) {
            const Piece& p = board.at(af, ar);
            if (p.type == KING && p.color == bySide) return true;
        }
    }

    static const int bishopDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto& d : bishopDirs) {
        int af = f + d[0], ar = r + d[1];
        while (onBoard(af, ar)) {
            const Piece& p = board.at(af, ar);
            if (!p.isEmpty()) {
                if (p.color == bySide && (p.type == BISHOP || p.type == QUEEN)) return true;
                break;
            }
            af += d[0]; ar += d[1];
        }
    }
    static const int rookDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto& d : rookDirs) {
        int af = f + d[0], ar = r + d[1];
        while (onBoard(af, ar)) {
            const Piece& p = board.at(af, ar);
            if (!p.isEmpty()) {
                if (p.color == bySide && (p.type == ROOK || p.type == QUEEN)) return true;
                break;
            }
            af += d[0]; ar += d[1];
        }
    }

    return false;
}

bool MoveGenerator::inCheck(const Board& board, Color side) {
    int kingSq = board.kingSquare(side);
    if (kingSq == -1) return false; 
    Color opp = (side == WHITE) ? BLACK : WHITE;
    return isSquareAttacked(board, kingSq, opp);
}

std::vector<Move> MoveGenerator::generateLegalMoves(const Board& board) {
    std::vector<Move> legal;
    Color us = board.sideToMove;
    Color opp = (us == WHITE) ? BLACK : WHITE;

    for (const Move& m : generatePseudoLegalMoves(board)) {

        Board next = board.makeMove(m);
        int kingSq = next.kingSquare(us);
        if (kingSq == -1) continue; 
        if (!isSquareAttacked(next, kingSq, opp)) {
            legal.push_back(m);
        }
    }
    return legal;
}

bool MoveGenerator::isCheckmate(const Board& board, Color side) {
    return inCheck(board, side) && generateLegalMoves(board).empty();
}

bool MoveGenerator::isStalemate(const Board& board, Color side) {
    return !inCheck(board, side) && generateLegalMoves(board).empty();
}