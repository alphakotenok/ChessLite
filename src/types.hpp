#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>

typedef unsigned long long ull;

enum Color : uint8_t {
    WHITE,
    BLACK,
};

constexpr Color operator~(Color c) { return Color(c ^ BLACK); }

enum CastlingRights : uint8_t {
    NO_CASTLING,
    WHITE_OO,
    WHITE_OOO = WHITE_OO << 1,
    BLACK_OO = WHITE_OO << 2,
    BLACK_OOO = WHITE_OO << 3,
    ANY_WHITE = WHITE_OO | WHITE_OOO,
    ANY_BLACK = BLACK_OO | BLACK_OOO,
    ANY_OO = WHITE_OO | BLACK_OO,
    ANY_OOO = WHITE_OOO | BLACK_OOO,
};

enum Direction {
    HORIZONTAL,
    RIGHTUP_DIAGONAL,
    VERTICAL,
    LEFTUP_DIAGONAL
};

enum ShiftDirection {
    UP = -8,
    DOWN = 8,
    LEFT = -1,
    RIGHT = 1,
    LEFTUP = -9,
    RIGHTUP = -7,
    LEFTDOWN = 7,
    RIGHTDOWN = 9
};

enum Line : ull {
    RANK_1 = 0xff00000000000000ull,
    RANK_2 = 0xff000000000000ull,
    RANK_3 = 0xff0000000000ull,
    RANK_4 = 0xff00000000ull,
    RANK_5 = 0xff000000ull,
    RANK_6 = 0xff0000ull,
    RANK_7 = 0xff00ull,
    RANK_8 = 0xffull,
    FILE_A = 0x0101010101010101ull,
    FILE_B = 0x0202020202020202ull,
    FILE_C = 0x0404040404040404ull,
    FILE_D = 0x0808080808080808ull,
    FILE_E = 0x1010101010101010ull,
    FILE_F = 0x2020202020202020ull,
    FILE_G = 0x4040404040404040ull,
    FILE_H = 0x8080808080808080ull,
    NOT_RANK_1 = ~RANK_1,
    NOT_RANK_2 = ~RANK_2,
    NOT_RANK_3 = ~RANK_3,
    NOT_RANK_4 = ~RANK_4,
    NOT_RANK_5 = ~RANK_5,
    NOT_RANK_6 = ~RANK_6,
    NOT_RANK_7 = ~RANK_7,
    NOT_RANK_8 = ~RANK_8,
    NOT_FILE_A = ~FILE_A,
    NOT_FILE_B = ~FILE_B,
    NOT_FILE_C = ~FILE_C,
    NOT_FILE_D = ~FILE_D,
    NOT_FILE_E = ~FILE_E,
    NOT_FILE_F = ~FILE_F,
    NOT_FILE_G = ~FILE_G,
    NOT_FILE_H = ~FILE_H
};

template <ShiftDirection sd>
constexpr ull shift(ull target) {
    if constexpr (sd == UP || sd == LEFTUP || sd == RIGHTUP) target &= NOT_RANK_8;
    if constexpr (sd == DOWN || sd == LEFTDOWN || sd == RIGHTDOWN) target &= NOT_RANK_1;
    if constexpr (sd == RIGHT || sd == RIGHTUP || sd == RIGHTDOWN) target &= NOT_FILE_H;
    if constexpr (sd == LEFT || sd == LEFTUP || sd == LEFTDOWN) target &= NOT_FILE_A;
    return sd > 0 ? (target << sd) : (target >> -sd);
}

enum PieceType {
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    PAWN,
    KING
};

enum MoveType {
    REGULAR,
    CASTLE = 1 << 12,
    EN_PASSANT = 2 << 12,
    PROMOTION = 3 << 12
};

enum KingSafety {
    SAFE,
    IN_CHECK
};

const int8_t NO_ENPASSANT = 64;
const int8_t DOUBLE_CHECK = 64;
const int8_t NO_CHECK = 65;
inline KingSafety isInCheck(uint8_t checker) { return (checker == NO_CHECK ? SAFE : IN_CHECK); }

class Board;
class Move;
class Node;
class MoveTree;
class Worker;
class BoardData;
class DataStack;
class BoardData;
class Evaluator;
class TranspositionTable;

enum Side {
    US,
    NOT_US
};

enum NodeType {
    COMMON,
    CHECKMATE,
    STALEMATE,
    PRUNED,
    LINKED
};

const int WORST_EVAL = -999999999;
const int PERFECT_EVAL = -WORST_EVAL;
const int MATED = -99999999;
const uint32_t MAX_MOVE_SIZE = 128;

enum DataType {
    MOVETREE_CACHED = 1,
    ACTIVE_STATE = MOVETREE_CACHED << 1,
    QUIESCENCE = MOVETREE_CACHED << 2
};

const ull SEVENTEEN_BITS = (1ull << 17) - 1;
const ull FIRST_SEVENTEEN_RESET = ~SEVENTEEN_BITS;
const ull SECOND_SEVENTEEN_RESET = ~(SEVENTEEN_BITS << 17);
const ull THIRD_SEVENTEEN_RESET = ~(SEVENTEEN_BITS << 34);

enum EvalType {
    EXACT,
    ALPHA,
    BETA
};

const uint32_t DATA_STACK_SIZE = 32;
const uint32_t REPETITION_TABLE_SIZE = 1 << 14;
