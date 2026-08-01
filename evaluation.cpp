#include "evaluation.h"

namespace {
    // Tabelas escritas em ordem "visual" (linha 0 = fileira 8, linha 7 = fileira 1),
    // do ponto de vista das brancas. Convenção padrão usada na maioria dos motores.

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

    // Rei em fase de abertura/meio-jogo: prioriza segurança (cantos, atrás de peões).
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
}

int Evaluation::pieceValue(PieceType t) {
    switch (t) {
        case PAWN:   return 100;
        case KNIGHT: return 320;
        case BISHOP: return 330;
        case ROOK:   return 500;
        case QUEEN:  return 900;
        default:     return 0;
    }
}

int Evaluation::pstValue(PieceType t, Color c, int square) {
    // As tabelas acima estão escritas com a fileira 8 na linha 0 (visão das brancas).
    // Nosso Board usa square = rank*8+file com a1 = 0 (fileira 1 embaixo).
    // Para peças brancas, square^56 espelha a fileira e cai direto na tabela.
    // Para peças pretas, o square "cru" já corresponde à casa espelhada equivalente
    // (ex: peão preto perto da promoção em rank2 cai na mesma linha de bônus alto
    // que o peão branco perto da promoção em rank7).
    int idx = (c == WHITE) ? (square ^ 56) : square;

    switch (t) {
        case PAWN:   return pawnPST[idx];
        case KNIGHT: return knightPST[idx];
        case BISHOP: return bishopPST[idx];
        case ROOK:   return rookPST[idx];
        case QUEEN:  return queenPST[idx];
        case KING:   return kingMiddlegamePST[idx];
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