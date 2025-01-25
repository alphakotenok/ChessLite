#pragma once
#include "extra.hpp"
#include "types.hpp"

class Board {
private:
    ull byType[6];
    ull byCol[2];
    ull all;
    ull pinned[2];
    ull pinner[2];
    ull extraZobrist;
    uint32_t pieceAt[64];
    Color col;
    uint8_t fiftyMoveTimer;
    uint8_t castling;
    uint8_t en_passant; // 0-63 - position of en passant, 64 - no en passant
    uint8_t checker;    // 0-63 - position of checker, 64 - double check, 65 - no check
    bool finished;

    void recalculatePinned(Color col);
    inline void recalculatePinned() {
        recalculatePinned(WHITE);
        recalculatePinned(BLACK);
    }
    void recalculateChecker();
    void recalculatePieceAt();

public:
    Board() {}

    Board(char c) : all(0xffff00000000ffffull),
                    col(WHITE),
                    fiftyMoveTimer(0),
                    castling(15),
                    en_passant(64),
                    checker(65) {
        byType[PAWN] = 0xff00000000ff00ull;
        byType[KNIGHT] = 0x4200000000000042ull;
        byType[BISHOP] = 0x2400000000000024ull;
        byType[ROOK] = 0x8100000000000081ull;
        byType[QUEEN] = 0x800000000000008ull;
        byType[KING] = 0x1000000000000010ull;
        byCol[WHITE] = 0xffff000000000000ull;
        byCol[BLACK] = 0xffff;
        recalculatePinned();
        recalculateChecker();
    }

    Board(int idiotMate) : all(0xff9f00e01000eff7ull),
                           col(WHITE),
                           fiftyMoveTimer(1),
                           castling(15),
                           en_passant(64),
                           checker(65) {
        byType[PAWN] = 0x9f00601000ef00ull;
        byType[KNIGHT] = 0x4200000000000042ull;
        byType[BISHOP] = 0x2400000000000024ull;
        byType[ROOK] = 0x8100000000000081ull;
        byType[QUEEN] = 0x800008000000000ull;
        byType[KING] = 0x1000000000000010ull;
        byCol[WHITE] = 0xff9f006000000000ull;
        byCol[BLACK] = 0x801000eff7ull;
        recalculatePinned();
        recalculateChecker();
    }

    Board(ull *byType, ull *byColor, ull all, Color col, uint8_t fiftyMoveTimer,
          uint8_t castling, uint8_t en_passant, uint8_t checker) : all(all),
                                                                   col(col),
                                                                   fiftyMoveTimer(fiftyMoveTimer),
                                                                   castling(castling),
                                                                   en_passant(en_passant),
                                                                   checker(checker) {
        memcpy(this->byType, byType, sizeof(ull) * 6);
        memcpy(this->byCol, byColor, sizeof(ull) * 2);
        recalculatePinned();
        recalculateChecker();
    }

    Board(Board *b) {
        memcpy(this, b, sizeof(Board));
    }

    Board(std::string s);

    inline Color getCol() const { return col; };
    inline ull pieces() const { return all; }
    inline ull pieces(Color col) const { return byCol[col]; }
    inline ull pieces(PieceType pt) const { return byType[pt]; }
    template <typename... PieceTypes>
    inline ull pieces(PieceType pt, PieceTypes... pts) const { return pieces(pt) | pieces(pts...); }
    template <typename... PieceTypes>
    inline ull pieces(Color col, PieceTypes... pts) const { return pieces(col) & pieces(pts...); }

    template <PieceType>
    inline ull getPseudolegalMoves(int pos); // including self-captures
    template <>
    inline ull getPseudolegalMoves<ROOK>(int pos) { return getRookMoves(pos, pieces()); }
    template <>
    inline ull getPseudolegalMoves<BISHOP>(int pos) { return getBishopMoves(pos, pieces()); }
    template <>
    inline ull getPseudolegalMoves<QUEEN>(int pos) { return getRookMoves(pos, pieces()) | getBishopMoves(pos, pieces()); }
    template <>
    inline ull getPseudolegalMoves<KING>(int pos) { return kingAttacks[pos]; }
    template <>
    inline ull getPseudolegalMoves<KNIGHT>(int pos) { return knightAttacks[pos]; }

    ull getAttackers(int pos, ull target) const;

    template <Color, KingSafety>
    Move *getPseudolegalPawnMoves(Move *mp);

    template <Color, KingSafety>
    Move *getCapturePseudolegalPawnMoves(Move *mp);

    template <PieceType, KingSafety>
    Move *getPseudolegalMoves(Move *mp); // all but king and pawn

    template <PieceType, KingSafety>
    Move *getCapturePseudolegalMoves(Move *mp); // all but king and pawn

    template <Color, KingSafety>
    Move *getAllPseudolegalMoves(Move *mp);

    template <Color, KingSafety>
    Move *getCapturePseudolegalMoves(Move *mp);

    bool isLegal(Move m);
    Move *getMoves(Move *mp);
    Move *getCaptureMoves(Move *mp);
    void applyMove(Move m, Board *b) const;

    inline bool isChecked() { return checker != NO_CHECK; };
    inline bool isFinished() { return finished; }

    // parsing

    Move parseStrToMove(std::string moveStr);
    std::string parseMoveToStr(Move move) const;

    void setPT(int *pt);

    inline uint32_t getPieceAt(int pos) { return pieceAt[pos]; }
    bool checkSEE(Move m, int val) const;

    ull getExtraZobrist();

    void print();
};
