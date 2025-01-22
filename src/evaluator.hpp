#pragma once

#include "move.hpp"
#include "types.hpp"

const int PSQT[6][64] = {{0, 0, 0, 0, 0, 0, 0, 0,
                          5, 10, 10, 10, 10, 10, 10, 5,
                          -5, 0, 0, 0, 0, 0, 0, -5,
                          -5, 0, 0, 0, 0, 0, 0, -5,
                          -5, 0, 0, 0, 0, 0, 0, -5,
                          -5, 0, 0, 0, 0, 0, 0, -5,
                          -5, 0, 0, 0, 0, 0, 0, -5,
                          0, 0, 0, 5, 5, 0, 0, 0},
                         {-50, -40, -30, -30, -30, -30, -40, -50,
                          -40, -20, 0, 0, 0, 0, -20, -40,
                          -30, 0, 10, 15, 15, 10, 0, -30,
                          -30, 5, 15, 20, 20, 15, 5, -30,
                          -30, 0, 15, 20, 20, 15, 0, -30,
                          -30, 5, 10, 15, 15, 10, 5, -30,
                          -40, -20, 0, 5, 5, 0, -20, -40,
                          -50, -40, -30, -30, -30, -30, -40, -50},
                         {-20, -10, -10, -10, -10, -10, -10, -20,
                          -10, 0, 0, 0, 0, 0, 0, -10,
                          -10, 0, 5, 10, 10, 5, 0, -10,
                          -10, 5, 5, 10, 10, 5, 5, -10,
                          -10, 0, 10, 10, 10, 10, 0, -10,
                          -10, 10, 10, 10, 10, 10, 10, -10,
                          -10, 5, 0, 0, 0, 0, 5, -10,
                          -20, -10, -10, -10, -10, -10, -10, -20},
                         {-20, -10, -10, -5, -5, -10, -10, -20,
                          -10, 0, 0, 0, 0, 0, 0, -10,
                          -10, 0, 5, 5, 5, 5, 0, -10,
                          -5, 0, 5, 5, 5, 5, 0, -5,
                          0, 0, 5, 5, 5, 5, 0, -5,
                          -10, 5, 5, 5, 5, 5, 0, -10,
                          -10, 0, 5, 0, 0, 0, 0, -10,
                          -20, -10, -10, -5, -5, -10, -10, -20},
                         {0, 0, 0, 0, 0, 0, 0, 0,
                          50, 50, 50, 50, 50, 50, 50, 50,
                          10, 10, 20, 30, 30, 20, 10, 10,
                          5, 5, 10, 25, 25, 10, 5, 5,
                          0, 0, 0, 20, 20, 0, 0, 0,
                          5, -5, -10, 0, 0, -10, -5, 5,
                          5, 10, 10, -20, -20, 10, 10, 5,
                          0, 0, 0, 0, 0, 0, 0, 0},
                         {-30, -40, -40, -50, -50, -40, -40, -30,
                          -30, -40, -40, -50, -50, -40, -40, -30,
                          -30, -40, -40, -50, -50, -40, -40, -30,
                          -30, -40, -40, -50, -50, -40, -40, -30,
                          -20, -30, -30, -40, -40, -30, -30, -20,
                          -10, -20, -20, -20, -20, -20, -20, -10,
                          20, 20, 0, 0, 0, 0, 20, 20,
                          20, 30, 10, 0, 0, 10, 30, 20}};

const int PIECE_VALUE[6] = {500, 320, 330, 900, 100, 20000};

class Evaluator {
private:
    ull zobrist = 0;
    int eval = 0;
    int pt[64]; // which piece is positioned at each square
    // positive = white
    // negative = black
    // numeration starts with one
    int getSquareVal(int sq);
    int getRawSquareVal(int sq, int pt);
    ull getSquareHash(int sq);

public:
    inline int getEval(Color col) { return col ? -eval : eval; }
    Evaluator() {};
    void update(Move m);
    int getMoveStrength(Move m, Color col);
    inline int *getPT() { return pt; }
    void reset();
    void print();
    inline ull getMainZobrist() { return zobrist; }
};
