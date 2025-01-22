#include "board.hpp"
#include "extra.hpp"
#include "move.hpp"
#include "types.hpp"

void Board::recalculatePinned() {
    pinned = 0;
    int kingPos = lsb(pieces(col, KING));
    Color notCol = ~col;
    ull pinnerCandidates = (getRookMoves(kingPos, pieces(notCol)) & pieces(notCol, ROOK, QUEEN)) |
                           (getBishopMoves(kingPos, pieces(notCol)) & pieces(notCol, BISHOP, QUEEN));

    while (pinnerCandidates) {
        int current = extractLSB(pinnerCandidates);
        ull attackLine = getSegment(kingPos, current) & pieces(col);
        if (attackLine) {
            ull pinnedCandidate = extractPowerLSB(attackLine);
            if (!attackLine) pinned |= pinnedCandidate;
        }
    }
}

void Board::recalculateChecker() {
    ull attackers = getAttackers(lsb(pieces(col, KING)), pieces()) & pieces(~col);
    if (!attackers) {
        checker = NO_CHECK;
        return;
    }
    checker = extractLSB(attackers);
    if (attackers) checker = DOUBLE_CHECK;
}

ull Board::getAttackers(int pos, ull target) {
    return (pieces(WHITE, PAWN) & pawnAttacks[(pos << 1) + 1]) |
           (pieces(BLACK, PAWN) & pawnAttacks[(pos << 1)]) |
           (pieces(KNIGHT) & knightAttacks[pos]) |
           (pieces(KING) & kingAttacks[pos]) |
           (pieces(QUEEN, ROOK) & (getRookMoves(pos, target) ^ posToULL[pos])) |
           (pieces(QUEEN, BISHOP) & (getBishopMoves(pos, target) ^ posToULL[pos]));
}

template <Color color, KingSafety ks>
Move *Board::getPseudolegalPawnMoves(Move *mp) {
    constexpr Color notColor = ~color;
    constexpr ull thirdRank = (color == WHITE ? RANK_3 : RANK_6);
    constexpr ull sevenRank = (color == WHITE ? RANK_7 : RANK_2);
    constexpr ull notSevenRank = (color == WHITE ? NOT_RANK_7 : NOT_RANK_2);
    constexpr ShiftDirection up = (color == WHITE ? UP : DOWN);
    constexpr ShiftDirection leftup = (color == WHITE ? LEFTUP : RIGHTDOWN);
    constexpr ShiftDirection rightup = (color == WHITE ? RIGHTUP : LEFTDOWN);

    ull empty = ~pieces();
    ull enemies = pieces(notColor);

    ull pawnsRegular = pieces(color, PAWN) & notSevenRank;
    ull pawnsPromotion = pieces(color, PAWN) & sevenRank;

    // one and two step forward(no promotions)
    ull step1 = shift<up>(pawnsRegular) & empty;
    ull step2 = shift<up>(step1 & thirdRank) & empty;

    // check4check
    if constexpr (ks == IN_CHECK) {
        ull target = getSegment(lsb(pieces(color, KING)), checker);
        empty &= target;
        enemies &= target;
        step1 &= target;
        step2 &= target;
    }

    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - up, to);
    }
    while (step2) {
        int to = extractLSB(step2);
        *(mp++) = Move(to - (up << 1), to);
    }

    // captures(no promotions)
    step1 = shift<leftup>(pawnsRegular) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - leftup, to);
    }
    step1 = shift<rightup>(pawnsRegular) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - rightup, to);
    }

    // en passant
    if (en_passant != NO_ENPASSANT) {
        step1 = pawnsRegular & pawnAttacks[(en_passant << 1) + notColor];
        while (step1) {
            int from = extractLSB(step1);
            *(mp++) = Move(from, en_passant - up, EN_PASSANT); // place the new pawn on the place of the taken one
        }
    }

    // promotions

    step1 = shift<up>(pawnsPromotion) & empty;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - up, to, PROMOTION, QUEEN);
        *(mp++) = Move(to - up, to, PROMOTION, KNIGHT);
        *(mp++) = Move(to - up, to, PROMOTION, ROOK);
        *(mp++) = Move(to - up, to, PROMOTION, BISHOP);
    }

    step1 = shift<leftup>(pawnsPromotion) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - leftup, to, PROMOTION, QUEEN);
        *(mp++) = Move(to - leftup, to, PROMOTION, KNIGHT);
        *(mp++) = Move(to - leftup, to, PROMOTION, ROOK);
        *(mp++) = Move(to - leftup, to, PROMOTION, BISHOP);
    }

    step1 = shift<rightup>(pawnsPromotion) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - rightup, to, PROMOTION, QUEEN);
        *(mp++) = Move(to - rightup, to, PROMOTION, KNIGHT);
        *(mp++) = Move(to - rightup, to, PROMOTION, ROOK);
        *(mp++) = Move(to - rightup, to, PROMOTION, BISHOP);
    }

    return mp;
}

