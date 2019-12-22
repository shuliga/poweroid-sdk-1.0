//
// Created by SHL on 18.11.2017.
//
#include <Arduino.h>

#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef POWEROID_SDK_10_H
    #define EXTERN
#else
    #define EXTERN extern
#endif

#define BUFF_SIZE 65
#define LINE_SIZE 16

#ifdef CONTROLLER_ONLY
#define FLAGS_DEFAULT 2
#endif

#ifdef NO_CONTROLLER
#define FLAGS_DEFAULT 6
#endif

#ifndef FLAGS_DEFAULT
#define FLAGS_DEFAULT 0
#endif

typedef struct {
    int16_t val;
    int16_t min;
    int16_t max;
    int16_t g_min;
    int16_t g_max;
    uint8_t measure;
} gauge_data;

typedef union {
    char text[LINE_SIZE];
    gauge_data gauges[2];
} banner_data;

typedef struct {
    int8_t mode;
    banner_data data;
} banner;

EXTERN char BUFF[BUFF_SIZE];
EXTERN banner BANNER;
EXTERN uint8_t PWR_FLAGS;
EXTERN uint8_t COM_TOKEN;

#endif //GLOBAL_H
