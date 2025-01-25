#pragma once
#include "types.hpp"

class Move {
private:
    uint16_t data; // first 6 bits - from, next 6 - to, next 2 - special (none, castle, en-passant, promotion), last2 - promotion type

public:
    inline Move(int from = 0, int to = 0, int type = 0, int promotion = 0) : data(from | (to << 6) | type | (promotion << 14)) {};
    inline int from() { return data & 63; }
    inline int to() { return (data >> 6) & 63; }
    inline int type() { return data & 12288; }
    inline int promotion() { return data >> 14; }
    inline void reset() { data = 0; }
    inline void setData(uint16_t data) { this->data = data; }
    inline bool isSet() { return this->data; }

    bool operator==(Move m) { return data == m.data; }
    explicit operator ull() const {
        return static_cast<ull>(data);
    }
};
