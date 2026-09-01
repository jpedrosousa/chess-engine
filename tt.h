#pragma once
#include "move.h"
#include <cstdint>
#include <vector>

enum class TTFlag : uint8_t { EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry {
    uint64_t key = 0;
    int depth = -1;
    int score = 0;
    TTFlag flag = TTFlag::EXACT;
    Move bestMove;
    bool valid = false;
};
class TranspositionTable {
public:
    explicit TranspositionTable(size_t sizeMB = 32);

    void clear();

    // Retorna ponteiro para a entrada se a chave bater (independente de profundidade),
    // util para pegar o melhor lance conhecido (ordenacao) mesmo sem corte.
    const TTEntry* find(uint64_t key) const;

    // Tenta um corte: se houver entrada valida, profunda o suficiente e cujo
    // score seja utilizavel dado o flag/alpha/beta, preenche outScore e retorna true.
    // outMove sempre e preenchido quando ha entrada (mesmo sem corte), para ordenacao.
    bool probe(uint64_t key, int depth, int alpha, int beta, int& outScore, Move& outMove) const;

    void store(uint64_t key, int depth, int score, TTFlag flag, const Move& bestMove);

    size_t sizeInEntries() const { return table.size(); }

private:
    std::vector<TTEntry> table;
    size_t indexFor(uint64_t key) const { return key % table.size(); }
};