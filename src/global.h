//
// Created by SHL on 18.11.2017.
//
#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduino.h>

#define BUFF_SIZE 65
#define LINE_SIZE 16

#ifdef MINI
#define PWR_SIZE 2
#else
#define PWR_SIZE 3
#endif


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
    int8_t mode;  // BANNER MODE: 0 - SINGLE TEXT LINE; 1 - SINGLE GAUGE; 2 - DOUBLE GAUGES; 4 - DOUBLE GAUGES WITH TEXT LINES IN EXTRA BUFFER
    banner_data data;
} banner;

extern char BUFF[BUFF_SIZE];
extern char EXTRA_BUFF[2][LINE_SIZE + 1];
extern char CUSTOM_HEADER[3];
extern banner BANNER;
extern uint8_t PWR_FLAGS;
extern uint8_t COM_TOKEN;
extern bool DAYLIGHT;

#endif //GLOBAL_H
