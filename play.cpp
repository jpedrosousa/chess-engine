#include "play.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <string>
#include <cctype>
#include <optional>

namespace {

PieceType charToPromotion(char c) {
    switch (std::tolower(static_cast<unsigned char>(c))) {
        case 'q': return QUEEN;
        case 'r': return ROOK;
        case 'b': return BISHOP;
        case 'n': return KNIGHT;
        default:  return EMPTY;
    }
}

std::optional<Move> tryParseMove(const std::string& input, const Board& board) {
    if (input.size() != 4 && input.size() != 5) return std::nullopt;

    int from, to;
    try {
        from = Board::squareFromAlgebraic(input.substr(0, 2));
        to   = Board::squareFromAlgebraic(input.substr(2, 2));
    } catch (const std::exception&) {
        return std::nullopt;
    }

    PieceType requestedPromo = EMPTY;
    if (input.size() == 5) {
        requestedPromo = charToPromotion(input[4]);
        if (requestedPromo == EMPTY) return std::nullopt; // letra de promoção inválida
    }

    std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);

    const Move* fallbackQueenPromo = nullptr;
    for (const Move& m : legalMoves) {
        if (m.from != from || m.to != to) continue;

        if (requestedPromo != EMPTY) {
            if (m.promotion == requestedPromo) return m;
        } else {
            if (m.promotion == EMPTY) return m; // lance comum, sem ambiguidade
            if (m.promotion == QUEEN) fallbackQueenPromo = &m;
        }
    }

    if (fallbackQueenPromo) return *fallbackQueenPromo;
    return std::nullopt;
}

std::string colorName(Color c) { return c == WHITE ? "Brancas" : "Pretas"; }

} // namespace

void playInteractiveGame(Color humanColor, int engineDepth) {
    Board board;
    board.initStartPosition();

    std::cout << "\n=== Partida iniciada. Voce joga de " << colorName(humanColor)
               << ". Motor busca " << engineDepth << " plies a frente. ===\n";
    std::cout << "Digite lances em notacao UCI (ex: e2e4, e7e8q para promover a dama). "
               << "Digite 'sair' para encerrar.\n";

    while (true) {
        board.print();

        Color side = board.sideToMove;
        std::vector<Move> legalMoves = MoveGenerator::generateLegalMoves(board);

        if (legalMoves.empty()) {
            if (MoveGenerator::inCheck(board, side)) {
                Color winner = (side == WHITE) ? BLACK : WHITE;
                std::cout << "Xeque-mate! " << colorName(winner) << " vencem.\n";
            } else {
                std::cout << "Afogamento (stalemate). Empate.\n";
            }
            break;
        }

        if (board.halfmoveClock >= 100) {
            std::cout << "Empate pela regra dos 50 lances sem captura ou movimento de peao.\n";
            break;
        }

        if (side == humanColor) {
            bool moveMade = false;
            while (!moveMade) {
                std::cout << colorName(side) << ", seu lance: ";
                std::string input;
                if (!(std::cin >> input)) {
                    std::cout << "\nEntrada encerrada. Saindo.\n";
                    return;
                }
                if (input == "sair" || input == "exit" || input == "quit") {
                    std::cout << "Saindo da partida.\n";
                    return;
                }

                std::optional<Move> parsed = tryParseMove(input, board);
                if (!parsed) {
                    std::cout << "Lance invalido ou ilegal: '" << input << "'. Tente novamente.\n";
                    continue;
                }

                board = board.makeMove(*parsed);
                moveMade = true;
            }
        } else {
            std::cout << "Motor (" << colorName(side) << ") pensando...\n";
            SearchResult result = Search::findBestMove(board, engineDepth);
            std::cout << "Motor joga: " << result.bestMove.toUCI()
                       << " (avaliacao: " << result.score << ")\n";
            board = board.makeMove(result.bestMove);
        }
    }

    std::cout << "FEN final: " << board.toFEN() << "\n";
}