#pragma once

#include "types.hpp"

const ull TT_SIZE = 1 << 17; // 4 mb

const ull EVAL_MASK = (1ull << 32) - 1;
const ull BEST_MOVE_MASK = ((1ull << 16) - 1) << 32;
const ull DEPTH_MASK = ((1ull << 8) - 1) << 48;
const ull EVAL_TYPE_MASK = 3ull << 56;

const int INVALID_TABLE_VALUE = -999999999;

class TTEntry {
public:
    ull hash;
    ull data; // first 32 bits - eval, next 16 - bestMove, 8 - depth, next 2 - et, last 6 - vacant

    TTEntry() : data(0), hash{0} {};

    int probe(ull hash, uint32_t depth, int alpha, int beta) {
        if (hash == this->hash) {
            int eval = static_cast<int>(data & EVAL_MASK);
            uint32_t entryDepth = (data & DEPTH_MASK) >> 48;
            EvalType et = static_cast<EvalType>((data & EVAL_TYPE_MASK) >> 56);
            if (entryDepth >= depth) {
                if (et == EXACT) return eval;
                if (et == ALPHA && eval <= alpha) return alpha;
                if (et == BETA && eval >= beta) return beta;
            }
        }
        return INVALID_TABLE_VALUE;
    }

    void add(ull hash, int eval, uint32_t depth, EvalType et) {
        this->hash = hash;
        this->data = static_cast<ull>(eval) | (static_cast<ull>(depth) << 48) | (static_cast<ull>(et) << 56);
    }
};

class TranspositionTable {
private:
    TTEntry table[TT_SIZE];

public:
    inline int probe(ull hash, uint32_t depth, int alpha, int beta) {
        return table[hash & (TT_SIZE - 1)].probe(hash, depth, alpha, beta);
    }
    inline void add(ull hash, int eval, uint32_t depth, EvalType et) {
        table[hash & (TT_SIZE - 1)].add(hash, eval, depth, et);
    }
};
