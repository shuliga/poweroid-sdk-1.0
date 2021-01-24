//
// Created by SHL on 09.09.2018.
//

#include "global.h"
#include "commons.h"

#if PWR_BOARD_VERSION != PWR20

#ifndef INDICATORS_H
#define INDICATORS_H

#include "timings.h"

#define IND_1 0
#define IND_2 1
#define IND_3 2

class Indicators{
public:

    uint8_t DATA = 0;

    void init();
    void set(uint8_t i, bool on);
    void flash(uint8_t i, bool flash, bool trigger);
private:
    static const uint8_t INDICATOR_PINS[];

};

extern Indicators INDICATORS;

#endif //INDICATORS_H

#endif