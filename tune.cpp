
#include "board.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
 
namespace {
 
constexpr int pawnPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};
constexpr int knightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};
constexpr int bishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};
constexpr int rookPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};
constexpr int queenPST[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};
constexpr int kingPST[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};
 
const int* pstFor(PieceType t) {
    switch (t) {
        case PAWN:   return pawnPST;
        case KNIGHT: return knightPST;
        case BISHOP: return bishopPST;
        case ROOK:   return rookPST;
        case QUEEN:  return queenPST;
        case KING:   return kingPST;
        default:     return nullptr;
    }
}
 
struct Params {
    double matN = 320, matB = 330, matR = 500, matQ = 900;
    double scalePawn = 1.0, scaleKnight = 1.0, scaleBishop = 1.0,
           scaleRook = 1.0, scaleQueen = 1.0, scaleKing = 1.0;
 
    std::vector<double> toVector() const {
        return {matN, matB, matR, matQ, scalePawn, scaleKnight, scaleBishop, scaleRook, scaleQueen, scaleKing};
    }
    void fromVector(const std::vector<double>& v) {
        matN=v[0]; matB=v[1]; matR=v[2]; matQ=v[3];
        scalePawn=v[4]; scaleKnight=v[5]; scaleBishop=v[6]; scaleRook=v[7]; scaleQueen=v[8]; scaleKing=v[9];
    }
};
 
double materialValue(PieceType t, const Params& p) {
    switch (t) {
        case PAWN:   return 100.0;
        case KNIGHT: return p.matN;
        case BISHOP: return p.matB;
        case ROOK:   return p.matR;
        case QUEEN:  return p.matQ;
        default:     return 0.0;
    }
}
 
double pstScale(PieceType t, const Params& p) {
    switch (t) {
        case PAWN:   return p.scalePawn;
        case KNIGHT: return p.scaleKnight;
        case BISHOP: return p.scaleBishop;
        case ROOK:   return p.scaleRook;
        case QUEEN:  return p.scaleQueen;
        case KING:   return p.scaleKing;
        default:     return 1.0;
    }
}
 
double evaluateWithParams(const Board& board, const Params& p) {
    double score = 0.0;
    for (int sq = 0; sq < 64; sq++) {
        const Piece& piece = board.at(sq);
        if (piece.isEmpty()) continue;
        const int* table = pstFor(piece.type);
        int idx = (piece.color == WHITE) ? (sq ^ 56) : sq;
        double value = materialValue(piece.type, p) + pstScale(piece.type, p) * table[idx];
        score += (piece.color == WHITE) ? value : -value;
    }
    return score;
}
 
struct Sample {
    Board board;
    double result; 
};
 
std::vector<Sample> loadDataset(const std::string& path) {
    std::vector<Sample> samples;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        size_t sep = line.find(';');
        if (sep == std::string::npos) continue;
        std::string fen = line.substr(0, sep);
        int code = std::stoi(line.substr(sep + 1));
        double result = (code == 2) ? 1.0 : (code == 0) ? 0.0 : 0.5;
 
        Sample s;
        try {
            s.board.setFromFEN(fen);
        } catch (...) {
            continue;
        }
        s.result = result;
        samples.push_back(std::move(s));
    }
    return samples;
}
 
double sigmoid(double x, double K) {
    return 1.0 / (1.0 + std::pow(10.0, -K * x / 400.0));
}
 
double computeError(const std::vector<Sample>& samples, const Params& p, double K) {
    double sumSq = 0.0;
    for (const auto& s : samples) {
        double eval = evaluateWithParams(s.board, p); 
        double pred = sigmoid(eval, K);
        double diff = s.result - pred;
        sumSq += diff * diff;
    }
    return sumSq / samples.size();
}
 
} 
int main(int argc, char* argv[]) {
    std::string dataPath = (argc > 1) ? argv[1] : "training_data.txt";
    int epochs = (argc > 2) ? std::stoi(argv[2]) : 8;
 
    std::cerr << "Carregando dataset de " << dataPath << "...\n";
    std::vector<Sample> samples = loadDataset(dataPath);
    std::cerr << "Amostras carregadas: " << samples.size() << "\n";
 
    Params params;
 
    double bestK = 1.0, bestKError = computeError(samples, params, bestK);
    for (double K = 0.2; K <= 3.0; K += 0.2) {
        double err = computeError(samples, params, K);
        if (err < bestKError) { bestKError = err; bestK = K; }
    }
    std::cerr << std::fixed << std::setprecision(6);
    std::cerr << "K inicial escolhido: " << bestK << " (erro=" << bestKError << ")\n";
 
    std::vector<double> vec = params.toVector();
    std::vector<double> steps = {8, 8, 10, 15, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05};
 
    double currentError = computeError(samples, params, bestK);
    std::cerr << "Erro inicial: " << currentError << "\n";
 
    for (int epoch = 0; epoch < epochs; epoch++) {
        bool improvedAny = false;
        for (size_t i = 0; i < vec.size(); i++) {
            double original = vec[i];
 
            vec[i] = original + steps[i];
            params.fromVector(vec);
            double errUp = computeError(samples, params, bestK);
 
            vec[i] = original - steps[i];
            params.fromVector(vec);
            double errDown = computeError(samples, params, bestK);
 
            if (errUp < currentError && errUp <= errDown) {
                vec[i] = original + steps[i];
                currentError = errUp;
                improvedAny = true;
            } else if (errDown < currentError) {
                vec[i] = original - steps[i];
                currentError = errDown;
                improvedAny = true;
            } else {
                vec[i] = original; 
            }
            params.fromVector(vec);
        }
 
        std::cerr << "Epoca " << (epoch + 1) << "/" << epochs
                   << " | erro = " << currentError << "\n";
 
        if (!improvedAny) {
            for (auto& st : steps) st *= 0.5;
        }
    }
 
    params.fromVector(vec);
 
    std::cerr << "\n=== Parametros finais ===\n";
    std::cerr << "K = " << bestK << "\n";
    std::cerr << "matN=" << params.matN << " matB=" << params.matB
               << " matR=" << params.matR << " matQ=" << params.matQ << "\n";
    std::cerr << "scalePawn=" << params.scalePawn << " scaleKnight=" << params.scaleKnight
               << " scaleBishop=" << params.scaleBishop << " scaleRook=" << params.scaleRook
               << " scaleQueen=" << params.scaleQueen << " scaleKing=" << params.scaleKing << "\n";
    std::cerr << "Erro final: " << currentError << " (era " << computeError(samples, Params(), bestK) << " com pesos originais)\n";
 
    std::ofstream out("tuned_params.txt");
    out << "K=" << bestK << "\n";
    out << "matN=" << params.matN << "\n";
    out << "matB=" << params.matB << "\n";
    out << "matR=" << params.matR << "\n";
    out << "matQ=" << params.matQ << "\n";
    out << "scalePawn=" << params.scalePawn << "\n";
    out << "scaleKnight=" << params.scaleKnight << "\n";
    out << "scaleBishop=" << params.scaleBishop << "\n";
    out << "scaleRook=" << params.scaleRook << "\n";
    out << "scaleQueen=" << params.scaleQueen << "\n";
    out << "scaleKing=" << params.scaleKing << "\n";
 
    return 0;
}
