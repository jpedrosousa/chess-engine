#include "board.h"
#include "movegen.h"
#include "search.h"
#include "evaluation.h"
#include "play.h"
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
using namespace std;


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
        cout << (ok ? "[OK]   " : "[FAIL] ") << fen;
        if (!ok) cout << "  -> obteve: " << result;
        cout << "\n";
    }
    return allOk;
}

static int runTests() {
    Board board;
    board.initStartPosition();
    board.print();

    board.setFromFEN("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    board.print();

    cout << "--- Teste de round-trip (FEN -> board -> FEN) ---\n";
    bool ok = testRoundTrip({
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "8/8/8/8/8/8/8/4K2k w - - 0 1",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b - e3 0 1",
    });
    cout << (ok ? "Todos os testes passaram.\n" : "Algum teste falhou!\n");

    cout << "\n--- Perft na posição inicial (valores conhecidos: 20, 400, 8902) ---\n";
    Board start;
    start.initStartPosition();
    for (int depth = 1; depth <= 3; depth++) {
        cout << "perft(" << depth << ") = " << perft(start, depth) << "\n";
    }

    cout << "\n--- Testando isCheckmate/isStalemate em posições conhecidas ---\n";

    struct TestCase { std::string name; std::string fen; Color sideToTest; bool expectedCheckmate; };
    std::vector<TestCase> cases = {
        {"Mate do pastor", "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4", BLACK, true},
        {"Mate do idiota", "rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3", WHITE, true},
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
        cout << (testOk ? "[OK]   " : "[FAIL] ") << tc.name
                   << " | em xeque: " << (inCheck ? "sim" : "nao")
                   << " | xeque-mate: " << (mate ? "sim" : "nao")
                   << " | lances legais: " << MoveGenerator::generateLegalMoves(b).size()
                   << "\n";
    }
    cout << (allTestsOk ? "Todos os testes de mate passaram.\n" : "Algum teste de mate falhou!\n");

    cout << "\n--- Testando Search::findBestMove ---\n";


    {
        Board b;
        b.setFromFEN("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
        SearchResult result = Search::findBestMove(b, 2);
        cout << "Posicao com mate em 1 disponivel: melhor lance = " << result.bestMove.toUCI()
                   << " (esperado: h5f7), score = " << result.score << "\n";
    }

    {
        Board b;
        b.setFromFEN("4k3/8/8/3n4/8/8/3R4/4K3 w - - 0 1");
        SearchResult result = Search::findBestMove(b, 3);
        cout << "Posicao com captura de cavalo disponivel: melhor lance = " << result.bestMove.toUCI()
                   << " (esperado: capturar em d5), score = " << result.score << "\n";
    }

    cout << "\n--- Testando iterative deepening + transposition table em profundidade maior ---\n";
    bool ttOk = true;
    {
        Board b;
        b.setFromFEN("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
        SearchResult result = Search::findBestMove(b, 4);
        bool testOk = (result.bestMove.toUCI() == "h5f7") && (result.depthReached == 4);
        ttOk &= testOk;
        cout << (testOk ? "[OK]   " : "[FAIL] ")
                   << "Mate em 1 continua encontrado em profundidade 4 (com TT) = " << result.bestMove.toUCI()
                   << ", depthReached = " << result.depthReached << ", score = " << result.score << "\n";
    }
    {
        Board b;
        b.setFromFEN("4k3/8/8/3n4/8/8/3R4/4K3 w - - 0 1");
        SearchResult result = Search::findBestMove(b, 5);
        bool testOk = (result.depthReached == 5) && (result.score > 0);
        ttOk &= testOk;
        cout << (testOk ? "[OK]   " : "[FAIL] ")
                   << "Busca completa profundidade 5 sem travar/quebrar, score = " << result.score
                   << ", depthReached = " << result.depthReached << "\n";
    }
    cout << (ttOk ? "Todos os testes de TT/iterative deepening passaram.\n" : "Algum teste de TT/iterative deepening falhou!\n");

    cout << "\n--- Testando quiescence search (nao deve travar em posicao com capturas em cadeia) ---\n";
    bool qOk = true;
    {
        Board b;
        b.setFromFEN("r3k2r/pp1n1ppp/2p1p3/3n4/1b2P3/2N2N2/PPP2PPP/R1B1K2R w KQkq - 0 1");
        SearchResult result = Search::findBestMove(b, 3);
        bool testOk = (result.bestMove.from != -1);
        qOk &= testOk;
        cout << (testOk ? "[OK]   " : "[FAIL] ")
                   << "Busca em posicao complexa retorna lance valido = " << result.bestMove.toUCI()
                   << ", score = " << result.score << "\n";
    }
    cout << (qOk ? "Teste de quiescence passou.\n" : "Teste de quiescence falhou!\n");

    cout << "\n--- Testando Evaluation::loadParams ---\n";
    bool paramsOk = true;
    {
        Board b;
        b.setFromFEN("4k3/8/8/8/8/8/8/Q3K3 w - - 0 1");
        int scoreBefore = Evaluation::evaluate(b);

        const std::string tmpPath = "test_tuned_params.txt";
        {
            std::ofstream out(tmpPath);
            out << "K=1.0\nmatQ=1000\nscaleQueen=1.2\n";
        }

        bool loaded = Evaluation::loadParams(tmpPath);
        int scoreAfter = Evaluation::evaluate(b);
        std::remove(tmpPath.c_str());

        bool testOk = loaded && (scoreAfter > scoreBefore);
        paramsOk &= testOk;
        cout << (testOk ? "[OK]   " : "[FAIL] ")
                   << "loadParams aplica matQ=1000 corretamente (score antes=" << scoreBefore
                   << ", depois=" << scoreAfter << ")\n";

        bool missingOk = !Evaluation::loadParams("arquivo_que_nao_existe.txt");
        paramsOk &= missingOk;
        cout << (missingOk ? "[OK]   " : "[FAIL] ")
                   << "loadParams retorna false quando arquivo nao existe (defaults mantidos)\n";
    }
        cout << (paramsOk ? "Teste de loadParams passou.\n" : "Teste de loadParams falhou!\n");

    cout << "\n--- Benchmark: profundidade alcancavel em tempo limitado (posicao inicial) ---\n";
    {
        Board b;
        b.initStartPosition();
        for (int d : {5, 6, 7}) {
            auto t0 = std::chrono::steady_clock::now();
            SearchResult result = Search::findBestMove(b, d);
            auto t1 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            cout << "profundidade " << d << ": lance = " << result.bestMove.toUCI()
                       << ", score = " << result.score << ", tempo = " << ms << " ms\n";
        }
    }

    cout << "\n--- Testando guard de null-move em final de so peoes (evita zugzwang) ---\n";
    bool zugzwangOk = true;
    {
        Board b;
        b.setFromFEN("8/8/8/4k3/8/4P3/4K3/8 w - - 0 1");
        SearchResult result = Search::findBestMove(b, 6);
        bool testOk = (result.bestMove.from != -1);
        zugzwangOk &= testOk;
        cout << (testOk ? "[OK]   " : "[FAIL] ")
                   << "Busca em final de peao retorna lance valido = " << result.bestMove.toUCI()
                   << ", score = " << result.score << "\n";
    }
    cout << (zugzwangOk ? "Teste de guard de zugzwang passou.\n" : "Teste de guard de zugzwang falhou!\n");

    return ok && allTestsOk && ttOk && qOk && paramsOk && zugzwangOk ? 0 : 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--test") {
        return runTests();
    }

    if (Evaluation::loadParams()) {
        cout << "Parametros ajustados (tuned_params.txt) carregados.\n";
    } else {
        cout << "tuned_params.txt nao encontrado; usando avaliacao padrao.\n";
    }

    cout << "=== Motor de Xadrez ===\n";
    cout << "Escolha sua cor - brancas ou pretas (b/p): ";
    char colorChoice = 'b';
    cin >> colorChoice;
    Color humanColor = (colorChoice == 'p' || colorChoice == 'P') ? BLACK : WHITE;

    cout << "Profundidade de busca do motor em plies (recomendado 3-5): ";
    int depth = 4;
    cin >> depth;
    if (depth < 1) depth = 1;
    if (depth > 6) {
        cout << "Profundidade " << depth << " pode ficar bem lenta; use com cautela.\n";
    }

    playInteractiveGame(humanColor, depth);
    return 0;
}