#pragma once
#include "board.h"
#include <string>


struct Move {
    int from = -1;
    int to = -1;

    PieceType promotion = EMPTY;

    bool isCapture = false;
    bool isEnPassant = false;
    bool isCastleKingside = false;
    bool isCastleQueenside = false;
    bool isDoublePawnPush = false;

    bool operator==(const Move& o) const {
        return from == o.from && to == o.to && promotion == o.promotion
            && isEnPassant == o.isEnPassant
            && isCastleKingside == o.isCastleKingside
            && isCastleQueenside == o.isCastleQueenside;
    }

    std::string toUCI() const {
        auto sqToStr = [](int sq) {
            char file = 'a' + (sq % 8);
            char rank = '1' + (sq / 8);
            return std::string(1, file) + std::string(1, rank);
        };
        std::string s = sqToStr(from) + sqToStr(to);
        switch (promotion) {
            case QUEEN:  s += 'q'; break;
            case ROOK:   s += 'r'; break;
            case BISHOP: s += 'b'; break;
            case KNIGHT: s += 'n'; break;
            default: break;
        }
        return s;
    }
};