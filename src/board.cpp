#include "board.hpp"
#include "evaluator.hpp"
#include "extra.hpp"
#include "move.hpp"
#include "types.hpp"

void Board::recalculatePinned(Color col) {
    pinned[col] = 0;
    pinner[col] = 0;
    int kingPos = lsb(pieces(col, KING));
    Color notCol = ~col;
    ull pinnerCandidates = (getRookMoves(kingPos, pieces(notCol)) & pieces(notCol, ROOK, QUEEN)) |
                           (getBishopMoves(kingPos, pieces(notCol)) & pieces(notCol, BISHOP, QUEEN));

    while (pinnerCandidates) {
        int current = extractLSB(pinnerCandidates);
        ull attackLine = getSegment(kingPos, current) & pieces(col);
        if (attackLine) {
            ull pinnedCandidate = extractPowerLSB(attackLine);
            if (!attackLine) {
                pinned[col] |= pinnedCandidate;
                pinner[col] |= posToULL[current];
            }
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

ull Board::getAttackers(int pos, ull target) const {
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
Move *Board::getCapturePseudolegalPawnMoves(Move *mp) {
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
Move *Board::getCapturePseudolegalMoves(Move *mp) { // not for a king or a pawn
    Color notCol = ~col;
    ull p = pieces(pt) & pieces(col);
    ull moves;
    while (p) {
        int from = extractLSB(p);
        moves = getPseudolegalMoves<pt>(from) & pieces(notCol);

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
Move *Board::getCapturePseudolegalMoves(Move *mp) {
    assert(ks == SAFE);
    constexpr Color notColor = ~color;

    // double check disables all but king moves
    if (checker != DOUBLE_CHECK) {
        mp = getCapturePseudolegalPawnMoves<color, ks>(mp);
        mp = getCapturePseudolegalMoves<BISHOP, ks>(mp);
        mp = getCapturePseudolegalMoves<KNIGHT, ks>(mp);
        mp = getCapturePseudolegalMoves<ROOK, ks>(mp);
        mp = getCapturePseudolegalMoves<QUEEN, ks>(mp);
    }

    int kingPos = lsb(pieces(color, KING));
    ull kingMoves;
    kingMoves = getPseudolegalMoves<KING>(kingPos) & pieces(notColor);

    while (kingMoves) {
        *(mp++) = Move(kingPos, extractLSB(kingMoves));
    }

    return mp;
}

// explicit instantiation
template Move *Board::getCapturePseudolegalMoves<WHITE, SAFE>(Move *);
template Move *Board::getCapturePseudolegalMoves<BLACK, SAFE>(Move *);
template Move *Board::getCapturePseudolegalMoves<WHITE, IN_CHECK>(Move *);
template Move *Board::getCapturePseudolegalMoves<BLACK, IN_CHECK>(Move *);

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

    return !(pinned[col] & posToULL[from]) || (getSegment(kingPos, to) & posToULL[from]) || (getSegment(kingPos, from) & posToULL[to]);
}

Move *Board::getMoves(Move *mp) {
    Move *endMove = col ? (isInCheck(checker) ? getAllPseudolegalMoves<BLACK, IN_CHECK>(mp)
                                              : getAllPseudolegalMoves<BLACK, SAFE>(mp))
                        : (isInCheck(checker) ? getAllPseudolegalMoves<WHITE, IN_CHECK>(mp)
                                              : getAllPseudolegalMoves<WHITE, SAFE>(mp));
    Move *ans = mp;
    finished = 1;
    for (; mp < endMove; ++mp) {
        if (isLegal(*mp)) {
            finished = 0;
            if (checkSEE(*mp, -200)) *(ans++) = *mp;
        }
    }
    return ans;
}

Move *Board::getCaptureMoves(Move *mp) {
    assert(!finished);
    Move *endMove = col ? (isInCheck(checker) ? getCapturePseudolegalMoves<BLACK, IN_CHECK>(mp)
                                              : getCapturePseudolegalMoves<BLACK, SAFE>(mp))
                        : (isInCheck(checker) ? getCapturePseudolegalMoves<WHITE, IN_CHECK>(mp)
                                              : getCapturePseudolegalMoves<WHITE, SAFE>(mp));
    Move *ans = mp;
    finished = 1;
    for (; mp < endMove; ++mp) {
        if (isLegal(*mp)) {
            finished = 0;
            if (checkSEE(*mp, -100)) *(ans++) = *mp;
        }
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
        ull rookMove;
        if (to > from) {
            rookMove = posToULL[to - 1] | posToULL[to + 1];
            b->pieceAt[to - 1] = pieceAt[to + 1];
            b->pieceAt[to + 1] = 0;
        } else {
            rookMove = posToULL[to + 1] | posToULL[to - 2];
            b->pieceAt[to + 1] = pieceAt[to - 2];
            b->pieceAt[to - 2] = 0;
        }

        ull kingMove = toULL | fromULL;
        b->pieceAt[to] = pieceAt[from];
        b->pieceAt[from] = 0;

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

        b->pieceAt[to] = 0;
        b->pieceAt[to + (col ? 8 : -8)] = pieceAt[from];
        b->pieceAt[from] = 0;

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
    int ptFrom = pieceAt[from];

    ptFrom -= (ptFrom > 6 ? 7 : 1);

    b->byCol[col] ^= pieceMove;

    // handle captures
    if (b->byCol[notCol] & toULL) {
        int ptTo = pieceAt[to];
        ptTo -= (ptTo > 6 ? 7 : 1);
        b->byType[ptTo] ^= toULL;
        b->byCol[notCol] ^= toULL;
        b->all ^= toULL;
        b->fiftyMoveTimer = 0;
    }

    b->byType[ptFrom] ^= pieceMove;
    b->all ^= pieceMove;

    // handle pawn moves
    if (ptFrom == PAWN) {
        b->fiftyMoveTimer = 0;
        if (abs(to - from) == 16) {
            b->en_passant = (from + to) / 2;
        }
    }

    // update castling rights
    if (ptFrom == KING) {
        b->castling &= ~(3 << (2 * col));
    }
    if (from == (col ? 0 : 56)) b->castling &= ~(2 << (2 * col));
    if (from == (col ? 7 : 63)) b->castling &= ~(1 << (2 * col));

    if (to == (col ? 56 : 0)) b->castling &= ~(2 << (2 * (1 - col)));
    if (to == (col ? 63 : 7)) b->castling &= ~(1 << (2 * (1 - col)));

    b->pieceAt[to] = pieceAt[from];
    b->pieceAt[from] = 0;

    // promotion
    if (type == PROMOTION) {
        b->byType[PAWN] ^= toULL;
        b->byType[m.promotion()] ^= toULL;
        b->pieceAt[to] = m.promotion() + (b->pieceAt[to] > 6 ? 7 : 1);
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

std::string Board::parseMoveToStr(Move move) const {
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
    memcpy(pt, pieceAt, 64 * sizeof(int));
}

Board::Board(std::string s) {
    int pos = 0;
    int boardPos = 0;
    memset(byCol, 0, sizeof(byCol));
    memset(byType, 0, sizeof(byType));
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
                    : s[pos] == 'q' ? BLACK_OOO
                                    : 0;
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
    recalculatePieceAt();
}

// from cfish
bool Board::checkSEE(Move m, int val) const {

    int from = m.from(), to = m.to();
    ull occ;
    if (!pieceAt[to]) return 0 >= val;

    int swap = PIECE_VALUE[pieceAt[to] - (pieceAt[to] > 6 ? 7 : 1)] - val;
    if (swap < 0)
        return false;

    swap = PIECE_VALUE[pieceAt[from] - (pieceAt[from] > 6 ? 7 : 1)] - swap;
    if (swap <= 0)
        return true;

    occ = pieces() ^ posToULL[from] ^ posToULL[to];
    Color curCol = pieceAt[from] > 6 ? BLACK : WHITE;
    ull attackers = getAttackers(to, occ), curAttackers;
    bool res = true;

    while (true) {
        curCol = ~curCol;
        attackers &= occ;
        if (!(curAttackers = attackers & byCol[curCol])) break;
        if ((curAttackers & pinned[curCol]) && (pinner[curCol] & occ))
            curAttackers &= ~pinned[curCol];
        if (!curAttackers) break;
        res = !res;
        ull bb;
        if ((bb = curAttackers & byType[PAWN])) {
            if ((swap = PIECE_VALUE[PAWN] - swap) < res) break;
            occ ^= bb & -bb;
            attackers |= getBishopMoves(to, occ) & pieces(BISHOP, QUEEN);
        } else if ((bb = curAttackers & byType[KNIGHT])) {
            if ((swap = PIECE_VALUE[KNIGHT] - swap) < res) break;
            occ ^= bb & -bb;
        } else if ((bb = curAttackers & byType[BISHOP])) {
            if ((swap = PIECE_VALUE[BISHOP] - swap) < res) break;
            occ ^= bb & -bb;
            attackers |= getBishopMoves(to, occ) & pieces(BISHOP, QUEEN);
        } else if ((bb = curAttackers & byType[ROOK])) {
            if ((swap = PIECE_VALUE[ROOK] - swap) < res) break;
            occ ^= bb & -bb;
            attackers |= getRookMoves(to, occ) & pieces(ROOK, QUEEN);
        } else if ((bb = curAttackers & byType[QUEEN])) {
            if ((swap = PIECE_VALUE[QUEEN] - swap) < res) break;
            occ ^= bb & -bb;
            attackers |= (getBishopMoves(to, occ) & pieces(BISHOP, QUEEN)) | (getRookMoves(to, occ) & pieces(ROOK, QUEEN));
        } else // KING
            return (attackers & ~byCol[curCol]) ? !res : res;
    }

    return res;
}

ull Board::getExtraZobrist() {
    ull zobrist = zobristHash[768 + castling];
    if (col) zobrist ^= zobristHash[792];
    if (en_passant != NO_ENPASSANT) zobrist ^= zobristHash[784 + (en_passant & 7)];
    return zobrist;
}

void Board::recalculatePieceAt() {
    memset(pieceAt, 0, 64 * sizeof(uint32_t));
    ull mask;
    int bit;
    for (int j = 0; j < 6; ++j) {
        mask = byCol[0] & byType[j];
        while (mask) {
            bit = extractLSB(mask);
            pieceAt[bit] = j + 1;
        }
        mask = byCol[1] & byType[j];
        while (mask) {
            bit = extractLSB(mask);
            pieceAt[bit] = j + 7;
        }
    }
}

void Board::print() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            std::cout << pieceAt[j + (i << 3)] << ' ';
        }
        std::cout << std::endl;
    }
}
