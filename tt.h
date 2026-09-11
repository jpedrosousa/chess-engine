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

    const TTEntry* find(uint64_t key) const;

    bool probe(uint64_t key, int depth, int alpha, int beta, int& outScore, Move& outMove) const;

    void store(uint64_t key, int depth, int score, TTFlag flag, const Move& bestMove);

    size_t sizeInEntries() const { return table.size(); }

private:
    std::vector<TTEntry> table;
    size_t indexFor(uint64_t key) const { return key % table.size(); }
};