#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Arduino.h>

#define TIMES 0
#define DAY 1
#define HOUR 2
#define MIN 3
#define SEC 4
#define DEG_C 5
#define PERCENT 6
#define KPA 7
#define CM 8


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
