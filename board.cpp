#include "board.h"
#include "move.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>

namespace {
    constexpr int A1 = 0, H1 = 7, A8 = 56, H8 = 63;
}

Board::Board() {
    squares.fill(Piece{});
}

void Board::initStartPosition() {
    setFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

char Board::pieceToChar(const Piece& p) {
    if (p.isEmpty()) return '.';

    char c;
    switch (p.type) {
        case PAWN:   c = 'p'; break;
        case KNIGHT: c = 'n'; break;
        case BISHOP: c = 'b'; break;
        case ROOK:   c = 'r'; break;
        case QUEEN:  c = 'q'; break;
        case KING:   c = 'k'; break;
        default:     c = '.'; break;
    }
    return p.color == WHITE ? static_cast<char>(std::toupper(c)) : c;
}

Piece Board::charToPiece(char c) {
    Piece p;
    p.color = std::isupper(static_cast<unsigned char>(c)) ? WHITE : BLACK;

    switch (std::tolower(c)) {
        case 'p': p.type = PAWN;   break;
        case 'n': p.type = KNIGHT; break;
        case 'b': p.type = BISHOP; break;
        case 'r': p.type = ROOK;   break;
        case 'q': p.type = QUEEN;  break;
        case 'k': p.type = KING;   break;
        default:
            throw std::invalid_argument("Caractere de peça inválido no FEN: " + std::string(1, c));
    }
    return p;
}

int Board::squareFromAlgebraic(const std::string& s) {
    if (s.size() != 2) throw std::invalid_argument("Casa inválida: " + s);
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7)
        throw std::invalid_argument("Casa fora do tabuleiro: " + s);
    return rank * 8 + file;
}

std::string Board::algebraicFromSquare(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string(1, file) + std::string(1, rank);
}

void Board::setFromFEN(const std::string& fen) {
    squares.fill(Piece{});

    std::istringstream iss(fen);
    std::string boardPart, sidePart, castlingPart, epPart, halfmovePart, fullmovePart;

    if (!(iss >> boardPart >> sidePart >> castlingPart >> epPart)) {
        throw std::invalid_argument("FEN incompleto: precisa de ao menos posição, lado, roque e en passant: " + fen);
    }

    if (!(iss >> halfmovePart)) halfmovePart = "0";
    if (!(iss >> fullmovePart)) fullmovePart = "1";

    if (sidePart != "w" && sidePart != "b")
        throw std::invalid_argument("Lado a jogar inválido no FEN (esperado 'w' ou 'b'): " + sidePart);

    int rank = 7;
    int file = 0;
    for (char c : boardPart) {
        if (c == '/') {
            if (file != 8)
                throw std::invalid_argument("Fileira incompleta/excedente no FEN antes de '/': " + boardPart);
            rank--;
            file = 0;
            if (rank < 0)
                throw std::invalid_argument("FEN tem mais de 8 fileiras: " + boardPart);
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            int skip = c - '0';
            if (skip < 1 || skip > 8)
                throw std::invalid_argument("Número inválido no FEN: " + std::string(1, c));
            file += skip;
            if (file > 8)
                throw std::invalid_argument("FEN excede 8 colunas em uma fileira: " + boardPart);
        } else {
            if (file >= 8)
                throw std::invalid_argument("FEN excede 8 colunas em uma fileira: " + boardPart);
            at(file, rank) = charToPiece(c); // charToPiece já lança se o caractere for inválido
            file++;
        }
    }
    if (rank != 0 || file != 8)
        throw std::invalid_argument("FEN não descreve exatamente 8 fileiras de 8 colunas: " + boardPart);

    sideToMove = (sidePart == "w") ? WHITE : BLACK;

    whiteCanCastleKingside  = castlingPart.find('K') != std::string::npos;
    whiteCanCastleQueenside = castlingPart.find('Q') != std::string::npos;
    blackCanCastleKingside  = castlingPart.find('k') != std::string::npos;
    blackCanCastleQueenside = castlingPart.find('q') != std::string::npos;

    enPassantSquare = (epPart == "-") ? -1 : squareFromAlgebraic(epPart);

    try {
        halfmoveClock  = std::stoi(halfmovePart);
        fullmoveNumber = std::stoi(fullmovePart);
    } catch (const std::exception&) {
        throw std::invalid_argument("Contadores de meio-lance/lance inválidos no FEN: " + fen);
    }
}

