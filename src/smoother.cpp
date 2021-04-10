//
// Created by SHL on 20.05.2020.
//

#include "smoother.h"
#define INT16_MAX 0x7fffL
#define INT16_MIN (-INT16_MAX - 1L)

void Smoother::feed(int16_t val) {
    shift();
    vals[0] = val;
}

int16_t Smoother::get() {
    int16_t min = INT16_MAX;
    int16_t max = INT16_MIN;
    int16_t res = 0;
    for (uint8_t i = 0; i < _curr_len; ++i){
        if (vals[i] < min)
            min = vals[i];
        if (vals[i] > max)
            max = vals[i];
        res = res + vals[i];
    }
    return _curr_len < 3 ? vals[0] : (res - min - max )/(_curr_len - 2);
}

void Smoother::shift() {
    if (_curr_len < _len){
        _curr_len++;
    }
    for(uint8_t i = _curr_len - 1; i > 0; --i){
        vals[i] = vals[i-1];
    }
}

int16_t Smoother::feedAndGet(uint16_t val) {
    feed(val);
    return get();
}

