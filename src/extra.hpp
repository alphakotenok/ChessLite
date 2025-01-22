#pragma once
#include "types.hpp"
#include <sys/types.h>

// total size ~ 256 kb

// useful tools

inline ull upShift(ull target) { return target >> 8; }
inline ull downShift(ull target) { return target << 8; }
inline ull leftShift(ull target) { return target >> 1; }
inline ull rightShift(ull target) { return target << 1; }

inline int lsb(ull target) { return __builtin_ctzll(target); }

inline int msb(ull target) { return 63 ^ __builtin_clzll(target); }

inline ull extractPowerLSB(ull &target) {
    ull ans = target & -target;
    target ^= ans;
    return ans;
}

inline ull extractPowerMSB(ull &target) {
    ull ans = 1ull << msb(target);
    target ^= ans;
    return ans;
}

inline int extractLSB(ull &target) {
    int ans = lsb(target);
    target &= target - 1;
    return ans;
}

inline int extractMSB(ull &target) {
    int ans = msb(target);
    target ^= 1ull << ans;
    return ans;
}

inline void removeLSB(ull &target) { target &= target - 1; }

inline void removeMSB(ull &target) { target ^= 1ull << msb(target); }

inline int min(int a, int b) { return (b < a) ? b : a; }

// sliders

extern ull sliderAttacks[16384]; // first 2 bits - direction, next 6 - target pos, last 6 - other pieces pos

extern ull dirMask[256];       // first 2 bits - direction, last 6 - pos
extern ull dirMultiplier[256]; // first 2 bits - direction, last 6 - pos

const ull LAST_SIX = 0xfc00000000000000;

void initSliders();

inline ull getDirMoves(int key, ull pieces) {
    return sliderAttacks[key | (((pieces & dirMask[key]) * dirMultiplier[key] & LAST_SIX) >> 50)];
}

inline ull getDirMoves(int pos, Direction dir, ull pieces) {
    return getDirMoves((pos << 2) | dir, pieces);
}

inline ull getRookMoves(int pos, ull pieces) {
    return getDirMoves(pos, HORIZONTAL, pieces) | getDirMoves(pos, VERTICAL, pieces);
}

inline ull getBishopMoves(int pos, ull pieces) {
    return getDirMoves(pos, LEFTUP_DIAGONAL, pieces) | getDirMoves(pos, RIGHTUP_DIAGONAL, pieces);
}

// segments (excluding first edge, always store last pos)

extern ull segment[4096]; // first 6 bist - first pos, last 6 - last pos

inline ull getSegment(int pos1, int pos2) {
    return segment[(pos1 << 6) | pos2];
}

void initSegments();

// attacks

extern ull knightAttacks[64];
extern ull kingAttacks[64];
extern ull pawnAttacks[128]; // first bit - color of defender, last 6 - pos

void initAttacks();

// init converter

extern ull posToULL[64];

void initConverter();

// rand gen

class RandGen {
    ull s;

public:
    ull get() {
        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }
    RandGen(ull s) : s(s) {}
};

extern ull zobristHash[793]; // 12*64 + 16 + 8 + 1

void initZobrist();

// init

inline void init() {
    initSliders();
    initSegments();
    initAttacks();
    initConverter();
    initZobrist();
}

inline uint32_t max(uint32_t a, uint32_t b) { return a > b ? a : b; }
