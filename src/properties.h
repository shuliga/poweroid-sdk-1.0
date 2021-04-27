#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Arduino.h>

#define MEASURE_NONE 0
#define MEASURE_TIMES 1
#define MEASURE_DAY 2
#define MEASURE_HOUR 3
#define MEASURE_MIN 4
#define MEASURE_SEC 5
#define MEASURE_DEG_C 6
#define MEASURE_PERCENT 7
#define MEASURE_KPA 8
#define MEASURE_CM 9
#define MEASURE_AMPER 10
#define MEASURE_BAUD 11

#define GET_PROP_NORM(i) PROPS.FACTORY[(i)].runtime / PROPS.FACTORY[(i)].scale

extern const char* MEASURES[];

typedef struct Property {

    const __FlashStringHelper *desc;
    uint8_t measure;
    long runtime;

    long val;
    long minv;
    long maxv;
    long scale = 1;

    Property(){}
    Property(long v, long n, long m, long s) : minv(n), maxv(m), val(v), scale(s) {}
};

#endif
