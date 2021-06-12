#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

#ifndef COMMONS_H
#define COMMONS_H

#if defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)
#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_RX_BUFFER_SIZE 128
#else
#define SERIAL_TX_BUFFER_SIZE 96
#endif

#include <Arduino.h>
#include "boards.h"
#include "global.h"

//#define SAVE_RAM
//#define DEBUG
#define WATCH_DOG

#define FLAG_RELAY_ON_LOW       0U
#define FLAG_REMOTE_ENABLE      1U
#define FLAG_REMOTE_SERVER      2U
#define FLAG_TOKEN_ENABLE       3U
#define FLAG_LOW_SPEED          4U
#define FLAG_REF_1V1            5U

#define FLAGS_MAX 63
#define TOKEN_MAX 9

#define ExtractFlag(flag)   (PWR_FLAGS >> (flag) & 1U)

#define RELAY_ON_LOW        ExtractFlag(FLAG_RELAY_ON_LOW)
#define REMOTE_ENABLE       ExtractFlag(FLAG_REMOTE_ENABLE)
#define REMOTE_SERVER       ExtractFlag(FLAG_REMOTE_SERVER)
#define TOKEN_ENABLE        ExtractFlag(FLAG_TOKEN_ENABLE)
#define LOW_SPEED           ExtractFlag(FLAG_LOW_SPEED)
#define REF_1V1           ExtractFlag(FLAG_REF_1V1)

#define DEBOUNCE_DELAY 500L
#define SERIAL_READ_TIMEOUT 150

#define REMOTE_CONTROL "ctrl"
#define REMOTE_HOST "host"

#define CHAR_CONNECTED      130
#define CHAR_DISCONNECTED   129

#ifdef SSERIAL
#include <AltSoftSerial/AltSoftSerial.h>
extern AltSoftSerial SSerial;

#define RX_SS 8
#define TX_SS 9

#endif

#ifdef DEBUG_SS
#define SSERIAL
#define NO_CONTROLLER
#endif

#ifdef NO_CONTROLLER
#undef SAVE_RAM
#endif

#ifndef SAVE_RAM
#define ALLOW_TOKEN
#endif

static const char *const NO_INFO_STR = "--\0";

unsigned long hash(uint8_t *data, unsigned long size);

char *idxToChar(uint8_t idx);

uint8_t flashStringHelperToChar(const __FlashStringHelper *ifsh, char *dst);

void noInfoToBuff();

void writeLog(char severity, const char * source, uint16_t code);
void writeLog(char severity, const char * source, uint16_t code, unsigned long value);
void writeLog(char severity, const char * source, uint16_t code, const char * descr);

#endif // COMMONS_H