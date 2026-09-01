#include "evaluation.h"
#include <fstream>
#include <sstream>

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

    constexpr int kingMiddlegamePST[64] = {
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
         20, 20,  0,  0,  0,  0, 20, 20,
         20, 30, 10,  0,  0, 10, 30, 20
    };

 
    struct EvalParams {
        double matN = 320, matB = 330, matR = 500, matQ = 900;
        double scalePawn = 1.3, scaleKnight = 0.85, scaleBishop = 1.2,
               scaleRook = 1.1, scaleQueen = 1.2, scaleKing = 1.3;
    };

    EvalParams params;

} 

bool Evaluation::loadParams(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    EvalParams p = params; 
    std::string line;
    while (std::getline(in, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string valStr = line.substr(eq + 1);
        double val;
        try {
            val = std::stod(valStr);
        } catch (const std::exception&) {
            continue; // linha mal formada, ignora
        }

        if (key == "matN") p.matN = val;
        else if (key == "matB") p.matB = val;
        else if (key == "matR") p.matR = val;
        else if (key == "matQ") p.matQ = val;
        else if (key == "scalePawn") p.scalePawn = val;
        else if (key == "scaleKnight") p.scaleKnight = val;
        else if (key == "scaleBishop") p.scaleBishop = val;
        else if (key == "scaleRook") p.scaleRook = val;
        else if (key == "scaleQueen") p.scaleQueen = val;
        else if (key == "scaleKing") p.scaleKing = val;
    }

    params = p;
    return true;
}

int Evaluation::pieceValue(PieceType t) {
    switch (t) {
        case PAWN:   return 100;
        case KNIGHT: return static_cast<int>(params.matN);
        case BISHOP: return static_cast<int>(params.matB);
        case ROOK:   return static_cast<int>(params.matR);
        case QUEEN:  return static_cast<int>(params.matQ);
        default:     return 0;
    }
}

int Evaluation::pstValue(PieceType t, Color c, int square) {
    int idx = (c == WHITE) ? (square ^ 56) : square;

    switch (t) {
        case PAWN:   return static_cast<int>(pawnPST[idx] * params.scalePawn);
        case KNIGHT: return static_cast<int>(knightPST[idx] * params.scaleKnight);
        case BISHOP: return static_cast<int>(bishopPST[idx] * params.scaleBishop);
        case ROOK:   return static_cast<int>(rookPST[idx] * params.scaleRook);
        case QUEEN:  return static_cast<int>(queenPST[idx] * params.scaleQueen);
        case KING:   return static_cast<int>(kingMiddlegamePST[idx] * params.scaleKing);
        default:     return 0;
    }
}

int Evaluation::evaluate(const Board& board) {
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        const Piece& p = board.at(sq);
        if (p.isEmpty()) continue;
        int value = pieceValue(p.type) + pstValue(p.type, p.color, sq);
        score += (p.color == WHITE) ? value : -value;
    }
    return score;
}