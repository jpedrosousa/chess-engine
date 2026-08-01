#pragma once
#include "board.h"
#include "move.h"
#include <vector>

class MoveGenerator {
public:
    static std::vector<Move> generatePseudoLegalMoves(const Board& board);

    static std::vector<Move> generateLegalMoves(const Board& board);

    static bool isSquareAttacked(const Board& board, int square, Color bySide);

    static bool inCheck(const Board& board, Color side);

    static bool isCheckmate(const Board& board, Color side);

    static bool isStalemate(const Board& board, Color side);

private:
    static void generatePawnMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out);
    static void generateKnightMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out);
    static void generateSlidingMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out);
    static void generateKingMoves(const Board& board, int sq, const Piece& p, std::vector<Move>& out);
};