template <Color color, KingSafety ks>
Move *Board::getAggressivePseudolegalPawnMoves(Move *mp) {
    constexpr Color notColor = ~color;
    constexpr ull thirdRank = (color == WHITE ? RANK_3 : RANK_6);
    constexpr ull sevenRank = (color == WHITE ? RANK_7 : RANK_2);
    constexpr ull notSevenRank = (color == WHITE ? NOT_RANK_7 : NOT_RANK_2);
    constexpr ShiftDirection up = (color == WHITE ? UP : DOWN);
    constexpr ShiftDirection leftup = (color == WHITE ? LEFTUP : RIGHTDOWN);
    constexpr ShiftDirection rightup = (color == WHITE ? RIGHTUP : LEFTDOWN);

    ull empty = ~pieces();
    ull enemies = pieces(notColor);

    ull pawnsRegular = pieces(color, PAWN) & notSevenRank;
    ull pawnsPromotion = pieces(color, PAWN) & sevenRank;

    ull step1;

    // check4check
    if constexpr (ks == IN_CHECK) {
        // one and two step forward(no promotions)
        step1 = shift<up>(pawnsRegular) & empty;
        ull step2 = shift<up>(step1 & thirdRank) & empty;
        ull target = getSegment(lsb(pieces(color, KING)), checker);
        empty &= target;
        enemies &= target;
        step1 &= target;
        step2 &= target;
        while (step1) {
            int to = extractLSB(step1);
            *(mp++) = Move(to - up, to);
        }
        while (step2) {
            int to = extractLSB(step2);
            *(mp++) = Move(to - (up << 1), to);
        }
    }

    // captures(no promotions)
    step1 = shift<leftup>(pawnsRegular) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - leftup, to);
    }
    step1 = shift<rightup>(pawnsRegular) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - rightup, to);
    }

    // en passant
    if (en_passant != NO_ENPASSANT) {
        step1 = pawnsRegular & pawnAttacks[(en_passant << 1) + notColor];
        while (step1) {
            int from = extractLSB(step1);
            *(mp++) = Move(from, en_passant - up, EN_PASSANT); // place the new pawn on the place of the taken one
        }
    }

    // promotions

    step1 = shift<leftup>(pawnsPromotion) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - leftup, to, PROMOTION, QUEEN);
        *(mp++) = Move(to - leftup, to, PROMOTION, KNIGHT);
        *(mp++) = Move(to - leftup, to, PROMOTION, ROOK);
        *(mp++) = Move(to - leftup, to, PROMOTION, BISHOP);
    }

    step1 = shift<rightup>(pawnsPromotion) & enemies;
    while (step1) {
        int to = extractLSB(step1);
        *(mp++) = Move(to - rightup, to, PROMOTION, QUEEN);
        *(mp++) = Move(to - rightup, to, PROMOTION, KNIGHT);
        *(mp++) = Move(to - rightup, to, PROMOTION, ROOK);
        *(mp++) = Move(to - rightup, to, PROMOTION, BISHOP);
    }

    return mp;
}

template <PieceType pt, KingSafety ks>
Move *Board::getPseudolegalMoves(Move *mp) { // not for a king or a pawn
    ull p = pieces(pt) & pieces(col);
    ull moves;
    while (p) {
        int from = extractLSB(p);
        moves = getPseudolegalMoves<pt>(from) & ~pieces(col);

        // check4check
        if constexpr (ks == IN_CHECK) {
            moves &= getSegment(lsb(pieces(col, KING)), checker);
        }

        while (moves) {
            *(mp++) = Move(from, extractLSB(moves));
        }
    }
    return mp;
}

template <PieceType pt, KingSafety ks>
Move *Board::getAggressivePseudolegalMoves(Move *mp) { // not for a king or a pawn
    Color notCol = ~col;
    ull p = pieces(pt) & pieces(col);
    ull moves;
    while (p) {
        int from = extractLSB(p);
        moves = getPseudolegalMoves<pt>(from) & pieces(notCol);

        // check4check
        if constexpr (ks == IN_CHECK) {
            moves &= getSegment(lsb(pieces(col, KING)), checker);
        }

        while (moves) {
            *(mp++) = Move(from, extractLSB(moves));
        }
    }
    return mp;
}

