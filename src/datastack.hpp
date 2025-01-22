#pragma once

#include "board.hpp"
#include "evaluator.hpp"
#include "move.hpp"
#include "types.hpp"

class BoardData {
public:
    Move moves[MAX_MOVE_SIZE];
    Move *endMove;
    Move *curMove;
    Evaluator evaluator;
    Board board;
    int alpha;
    int beta;
    int evalBuffer;
    Move bestMove;

    BoardData(Board board = Board()) : board(board) {
        board.setPT(evaluator.getPT());
        evaluator.reset();
        alpha = WORST_EVAL;
        beta = PERFECT_EVAL;
    }

    inline int getEval() { return evaluator.getEval(board.getCol()); }

    ull getZobrist() { return evaluator.getMainZobrist() ^ board.getExtraZobrist(); }
};

class DataStack {
private:
    BoardData stack[DATA_STACK_SIZE];
    uint8_t repetitionTable[REPETITION_TABLE_SIZE]; // 16 kb
    uint32_t pointer;

public:
    DataStack(Board &startingBoard) : repetitionTable{0} {
        stack[0] = BoardData(startingBoard);
        pointer = 1;
        // add new position to rep table
        increaseRepetitions(stack[0].getZobrist());
    }

    inline BoardData &getTopBD() { return stack[pointer - 1]; }
    inline BoardData &getCurBD() { return stack[0]; }
    inline uint32_t &getCurDepth() { return pointer; }
    void down(Move m);
    void up(int value);
    void makeMove(Move m);
    void setAB(int alpha, int beta);

    void decreaseRepetitions(uint32_t hash);
    void increaseRepetitions(uint32_t hash);
    uint32_t getRepetitions(uint32_t hash);
};
