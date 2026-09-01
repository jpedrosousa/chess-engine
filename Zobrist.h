#pragma once
#include "board.h"
#include <cstdint>


namespace Zobrist {
    uint64_t computeHash(const Board& board);
}