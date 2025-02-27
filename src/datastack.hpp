#pragma once

#include "board.hpp"
#include "evaluator.hpp"
#include "move.hpp"
#include "types.hpp"

class BoardData {
public:
    Evaluator evaluator;
    Board board;
    Move *endMove;
    Move *curMove;
    Move moves[MAX_MOVE_SIZE];
    Move pvLine[DATA_STACK_SIZE];
    uint32_t pvLen;
    int alpha;
    int beta;
    int evalBuffer;
    int isStart;
    Move bestMove;

    BoardData(Board board = Board()) : board(board) {
        alpha = WORST_EVAL;
        beta = PERFECT_EVAL;
        uint32_t isStart = 0;
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
        stack[0].board = startingBoard;
        stack[0].evaluator.setBoard(&stack[0].board);
        stack[0].evaluator.reset();
        for (int i = 1; i < DATA_STACK_SIZE; ++i) stack[i].evaluator.setBoard(&stack[i].board);
        pointer = 1;
        // add new position to rep table
        increaseRepetitions(stack[0].getZobrist());
    }

    inline BoardData &getTopBD() { return stack[pointer - 1]; }
    inline BoardData &getCurBD() { return stack[0]; }
    inline uint32_t &getCurDepth() { return pointer; }
    inline Move *getFuturePVLine() { return stack[pointer].pvLine; }
    inline uint32_t &getFuturePVLen() { return stack[pointer].pvLen; }
    void down(Move m);
    void up(int value);
    void makeMove(Move m);
    void setAB(int alpha, int beta);

    void decreaseRepetitions(uint32_t hash);
    void increaseRepetitions(uint32_t hash);
    uint32_t getRepetitions(uint32_t hash);
};
