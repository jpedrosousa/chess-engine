#pragma once
#include <array>
#include <string>
#include <cstdint>

enum PieceType : uint8_t {
    EMPTY = 0,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

enum Color : uint8_t {
    WHITE = 0,
    BLACK = 1,
    NO_COLOR = 2
};

struct Piece {
    PieceType type = EMPTY;
    Color color = NO_COLOR;

    bool isEmpty() const { return type == EMPTY; }

    bool operator==(const Piece& other) const {
        return type == other.type && color == other.color;
    }
    bool operator!=(const Piece& other) const { return !(*this == other); }
};

class Board {
public:
    Board();

    void initStartPosition();

    void setFromFEN(const std::string& fen);

    std::string toFEN() const;

    void print() const;

    Piece& at(int square) { return squares[square]; }
    const Piece& at(int square) const { return squares[square]; }
    Piece& at(int file, int rank) { return squares[rank * 8 + file]; }
    const Piece& at(int file, int rank) const { return squares[rank * 8 + file]; }

 
    int kingSquare(Color c) const;

    int countPieces(Color c, PieceType t) const;

    
    Board makeMove(const struct Move& m) const;

    Color sideToMove = WHITE;

    bool whiteCanCastleKingside  = true;
    bool whiteCanCastleQueenside = true;
    bool blackCanCastleKingside  = true;
    bool blackCanCastleQueenside = true;

    int enPassantSquare = -1;

    int halfmoveClock = 0;
    int fullmoveNumber = 1;

    static int squareFromAlgebraic(const std::string& s);
    static std::string algebraicFromSquare(int sq);

private:
    std::array<Piece, 64> squares;

    static char pieceToChar(const Piece& p);
    static Piece charToPiece(char c);
};