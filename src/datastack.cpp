#include "datastack.hpp"
#include "types.hpp"

void DataStack::down(Move m) {
    BoardData &oldBD = stack[pointer - 1], &newBD = stack[pointer];
    memcpy(&newBD.evaluator, &oldBD.evaluator, sizeof(Evaluator));
    oldBD.board.applyMove(m, &newBD.board);
    newBD.evaluator.update(m);
    newBD.alpha = -oldBD.beta;
    newBD.beta = -oldBD.alpha;
    increaseRepetitions(newBD.getZobrist());
    ++pointer;
}

void DataStack::up(int value) {
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
    bd.board.setPT(bd.evaluator.getPT());
    bd.evaluator.reset();
    bd.alpha = WORST_EVAL;
    bd.beta = PERFECT_EVAL;
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
