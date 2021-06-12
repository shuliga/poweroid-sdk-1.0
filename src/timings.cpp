#include "timings.h"
#include <Arduino.h>

void splitTime(long millis, TimeSplit& timeSplit){
    timeSplit.totalSec = static_cast<uint16_t>(millis / 1000);
    timeSplit.hrs = static_cast<uint8_t>(timeSplit.totalSec / 3600);
    timeSplit.secM = timeSplit.totalSec - (timeSplit.hrs * 3600);
    timeSplit.mins = static_cast<uint8_t>(timeSplit.secM / 60);
    timeSplit.sec = static_cast<uint8_t>(timeSplit.secM - (timeSplit.mins * 60));
}

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