template <Color color, KingSafety ks>
Move *Board::getAllPseudolegalMoves(Move *mp) {

    // double check disables all but king moves
    if (checker != DOUBLE_CHECK) {
        mp = getPseudolegalPawnMoves<color, ks>(mp);
        mp = getPseudolegalMoves<BISHOP, ks>(mp);
        mp = getPseudolegalMoves<KNIGHT, ks>(mp);
        mp = getPseudolegalMoves<ROOK, ks>(mp);
        mp = getPseudolegalMoves<QUEEN, ks>(mp);
    }

    int kingPos = lsb(pieces(color, KING));
    ull kingMoves = getPseudolegalMoves<KING>(kingPos) & ~pieces(color);

    while (kingMoves) {
        *(mp++) = Move(kingPos, extractLSB(kingMoves));
    }

    // castles
    if constexpr (color == WHITE && ks == SAFE) {
        if (castling & WHITE_OOO) *(mp++) = Move(kingPos, kingPos - 2, CASTLE);
        if (castling & WHITE_OO) *(mp++) = Move(kingPos, kingPos + 2, CASTLE);
    }
    if constexpr (color == BLACK && ks == SAFE) {
        if (castling & BLACK_OOO) *(mp++) = Move(kingPos, kingPos - 2, CASTLE);
        if (castling & BLACK_OO) *(mp++) = Move(kingPos, kingPos + 2, CASTLE);
    }
    return mp;
}

// explicit instantiation
template Move *Board::getAllPseudolegalMoves<WHITE, SAFE>(Move *);
template Move *Board::getAllPseudolegalMoves<BLACK, SAFE>(Move *);
template Move *Board::getAllPseudolegalMoves<WHITE, IN_CHECK>(Move *);
template Move *Board::getAllPseudolegalMoves<BLACK, IN_CHECK>(Move *);

template <Color color, KingSafety ks>
Move *Board::getAggressivePseudolegalMoves(Move *mp) {

    constexpr Color notColor = ~color;

    // double check disables all but king moves
    if (checker != DOUBLE_CHECK) {
        mp = getAggressivePseudolegalPawnMoves<color, ks>(mp);
        mp = getAggressivePseudolegalMoves<BISHOP, ks>(mp);
        mp = getAggressivePseudolegalMoves<KNIGHT, ks>(mp);
        mp = getAggressivePseudolegalMoves<ROOK, ks>(mp);
        mp = getAggressivePseudolegalMoves<QUEEN, ks>(mp);
    }

    int kingPos = lsb(pieces(color, KING));
    ull kingMoves;
    if constexpr (ks == IN_CHECK) {
        kingMoves = getPseudolegalMoves<KING>(kingPos) & ~pieces(color);
    } else {
        kingMoves = getPseudolegalMoves<KING>(kingPos) & pieces(notColor);
    }

    while (kingMoves) {
        *(mp++) = Move(kingPos, extractLSB(kingMoves));
    }

    return mp;
}

// explicit instantiation
template Move *Board::getAggressivePseudolegalMoves<WHITE, SAFE>(Move *);
template Move *Board::getAggressivePseudolegalMoves<BLACK, SAFE>(Move *);
template Move *Board::getAggressivePseudolegalMoves<WHITE, IN_CHECK>(Move *);
template Move *Board::getAggressivePseudolegalMoves<BLACK, IN_CHECK>(Move *);

bool Board::isLegal(Move m) {
    int from = m.from();
    int to = m.to();
    int type = m.type();
    Color notCol = ~col;
    int kingPos = lsb(pieces(col, KING));

    // check whether castle is correct
    if (type == CASTLE) {
        int step = (to > from) ? 1 : -1;
        int pos = from + step;
        if ((pieces() & posToULL[pos]) || (getAttackers(pos, pieces()) & pieces(notCol))) return false;
        pos += step;
        if ((pieces() & posToULL[pos]) || (getAttackers(pos, pieces()) & pieces(notCol))) return false;
        if (step == -1) { // long castle
            pos += step;
            if (pieces() & posToULL[pos]) return false;
        }
        return true;
    }

    // manually check whether en passant does not lead to self check
    if (type == EN_PASSANT) {
        ull target = (pieces() ^ posToULL[from] ^ posToULL[to]) | posToULL[en_passant];
        return !((getRookMoves(kingPos, target) & pieces(notCol, ROOK, QUEEN)) |
                 (getBishopMoves(kingPos, target) & pieces(notCol, BISHOP, QUEEN)));
    }

    // check whether king move is legal
    if (from == kingPos) {
        return !(getAttackers(m.to(), pieces() ^ posToULL[from]) & pieces(notCol));
    }

    // check whether move did not lead to self check

    return !(pinned & posToULL[from]) || (getSegment(kingPos, to) & posToULL[from]) || (getSegment(kingPos, from) & posToULL[to]);
}

