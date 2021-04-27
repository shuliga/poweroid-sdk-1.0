#include "timings.h"
#include <Arduino.h>

unsigned long TimingState::getCurrent() {
    unsigned long current = millis();
    if (current < mils) {
        delta = -(MAX_LONG - mils);
        mils = 0;
    } else {
        delta = 0;
    }
    return current;
}

inline bool TimingState::testInterval(unsigned long current) {
    return millsToGo(current) <= 0;
}

long TimingState::millsToGo() {
    return interval - (getCurrent() - mils - delta);
}

long TimingState::millsToGo(unsigned long current) {
    return interval - (current - mils - delta);
}

/*
 * Returns 'true' if Timer interval has passed since 'state_on' was set to true.
 * Returns 'false' if 'state_on' is set to false.
 *
 */
bool TimingState::countdown(bool trigger, bool suspend, bool cancel) {
    unsigned long current = getCurrent();
    if (!state && !dirty && trigger) {
        mils = current;
        state = true;
    }
    if (dirty && !trigger) {
        dirty = false;
    }
    if (state) {
        if (suspend) {
            if (suspended == 0) {
                suspended = current;
            } else {
                mils = mils + (current - suspended);
                suspended = current;
            }
        } else {
            suspended = 0;
        }
        if ((testInterval(current) || cancel)) {
            mils = 0;
            state = false;
            dirty = trigger;
        }
    }
    return state;
}

/*
 * Returns 'true' if Timer interval has passed, since 'state_on' was set from 'false' to 'true'.
 * Returns 'false' if 'state_on' is set to false.
 *
 */
bool TimingState::isTimeAfter(bool state_on) {
    unsigned long current = getCurrent();
    if (state_on) {
        if (testInterval(current)) {
            state = true;
        }
    } else {
        mils = current;
        state = false;
    }
    return state;
}

bool TimingState::flash() {
    unsigned long current = getCurrent();
    if (testInterval(current)) {
        state = !state;
        mils = current;
    }
    return state;
}

/*
 * Returns true if Timer interval has passed since last call.
 * Each subsequent call resets counter.
 *
 */
bool TimingState::ping() {
    unsigned long current = getCurrent();
    if (testInterval(current)) {
        mils = current;
        return true;
    } else {
        return false;
    }
}

void TimingState::reset() {
    mils = getCurrent();
    suspended = 0;
    state = false;
    dirty = false;
}
