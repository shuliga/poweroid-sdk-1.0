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

#define flash_symm(counter) (counter < 2)
#define flash_accent(counter) (counter == 0)
#define flash_(counter) (counter % 2 == 0)

const unsigned long MAX_LONG = 4294967295L;

extern uint8_t timerFlags;
extern uint8_t timerCounter_1Hz;
extern uint8_t timerCounter_4Hz;

typedef struct TimeSplit {
    uint16_t totalSec;
    uint8_t hrs;
    uint16_t secM;
    uint8_t mins;
    uint8_t sec;
};

void splitTime(long millis, TimeSplit& timeSplit);

typedef struct TimingState {
    unsigned long mils;
    unsigned long interval;
    long delta;
    long suspended;
    bool state;
    bool dirty;

    TimingState(unsigned long interval) : interval(interval) {};

    unsigned long getCurrent();

    bool testInterval(unsigned long current);

    /**
     *
     * Returns 'true as soon as initiated by <code>trigger</code> and till timer countdown was not reached.<br/>
     * Countdown can be temporary suspended or permanently canceled.<br/>
     * If countdown was ended or cancelled, while <code>trigger</code> is on, it can be restarted only after setting <code>trigger</code> to 'false'.
     * Also <code>reset()</code> can be called to countdown reinitialize.
     *
     * @param trigger Triggers countdown, should pass <code>false</code> state to initiate next countdown
     * @param suspend Suspends countdown timer
     * @param cancel Cancels countdown
     *
     * @return true if triggered and not suspended or cancelled
     *
     */
    bool countdown(bool trigger, bool suspend, bool cancel);

    /**
     *
     * Returns 'true' if time has passed since <code>state_on</code> was set from 'false' to 'true'.<br/>
     * As soon as <code>state_on</code> is 'false', the result is reset to 'false' as well.
     *
     * @param state_on
     * @return true if time has passed
     */
    bool isTimeAfter(bool state_on);

    /**
     *
     * Inverses state as soon as timer interval has passed.
     * On inverse, the timer is set back again and cycle continues.
     *
     * @return oscillating state
     */
    bool flash();

    /**
     *
     * Returns 'true' once, when timer interval has passed since last call, then 'false'.
     * On success, the timer is set back again and cycle continues.
     *
     * @return ping state
     */
    bool ping();

    /**
     *
     * Resets the timer and state. Used for <code>countdown</code>, <code>isTimeAfter</code> functions.
     *
     */
    void reset();

    long millsToGo(unsigned long current);

    long millsToGo();
};

#endif
