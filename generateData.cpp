
#include "board.h"
#include "movegen.h"
#include "search.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <string>
 
int main(int argc, char* argv[]) {
    int numGames = (argc > 1) ? std::stoi(argv[1]) : 200;
    int searchDepth = (argc > 2) ? std::stoi(argv[2]) : 2;
    int randomOpeningPlies = (argc > 3) ? std::stoi(argv[3]) : 6;
    std::string outPath = (argc > 4) ? argv[4] : "training_data.txt";
 
    std::ofstream out(outPath);
    std::mt19937 rng(std::random_device{}());
 
    int totalPositions = 0;
 
    for (int g = 0; g < numGames; g++) {
        Board board;
        board.initStartPosition();
 
        std::vector<std::string> quietFens;
        int result = -1; 
        bool realEnding = false;
 
        for (int ply = 0; ply < 220; ply++) {
            std::vector<Move> legal = MoveGenerator::generateLegalMoves(board);
            if (legal.empty()) {
                bool inCheck = MoveGenerator::inCheck(board, board.sideToMove);
                if (inCheck) {
                    result = (board.sideToMove == WHITE) ? 0 : 2;
                } else {
                    result = 1; 
                }
                realEnding = true;
                break;
            }
            if (board.halfmoveClock >= 100) { result = 1; realEnding = true; break; }
 
            if (!MoveGenerator::inCheck(board, board.sideToMove)) {
                quietFens.push_back(board.toFEN());
            }
 
            Move chosen;
            if (ply < randomOpeningPlies) {
                std::uniform_int_distribution<size_t> dist(0, legal.size() - 1);
                chosen = legal[dist(rng)];
            } else {
                SearchResult sr = Search::findBestMove(board, searchDepth);
                chosen = sr.bestMove;
            }
            board = board.makeMove(chosen);
        }
 
        if (!realEnding) continue; 
        for (const auto& fen : quietFens) {
            out << fen << ";" << result << "\n";
            totalPositions++;
        }
 
        if ((g + 1) % 20 == 0) {
            std::cerr << "Partidas geradas: " << (g + 1) << "/" << numGames
                       << " | posicoes ate agora: " << totalPositions << "\n";
        }
    }
 
    std::cerr << "Concluido. Total de posicoes: " << totalPositions << " em " << outPath << "\n";
    return 0;
}
 