std::string Board::toFEN() const {
    std::ostringstream oss;

    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;
        for (int file = 0; file < 8; file++) {
            const Piece& p = at(file, rank);
            if (p.isEmpty()) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    oss << emptyCount;
                    emptyCount = 0;
                }
                oss << pieceToChar(p);
            }
        }
        if (emptyCount > 0) oss << emptyCount;
        if (rank > 0) oss << '/';
    }

    oss << ' ' << (sideToMove == WHITE ? 'w' : 'b') << ' ';

    std::string castling;
    if (whiteCanCastleKingside)  castling += 'K';
    if (whiteCanCastleQueenside) castling += 'Q';
    if (blackCanCastleKingside)  castling += 'k';
    if (blackCanCastleQueenside) castling += 'q';
    oss << (castling.empty() ? "-" : castling) << ' ';

    oss << (enPassantSquare == -1 ? "-" : algebraicFromSquare(enPassantSquare)) << ' ';
    oss << halfmoveClock << ' ' << fullmoveNumber;

    return oss.str();
}

int Board::kingSquare(Color c) const {
    for (int sq = 0; sq < 64; sq++) {
        const Piece& p = squares[sq];
        if (p.type == KING && p.color == c) return sq;
    }
    return -1;
}

int Board::countPieces(Color c, PieceType t) const {
    int count = 0;
    for (const Piece& p : squares) {
        if (p.type == t && p.color == c) count++;
    }
    return count;
}

Board Board::makeMove(const Move& m) const {
    Board nb = *this;
    Piece moving = nb.at(m.from);
    Color us = moving.color;
    Color them = (us == WHITE) ? BLACK : WHITE;

    if (m.isEnPassant) {
        int capturedSq = (us == WHITE) ? m.to - 8 : m.to + 8;
        nb.at(capturedSq) = Piece{};
    }

    nb.at(m.to) = moving;
    nb.at(m.from) = Piece{};

    if (m.promotion != EMPTY) {
        nb.at(m.to).type = m.promotion;
    }

    if (m.isCastleKingside) {
        int rookFrom = (us == WHITE) ? H1 : H8;
        int rookTo   = rookFrom - 2; 
        nb.at(rookTo) = nb.at(rookFrom);
        nb.at(rookFrom) = Piece{};
    } else if (m.isCastleQueenside) {
        int rookFrom = (us == WHITE) ? A1 : A8;
        int rookTo   = rookFrom + 3; // a1(0)->d1(3)
        nb.at(rookTo) = nb.at(rookFrom);
        nb.at(rookFrom) = Piece{};
    }

    nb.enPassantSquare = m.isDoublePawnPush
        ? (us == WHITE ? m.from + 8 : m.from - 8)
        : -1;

    if (moving.type == KING) {
        if (us == WHITE) { nb.whiteCanCastleKingside = false; nb.whiteCanCastleQueenside = false; }
        else             { nb.blackCanCastleKingside = false; nb.blackCanCastleQueenside = false; }
    }
    auto clearRookRight = [&](int square) {
        if (square == A1) nb.whiteCanCastleQueenside = false;
        if (square == H1) nb.whiteCanCastleKingside  = false;
        if (square == A8) nb.blackCanCastleQueenside = false;
        if (square == H8) nb.blackCanCastleKingside  = false;
    };
    clearRookRight(m.from);
    clearRookRight(m.to); 
    
    if (moving.type == PAWN || m.isCapture || m.isEnPassant) nb.halfmoveClock = 0;
    else nb.halfmoveClock++;

    if (us == BLACK) nb.fullmoveNumber++;

    nb.sideToMove = them;
    return nb;
}

Board Board::makeNullMove() const {
    Board nb = *this;
    nb.sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
    nb.enPassantSquare = -1;
    nb.halfmoveClock++;
    if (sideToMove == BLACK) nb.fullmoveNumber++;
    return nb;
}

void Board::print() const {
    std::cout << "\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            std::cout << pieceToChar(at(file, rank)) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";
    std::cout << "Lado a jogar: " << (sideToMove == WHITE ? "Brancas" : "Pretas") << "\n";
    std::cout << "FEN: " << toFEN() << "\n\n";
}