Move *Board::getMoves(Move *mp) {
    Move *endMove = col ? (isInCheck(checker) ? getAllPseudolegalMoves<BLACK, IN_CHECK>(mp)
                                              : getAllPseudolegalMoves<BLACK, SAFE>(mp))
                        : (isInCheck(checker) ? getAllPseudolegalMoves<WHITE, IN_CHECK>(mp)
                                              : getAllPseudolegalMoves<WHITE, SAFE>(mp));
    Move *ans = mp;
    for (; mp < endMove; ++mp) {
        if (isLegal(*mp)) *(ans++) = *mp;
    }
    return ans;
}

Move *Board::getAggressiveMoves(Move *mp) {
    Move *endMove = col ? (isInCheck(checker) ? getAggressivePseudolegalMoves<BLACK, IN_CHECK>(mp)
                                              : getAggressivePseudolegalMoves<BLACK, SAFE>(mp))
                        : (isInCheck(checker) ? getAggressivePseudolegalMoves<WHITE, IN_CHECK>(mp)
                                              : getAggressivePseudolegalMoves<WHITE, SAFE>(mp));
    Move *ans = mp;
    for (; mp < endMove; ++mp) {
        if (isLegal(*mp)) *(ans++) = *mp;
    }
    return ans;
}

void Board::applyMove(Move m, Board *b) const {
    memcpy(b, this, sizeof(Board));

    int from = m.from();
    int to = m.to();
    int type = m.type();
    Color notCol = ~col;

    ull fromULL = posToULL[from];
    ull toULL = posToULL[to];

    b->col = notCol;
    b->fiftyMoveTimer++;
    b->en_passant = NO_ENPASSANT;

    // castling
    if (type == CASTLE) {
        b->castling &= ~(3 << (2 * col));
        ull rookMove = ((to > from) ? posToULL[to - 1] | posToULL[to + 1] : posToULL[to + 1] | posToULL[to - 2]);
        ull kingMove = toULL | fromULL;

        b->byType[ROOK] ^= rookMove;
        b->byType[KING] ^= kingMove;
        b->byCol[col] ^= rookMove ^ kingMove;
        b->all ^= rookMove ^ kingMove;
        b->recalculatePinned();
        b->recalculateChecker();
        return;
    }

    // en passant
    if (type == EN_PASSANT) {
        ull pawnMove = fromULL | posToULL[en_passant];
        ull capturedPawn = toULL;
        b->byCol[col] ^= pawnMove;
        b->byCol[notCol] ^= capturedPawn;
        b->byType[PAWN] ^= pawnMove ^ capturedPawn;
        b->all ^= pawnMove ^ capturedPawn;
        b->fiftyMoveTimer = 0;
        b->recalculatePinned();
        b->recalculateChecker();
        return;
    }

    // handle regular piece movement
    ull pieceMove = fromULL | toULL;
    int pt = ROOK;
    while (!(b->byType[pt] & fromULL)) ++pt;

    b->byCol[col] ^= pieceMove;

    // handle captures
    if (b->byCol[notCol] & toULL) {
        for (int p = ROOK; p < KING; ++p) {
            b->byType[p] &= ~toULL;
        }
        b->byCol[notCol] ^= toULL;
        b->all ^= toULL;
        b->fiftyMoveTimer = 0;
    }

    b->byType[pt] ^= pieceMove;
    b->all ^= pieceMove;

    // handle pawn moves
    if (pt == PAWN) {
        b->fiftyMoveTimer = 0;
        if (abs(to - from) == 16) {
            b->en_passant = (from + to) / 2;
        }
    }

    // update castling rights
    if (pt == KING) {
        b->castling &= ~(3 << (2 * col));
    }
    if (from == (col ? 0 : 56)) b->castling &= ~(2 << (2 * col));
    if (from == (col ? 7 : 63)) b->castling &= ~(1 << (2 * col));

    if (to == (col ? 56 : 0)) b->castling &= ~(2 << (2 * (1 - col)));
    if (to == (col ? 63 : 7)) b->castling &= ~(1 << (2 * (1 - col)));

    // promotion
    if (type == PROMOTION) {
        b->byType[PAWN] ^= toULL;
        b->byType[m.promotion()] ^= toULL;
    }

    b->recalculatePinned();
    b->recalculateChecker();
}

