#include "evaluator.hpp"
#include "extra.hpp"
#include "types.hpp"

int Evaluator::getSquareVal(int sq) {
    if (!b->getPieceAt(sq)) return 0;
    return b->getPieceAt(sq) < 7 ? -PIECE_VALUE[b->getPieceAt(sq) - 1] + PSQT[b->getPieceAt(sq) - 1][sq]
                                 : -(PIECE_VALUE[b->getPieceAt(sq) - 7] + PSQT[b->getPieceAt(sq) - 7][((63 - sq) & ~7) | (sq & 7)]);
}

ull Evaluator::getSquareHash(int sq) {
    if (!b->getPieceAt(sq)) return 0;
    return zobristHash[((b->getPieceAt(sq) - 1) << 6) | sq];
}

int Evaluator::getRawSquareVal(int sq, int pt) {
    if (!pt) return 0;
    return pt < 7 ? PIECE_VALUE[pt - 1] + PSQT[pt - 1][sq]
                  : -(PIECE_VALUE[pt - 7] + PSQT[pt - 7][((63 - sq) & ~7) | (sq & 7)]);
}

void Evaluator::reset() {
    eval = 0;
    zobrist = 0;
    for (int i = 0; i < 64; ++i) {
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
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    } else if (type == CASTLE) {
        eval -= getSquareVal(from);
        zobrist ^= getSquareHash(from);
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
        if (from - to == 2) {
            eval -= getSquareVal(to - 2);
            zobrist ^= getSquareHash(to - 2);
            eval += getSquareVal(to + 1);
            zobrist ^= getSquareHash(to + 1);
        } else {
            eval -= getSquareVal(to + 1);
            zobrist ^= getSquareHash(to + 1);
            eval += getSquareVal(to - 1);
            zobrist ^= getSquareHash(to - 1);
        }
    } else if (type == PROMOTION) {
        eval -= getSquareVal(from) + getSquareVal(to);
        zobrist ^= getSquareHash(from) ^ getSquareHash(to);
        int promotion = m.promotion();
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    } else {
        eval -= getSquareVal(from) + getSquareVal(to);
        zobrist ^= getSquareHash(from) ^ getSquareHash(to);
        eval += getSquareVal(to);
        zobrist ^= getSquareHash(to);
    }
}

void Evaluator::print() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << b->getPieceAt((i << 3) + j) << ' ';
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
        ans += getRawSquareVal(to, b->getPieceAt(from));
    } else if (type == CASTLE) {
        ans -= getSquareVal(from);
        ans += getRawSquareVal(to, b->getPieceAt(from));
        if (from - to == 2) {
            ans -= getSquareVal(to - 2);
            ans += getRawSquareVal(to + 1, b->getPieceAt(to - 2));
        } else {
            ans -= getSquareVal(to + 1);
            ans += getRawSquareVal(to - 1, b->getPieceAt(to + 1));
        }
    } else if (type == PROMOTION) {
        ans -= getSquareVal(from) + getSquareVal(to);
        int promotion = m.promotion();
        ans += getRawSquareVal(to, b->getPieceAt(from) & 32 ? (promotion + 7) : (promotion + 1));
    } else {
        ans -= getSquareVal(from) + getSquareVal(to);
        to += to & 32 ? 8 : -8;
        ans += getRawSquareVal(to, b->getPieceAt(from));
    }
    return col ? -ans : ans;
}
