#include "tt.h"

TranspositionTable::TranspositionTable(size_t sizeMB) {
    size_t numEntries = (sizeMB * 1024ULL * 1024ULL) / sizeof(TTEntry);
    if (numEntries < 1) numEntries = 1;
    table.resize(numEntries);
}

void TranspositionTable::clear() {
    for (auto& e : table) e = TTEntry{};
}

const TTEntry* TranspositionTable::find(uint64_t key) const {
    const TTEntry& e = table[indexFor(key)];
    if (e.valid && e.key == key) return &e;
    return nullptr;
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& outScore, Move& outMove) const {
    const TTEntry* e = find(key);
    if (!e) return false;

    outMove = e->bestMove; 

    if (e->depth < depth) return false; 

    switch (e->flag) {
        case TTFlag::EXACT:
            outScore = e->score;
            return true;
        case TTFlag::LOWERBOUND:
            if (e->score >= beta) { outScore = e->score; return true; }
            break;
        case TTFlag::UPPERBOUND:
            if (e->score <= alpha) { outScore = e->score; return true; }
            break;
    }
    return false;
}

void TranspositionTable::store(uint64_t key, int depth, int score, TTFlag flag, const Move& bestMove) {
    TTEntry& e = table[indexFor(key)];
    if (!e.valid || e.key != key || depth >= e.depth) {
        e.key = key;
        e.depth = depth;
        e.score = score;
        e.flag = flag;
        e.bestMove = bestMove;
        e.valid = true;
    }
}