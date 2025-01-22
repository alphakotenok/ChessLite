#include "evaluator.hpp"
#include "extra.hpp"
#include "types.hpp"

int aboba = 0;

int Evaluator::getSquareVal(int sq) {
    if (!pt[sq]) return 0;
    return pt[sq] > 0 ? PIECE_VALUE[pt[sq] - 1] + PSQT[pt[sq] - 1][sq]
                      : -(PIECE_VALUE[-pt[sq] - 1] + PSQT[-pt[sq] - 1][((63 - sq) & ~7) | (sq & 7)]);
}

ull Evaluator::getSquareHash(int sq) {
    if (!pt[sq]) return 0;
    return zobristHash[pt[sq] > 0 ? ((pt[sq] - 1) << 6) | sq
                                  : ((6 - pt[sq] - 1) << 6) | sq];
}

int Evaluator::getRawSquareVal(int sq, int pt) {
    if (!pt) return 0;
    return pt > 0 ? PIECE_VALUE[pt - 1] + PSQT[pt - 1][sq]
                  : -(PIECE_VALUE[-pt - 1] + PSQT[-pt - 1][((63 - sq) & ~7) | (sq & 7)]);
}

void Evaluator::reset() {
    eval = 0;
    zobrist = 0;
    for (int i = 0; i < 64; ++i) {
        if (!pt[i]) continue;
        eval += getSquareVal(i);
        zobrist ^= getSquareHash(i);
    }
}

void Evaluator::update(Move m) {
    int from = m.from();
    int to = m.to();
    int type = m.type();
    if (type == REGULAR) {
        eval -= getSquareVal(from) + getSquareVal(to);
        zobrist ^= getSquareHash(from) ^ getSquareHash(to);
        pt[to] = pt[from];
        pt[from] = 0;
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    } else if (type == CASTLE) {
        eval -= getSquareVal(from);
        zobrist ^= getSquareHash(from);
        pt[to] = pt[from];
        pt[from] = 0;
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
        if (from - to == 2) {
            eval -= getSquareVal(to - 2);
            zobrist ^= getSquareHash(to - 2);
            pt[to + 1] = pt[to - 2];
            pt[to - 2] = 0;
            eval += getSquareVal(to + 1);
            zobrist ^= getSquareHash(to + 1);
        } else {
            eval -= getSquareVal(to + 1);
            zobrist ^= getSquareHash(to + 1);
            pt[to - 1] = pt[to + 1];
            pt[to + 1] = 0;
            eval += getSquareVal(to - 1);
            zobrist ^= getSquareHash(to - 1);
        }
    } else if (type == PROMOTION) {
        eval -= getSquareVal(from) + getSquareVal(to);
        zobrist ^= getSquareHash(from) ^ getSquareHash(to);
        int promotion = m.promotion();
        pt[to] = pt[from] & 32 ? (-promotion - 1) : (promotion + 1);
        pt[from] = 0;
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    } else {
        eval -= getSquareVal(from) + getSquareVal(to);
        zobrist ^= getSquareHash(from) ^ getSquareHash(to);
        pt[to] = 0;
        to += to & 32 ? 8 : -8;
        pt[to] = pt[from];
        pt[from] = 0;
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    }
}

void Evaluator::print() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << pt[(i << 3) + j] << ' ';
        }
        std::cout << std::endl;
    }
}

int Evaluator::getMoveStrength(Move m, Color col) {
    int ans = 0;
    int from = m.from();
    int to = m.to();
    int type = m.type();
    if (type == REGULAR) {
        ans -= getSquareVal(from) + getSquareVal(to);
        ans += getRawSquareVal(to, pt[from]);
    } else if (type == CASTLE) {
        ans -= getSquareVal(from);
        ans += getRawSquareVal(to, pt[from]);
        if (from - to == 2) {
            ans -= getSquareVal(to - 2);
            ans += getRawSquareVal(to + 1, pt[to - 2]);
        } else {
            ans -= getSquareVal(to + 1);
            ans += getRawSquareVal(to - 1, pt[to + 1]);
        }
    } else if (type == PROMOTION) {
        ans -= getSquareVal(from) + getSquareVal(to);
        int promotion = m.promotion();
        ans += getRawSquareVal(to, pt[from] & 32 ? (-promotion - 1) : (promotion + 1));
    } else {
        ans -= getSquareVal(from) + getSquareVal(to);
        to += to & 32 ? 8 : -8;
        ans += getRawSquareVal(to, pt[from]);
    }
    return col ? -ans : ans;
}