// parsing

Move Board::parseStrToMove(std::string moveStr) {
    int from = (moveStr[0] - 'a') + (('8' - moveStr[1]) << 3);
    int to = (moveStr[2] - 'a') + (('8' - moveStr[3]) << 3);
    int type = 0;
    int promotion = 0;
    // en passant check
    if (en_passant == to && (byType[PAWN] & posToULL[from])) {
        to += (col ? -8 : 8);
        type = EN_PASSANT;
    }
    // castling check
    int kingPos = lsb(pieces(col, KING));
    if (from == kingPos && abs(from - to) == 2) type = CASTLE;

    // promotion check
    if (moveStr.size() > 4) {
        type = PROMOTION;
        promotion = (moveStr[4] == 'q'   ? QUEEN
                     : moveStr[4] == 'n' ? KNIGHT
                     : moveStr[4] == 'r' ? ROOK
                                         : BISHOP);
    }

    return Move(from, to, type, promotion);
}

std::string Board::parseMoveToStr(Move move) {
    if (move == 0) return "invalid";
    std::string ans;
    ans += (char)('a' + (move.from() & 7));
    ans += (char)('8' - (move.from() >> 3));
    ans += (char)('a' + (move.to() & 7));
    if (move.type() == EN_PASSANT)
        ans += (char)((col ? '7' : '9') - (move.to() >> 3));
    else
        ans += (char)('8' - (move.to() >> 3));
    if (move.type() == PROMOTION) {
        int promotion = move.promotion();
        ans += promotion == QUEEN    ? 'q'
               : promotion == KNIGHT ? 'n'
               : promotion == ROOK   ? 'r'
                                     : 'b';
    }
    return ans;
}

void Board::setPT(int *pt) {
    memset(pt, 0, 64 * sizeof(int));
    ull mask;
    int bit;
    for (int j = 0; j < 6; ++j) {
        mask = byCol[0] & byType[j];
        while (mask) {
            bit = extractLSB(mask);
            pt[bit] = j + 1;
        }
        mask = byCol[1] & byType[j];
        while (mask) {
            bit = extractLSB(mask);
            pt[bit] = -j - 1;
        }
    }
}

Board::Board(std::string s) {
    int pos = 0;
    int boardPos = 0;
    while (s[pos] != ' ') {
        if (s[pos] == '/') {
            ++pos;
            continue;
        } else {
            if (s[pos] <= '9') {
                boardPos += s[pos] - '0';
                ++pos;
                continue;
            } else {
                byCol[s[pos] < 'Z' ? WHITE : BLACK] |= posToULL[boardPos];
                byType[(s[pos] | 32) == 'r'   ? ROOK
                       : (s[pos] | 32) == 'n' ? KNIGHT
                       : (s[pos] | 32) == 'b' ? BISHOP
                       : (s[pos] | 32) == 'q' ? QUEEN
                       : (s[pos] | 32) == 'p' ? PAWN
                                              : KING] |= posToULL[boardPos];
                all |= posToULL[boardPos];
            }
        }
        ++boardPos;
        ++pos;
    }
    ++pos;
    col = s[pos] == 'b' ? BLACK : WHITE;
    pos += 2;
    while (s[pos] != ' ') {
        castling |= s[pos] == 'K'   ? WHITE_OO
                    : s[pos] == 'k' ? BLACK_OO
                    : s[pos] == 'Q' ? WHITE_OOO
                                    : BLACK_OOO;
        ++pos;
    }
    ++pos;
    if (s[pos] == '-')
        en_passant = 64;
    else {
        en_passant = (s[pos] - 'a') + (('8' - s[pos + 1]) << 3);
        ++pos;
    }
    pos += 2;
    // only for openings
    fiftyMoveTimer = s[pos] - '0';
    recalculatePinned();
    recalculateChecker();
}

ull Board::getExtraZobrist() {
    ull zobrist = zobristHash[768 + castling];
    if (col) zobrist ^= zobristHash[792];
    if (en_passant != NO_ENPASSANT) zobrist ^= zobristHash[784 + (en_passant & 7)];
    return zobrist;
}
