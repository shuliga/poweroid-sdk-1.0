//
// Created by SHL on 20.05.2020.
//

#ifndef SMOOTHER_H
#define SMOOTHER_H

#include <Arduino.h>

#define SMTH_LEN 5

class Smoother {

public:

    void feed(int16_t val);
    int16_t get();
    int16_t feedAndGet(uint16_t val);

private:
    const uint8_t _len = SMTH_LEN;
    uint8_t _curr_len = 0;
    int16_t vals[SMTH_LEN];

    void shift();

};

#endif //SMOOTHER_H
