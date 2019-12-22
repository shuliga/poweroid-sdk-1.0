#ifndef TIMINGS_H
#define TIMINGS_H

#include <Arduino.h>

#define TIMER_0_25HZ    4
#define TIMER_0_5HZ     3
#define TIMER_1HZ       2
#define TIMER_2HZ       1
#define TIMER_4HZ       0

#define TIMERS_FLASH_COUNTS 4
#define TIMERS_COUNT        5
#define TIMER_COUNTER_MAX   15

#define test_timer(timer) (timerFlags >> timer) & 1U

#define flash_symm(counter) counter < 2
#define flash_accent(counter) counter == 0
#define flash_(counter) counter % 2 == 0

const unsigned long MAX_LONG = 4294967295L;

extern uint8_t timerFlags;
extern uint8_t timerCounter_1Hz;
extern uint8_t timerCounter_4Hz;


typedef struct TimingState {
    unsigned long mils;
    unsigned long interval;
    long delta;
    long suspended;
    bool state;
    bool dirty;

    TimingState(unsigned long interval) : interval(interval) {};

    void reset();

    unsigned long getCurrent();

    bool testInterval(unsigned long current);

    bool countdown(bool on, bool suspend, bool cancel);

    bool isTimeAfter(bool trigger);

    bool flash();

    bool ping();

    long millsToGo(unsigned long current);

    long millsToGo();
};

#endif
