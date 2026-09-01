#include "Zobrist.h"
#include <array>
#include <random>

namespace {

std::array<std::array<std::array<uint64_t, 64>, 7>, 2> pieceKeys;
uint64_t sideToMoveKey;
std::array<uint64_t, 16> castlingKeys;
std::array<uint64_t, 8> enPassantFileKeys;

bool initialized = false;

void init() {

    std::mt19937_64 rng(0x9E3779B97F4A7C15ULL);

    for (auto& colorArr : pieceKeys)
        for (auto& typeArr : colorArr)
            for (auto& key : typeArr)
                key = rng();

    sideToMoveKey = rng();
    for (auto& key : castlingKeys) key = rng();
    for (auto& key : enPassantFileKeys) key = rng();

    initialized = true;
}

} 

uint64_t Zobrist::computeHash(const Board& board) {
    if (!initialized) init();

    uint64_t hash = 0;

    for (int sq = 0; sq < 64; sq++) {
        const Piece& p = board.at(sq);
        if (p.isEmpty()) continue;
        hash ^= pieceKeys[p.color][p.type][sq];
    }

    if (board.sideToMove == BLACK) hash ^= sideToMoveKey;

    int castlingIdx = (board.whiteCanCastleKingside  ? 1 : 0)
                     | (board.whiteCanCastleQueenside ? 2 : 0)
                     | (board.blackCanCastleKingside  ? 4 : 0)
                     | (board.blackCanCastleQueenside ? 8 : 0);
    hash ^= castlingKeys[castlingIdx];

    if (board.enPassantSquare != -1) {
        int file = board.enPassantSquare % 8;
        hash ^= enPassantFileKeys[file];
    }

    return hash;
}