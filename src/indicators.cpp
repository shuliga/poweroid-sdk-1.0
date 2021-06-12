//
// Created by SHL on 09.09.2018.
//
#include "indicators.h"

#ifdef INDICATORS_H
#define ORIGIN "INDICATORS"

Indicators INDICATORS;

#ifndef MINI
const uint8_t Indicators::INDICATOR_PINS[] = {IND1_PIN, IND2_PIN, IND3_PIN};
#else
const uint8_t Indicators::INDICATOR_PINS[] = {IND1_PIN, IND2_PIN};
#endif

void Indicators::init() {
    for(uint8_t i = 0; i < ARRAY_SIZE(INDICATOR_PINS); ++i) {
        pinMode(INDICATOR_PINS[i], OUTPUT);
#ifdef DEBUG
        writeLog('I', ORIGIN, 110, INDICATOR_PINS[i]);
#endif
        set(i, false);
    }
}

void Indicators::set(uint8_t i, bool on) {
    digitalWrite(INDICATOR_PINS[i], on ? HIGH : LOW);
    multiplexed = on ? multiplexed | (1UL << i) : multiplexed & ~(1UL << i);
#ifdef DEBUG
    writeLog('I', ORIGIN, 120, INDICATOR_PINS[i]);
#endif
}

void Indicators::flash(uint8_t i, bool flash, bool trigger) {
    set(i, trigger ? (flash ? HIGH : LOW) : LOW);
}

#endif