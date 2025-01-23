#pragma once

#include "board.hpp"
#include "datastack.hpp"
#include "move.hpp"
#include "transposition.hpp"
#include "types.hpp"

class Search {
private:
    DataStack ds;
    TranspositionTable tt;
    uint32_t maxDepth = 5;
    uint32_t vis = 0;
    int moveStrength[MAX_MOVE_SIZE];

    void calculate(uint32_t depth);
    void quiescent();
    template <Side>
    void moveBySide();

public:
    Search(Board board) : ds(board) {}
    void start();
};
