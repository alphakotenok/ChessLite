#include "extra.hpp"
#include "types.hpp"

ull sliderAttacks[16384];
ull dirMask[256];
ull dirMultiplier[256];

void initSliders() {
    int dirPos[4];
    for (int i = 0; i < 64; ++i) {

        // init masks

        int f = i & 7;
        int r = i >> 3;
        for (int j = 0; j <= 7; ++j) dirMask[i << 2] |= 1ull << ((r << 3) | j);
        for (int j = -min(r, 7 - f); j <= min(7 - r, f); ++j) dirMask[(i << 2) | 1] |= 1ull << (((r + j) << 3) | (f - j));
        for (int j = 0; j <= 7; ++j) dirMask[(i << 2) | 2] |= 1ull << ((j << 3) | f);
        for (int j = -min(r, f); j <= min(7 - r, 7 - f); ++j) dirMask[(i << 2) | 3] |= 1ull << (((r + j) << 3) | (f + j));

        for (int k = 0; k < 4; ++k) {

            // init shift and multipliers

            ull temp = dirMask[(i << 2) + k];
            extractMSB(temp);
            for (int j = 63; temp; --j)
                dirMultiplier[(i << 2) + k] |= 1ull << (j - extractMSB(temp));

            // init Pos

            temp = dirMask[(i << 2) + k];
            for (int j = 7; temp; --j)
                if (i == extractMSB(temp))
                    dirPos[k] = j;
        }

        // init sliderAttacks

        for (int j = 0; j < 256; ++j) {
            int index = (j & 3) | (i << 2) | ((j & 252) << 6);
            ull line = ((j & 252) >> 1) | 128;
            int type = j & 3;
            int pos = dirPos[type];
            int lineAttack = 0;
            while (line) {
                int curPos = extractLSB(line);
                if (curPos < pos) {
                    lineAttack |= (1 << curPos) - 1;
                }
                if (curPos > pos || curPos == 7) {
                    lineAttack ^= (1 << (curPos + 1)) - 1;
                    break;
                }
            }

            ull temp = dirMask[(i << 2) | type];
            for (int k = 7; temp; --k) {
                if (lineAttack & 1 << k)
                    sliderAttacks[index] |= extractPowerMSB(temp);
                else
                    removeMSB(temp);
            }
        }
    }
}

ull segment[4096];

void initSegments() {
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 64; ++j) {
            segment[i | (j << 6)] = 1ull << i;
        }
    }
    for (int i = 0; i < 64; ++i) {
        for (int dirR = -1; dirR <= 1; ++dirR) {
            for (int dirF = -1; dirF <= 1; ++dirF) {
                if (dirR == 0 && dirF == 0) continue;
                int r = i >> 3;
                int f = i & 7;
                for (int j = 2; r + dirR * j < 8 && r + dirR * j >= 0 && f + dirF * j < 8 && f + dirF * j >= 0; ++j) {
                    segment[(i << 6) | ((r + dirR * j) << 3 | (f + dirF * j))] |=
                        segment[(i << 6) | ((r + dirR * (j - 1)) << 3 | (f + dirF * (j - 1)))];
                }
            }
        }
    }
}

ull knightAttacks[64];
ull kingAttacks[64];
ull pawnAttacks[128];

void initAttacks() {
    for (int i = 0; i < 64; ++i) {
        ull pos = 1ull << i;

        // knight
        knightAttacks[i] = ((pos & ~(FILE_A | FILE_B | RANK_1)) << 6) |
                           ((pos & ~(FILE_A | RANK_1 | RANK_2)) << 15) |
                           ((pos & ~(FILE_H | RANK_1 | RANK_2)) << 17) |
                           ((pos & ~(FILE_G | FILE_H | RANK_1)) << 10) |
                           ((pos & ~(FILE_G | FILE_H | RANK_8)) >> 6) |
                           ((pos & ~(FILE_H | RANK_7 | RANK_8)) >> 15) |
                           ((pos & ~(FILE_A | RANK_7 | RANK_8)) >> 17) |
                           ((pos & ~(FILE_A | FILE_B | RANK_8)) >> 10);

        // king
        kingAttacks[i] = ((pos & ~(FILE_A | RANK_1)) << 7) |
                         ((pos & ~RANK_1) << 8) |
                         ((pos & ~(FILE_H | RANK_1)) << 9) |
                         ((pos & ~FILE_A) >> 1) |
                         ((pos & ~FILE_H) << 1) |
                         ((pos & ~(FILE_A | RANK_8)) >> 9) |
                         ((pos & ~RANK_8) >> 8) |
                         ((pos & ~(FILE_H | RANK_8)) >> 7);

        // pawn
        pawnAttacks[i << 1] = ((pos & ~FILE_H) >> 7) |
                              ((pos & ~FILE_A) >> 9);
        pawnAttacks[(i << 1) | 1] = ((pos & ~FILE_A) << 7) |
                                    ((pos & ~FILE_H) << 9);
    }
}

ull posToULL[64];

void initConverter() {
    for (int i = 0; i < 64; ++i) {
        posToULL[i] = 1ull << i;
    }
}

ull zobristHash[793]; // 8 kb

void initZobrist() {
    RandGen rand(1070372);
    for (int i = 0; i < 793; ++i) {
        zobristHash[i] = rand.get();
    }
}
