//
// Created by SHL on 21.03.2017.
//
#include "properties.h"

const char* MEASURES[] = {"", "times", "day", "hour", "min", "sec", "~C", "%", "kPa", "cm", "A", "baud"};


void Property::updateRuntime(long new_value) {
    runtime = new_value * scale;
}
