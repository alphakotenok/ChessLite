#include "datastack.hpp"
#include "types.hpp"

void DataStack::down(Move m) {
    BoardData &oldBD = stack[pointer - 1], &newBD = stack[pointer];
    newBD.evaluator.setFrom(oldBD.evaluator);
    oldBD.board.applyMove(m, &newBD.board);
    newBD.evaluator.update(m);
    newBD.alpha = -oldBD.beta;
    newBD.beta = -oldBD.alpha;
    newBD.isStart = 0;
    newBD.bestMove.reset();
    newBD.pvLen = 0;
    increaseRepetitions(newBD.getZobrist());
    ++pointer;
}

void DataStack::up(int value) {
    if (stack[pointer - 1].isStart) {
        stack[pointer - 1].evalBuffer = -value;
        --stack[pointer - 1].isStart;
        return;
    }
    assert(pointer > 1);
    stack[pointer - 2].evalBuffer = value;
    decreaseRepetitions(stack[pointer - 1].getZobrist());
    --pointer;
}

void DataStack::makeMove(Move m) {
    Board tempB;
    BoardData &bd = stack[0];
    bd.board.applyMove(m, &tempB);
    memcpy(&bd.board, &tempB, sizeof(Board));
    bd.evaluator.reset();
    bd.alpha = WORST_EVAL;
    bd.beta = PERFECT_EVAL;
    bd.isStart = 0;
    bd.bestMove.reset();
    bd.pvLen = 0;
    increaseRepetitions(bd.getZobrist());
}

void DataStack::setAB(int alpha, int beta) {
    BoardData &bd = stack[pointer - 1];
    bd.alpha = alpha;
    bd.beta = beta;
}

void DataStack::decreaseRepetitions(uint32_t hash) {
    uint32_t key = hash & (REPETITION_TABLE_SIZE - 1);
    assert(repetitionTable[key]);
    --repetitionTable[key];
}
void DataStack::increaseRepetitions(uint32_t hash) {
    uint32_t key = hash & (REPETITION_TABLE_SIZE - 1);
    ++repetitionTable[key];
}

uint32_t DataStack::getRepetitions(uint32_t hash) {
    uint32_t key = hash & (REPETITION_TABLE_SIZE - 1);
    return repetitionTable[key];
}
