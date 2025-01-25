#pragma once

#include "board.hpp"
#include "datastack.hpp"
#include "move.hpp"
#include "transposition.hpp"
#include "types.hpp"
#include <chrono>
#include <thread>

class Search {
private:
    DataStack ds;
    TranspositionTable tt;
    uint32_t maxDepth = 6;
    uint32_t aspirationWindow = 75;
    uint32_t vis = 0;
    int moveStrength[MAX_MOVE_SIZE];
    Move PVStack[DATA_STACK_SIZE];
    uint32_t PVPointer = 0;
    bool pvUsed;
    bool debug = 0;
    int alpha;
    int beta;
    int moveTL = 70;
    bool workerHasToStop;

    void calculate(uint32_t depth);
    void quiescent();
    template <Side>
    void moveBySide();
    void setWorkerTimer();

public:
    Search(Board board) : ds(board), PVStack{0} {}
    void start();
};
