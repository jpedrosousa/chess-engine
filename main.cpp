#include "board.h"
#include <iostream>
#include <vector>
#include <string>


static bool testRoundTrip(const std::vector<std::string>& fens) {
    bool allOk = true;
    Board board;
    for (const auto& fen : fens) {
        board.setFromFEN(fen);
        std::string result = board.toFEN();
        bool ok = (result == fen);
        allOk &= ok;
        std::cout << (ok ? "[OK]   " : "[FAIL] ") << fen;
        if (!ok) std::cout << "  -> obteve: " << result;
        std::cout << "\n";
    }
    return allOk;
}

int main() {
    Board board;
    board.initStartPosition();
    board.print();

    board.setFromFEN("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    board.print();

    std::cout << "--- Teste de round-trip (FEN -> board -> FEN) ---\n";
    bool ok = testRoundTrip({
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "8/8/8/8/8/8/8/4K2k w - - 0 1",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b - e3 0 1",
    });
    std::cout << (ok ? "Todos os testes passaram.\n" : "Algum teste falhou!\n");

    return ok ? 0 : 1;
}