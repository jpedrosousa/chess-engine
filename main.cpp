#include "board.h"
#include "movegen.h"
#include "search.h"
#include "play.h"
#include <iostream>
#include <vector>
#include <string>


static long long perft(const Board& board, int depth) {
    if (depth == 0) return 1;
    long long nodes = 0;
    for (const Move& m : MoveGenerator::generateLegalMoves(board)) {
        Board next = board.makeMove(m);
        nodes += perft(next, depth - 1);
    }
    return nodes;
}


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

static int runTests() {
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

    std::cout << "\n--- Perft na posição inicial (valores conhecidos: 20, 400, 8902) ---\n";
    Board start;
    start.initStartPosition();
    for (int depth = 1; depth <= 3; depth++) {
        std::cout << "perft(" << depth << ") = " << perft(start, depth) << "\n";
    }

    std::cout << "\n--- Testando isCheckmate/isStalemate em posições conhecidas ---\n";

    struct TestCase { std::string name; std::string fen; Color sideToTest; bool expectedCheckmate; };
    std::vector<TestCase> cases = {
        // Mate do pastor: 1.e4 e5 2.Bc4 Nc6 3.Qh5 Nf6?? 4.Qxf7#
        {"Mate do pastor", "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4", BLACK, true},
        // Mate do idiota: 1.f3 e5 2.g4?? Qh4#
        {"Mate do idiota", "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3", WHITE, true},
        // Posição inicial claramente não é mate
        {"Posicao inicial (nao e mate)", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", WHITE, false},
    };

    bool allTestsOk = true;
    for (const auto& tc : cases) {
        Board b;
        b.setFromFEN(tc.fen);
        bool inCheck = MoveGenerator::inCheck(b, tc.sideToTest);
        bool mate = MoveGenerator::isCheckmate(b, tc.sideToTest);
        bool testOk = (mate == tc.expectedCheckmate);
        allTestsOk &= testOk;
        std::cout << (testOk ? "[OK]   " : "[FAIL] ") << tc.name
                   << " | em xeque: " << (inCheck ? "sim" : "nao")
                   << " | xeque-mate: " << (mate ? "sim" : "nao")
                   << " | lances legais: " << MoveGenerator::generateLegalMoves(b).size()
                   << "\n";
    }
    std::cout << (allTestsOk ? "Todos os testes de mate passaram.\n" : "Algum teste de mate falhou!\n");

    std::cout << "\n--- Testando Search::findBestMove ---\n";


    {
        Board b;
        b.setFromFEN("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
        SearchResult result = Search::findBestMove(b, 2);
        std::cout << "Posicao com mate em 1 disponivel: melhor lance = " << result.bestMove.toUCI()
                   << " (esperado: h5f7), score = " << result.score << "\n";
    }

    {
        Board b;
        b.setFromFEN("4k3/8/8/3n4/8/8/3R4/4K3 w - - 0 1"); 
        SearchResult result = Search::findBestMove(b, 3);
        std::cout << "Posicao com captura de cavalo disponivel: melhor lance = " << result.bestMove.toUCI()
                   << " (esperado: capturar em d5), score = " << result.score << "\n";
    }

    return ok && allTestsOk ? 0 : 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--test") {
        return runTests();
    }

    std::cout << "=== Motor de Xadrez ===\n";
    std::cout << "Escolha sua cor - brancas ou pretas (b/p): ";
    char colorChoice = 'b';
    std::cin >> colorChoice;
    Color humanColor = (colorChoice == 'p' || colorChoice == 'P') ? BLACK : WHITE;

    std::cout << "Profundidade de busca do motor em plies (recomendado 3-5): ";
    int depth = 4;
    std::cin >> depth;
    if (depth < 1) depth = 1;
    if (depth > 6) {
        std::cout << "Profundidade " << depth << " pode ficar bem lenta; use com cautela.\n";
    }

    playInteractiveGame(humanColor, depth);
    return 0;
}