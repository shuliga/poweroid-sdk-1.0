//
// Created by SHL on 26.08.2018.
//

#ifndef COMMANDS_H
#define COMMANDS_H

#include <Arduino.h>

typedef struct CommandsStr {
    const char* CMD_GET_VER;
    const char* CMD_GET_DHT;
    const char* CMD_GET_STATE;
    const char* CMD_DISARM_STATE;
#ifdef ALLOW_TOKEN
    const char* CMD_SET_TOKEN;
    const char* CMD_PASS_TOKEN;
#endif
    const char* CMD_GET_PROP;
    const char* CMD_GET_BIN_PROP;
    const char* CMD_SET_BIN_PROP;
    const char* CMD_SET_PROP;
    const char* CMD_LOAD_PROPS;
    const char* CMD_STORE_PROPS;
    const char* CMD_RESET_PROPS;
    const char* CMD_SET_RELAY;
    const char* CMD_REMOTE_STATE;
    const char* CMD_GET_ALL_PROP;
#ifndef SAVE_RAM
    const char* CMD_GET_ALL_STATE;
    const char* CMD_GET_ALL_SENSOR;
    const char* CMD_GET_ALL_RELAY;
#endif
};

extern union Cu{
    CommandsStr cmd_str;
    char * cmd_array[21];
} cu;

extern const char *cmd_arrow;

extern String cmd;

void printCmdResponse(const String &cmd, const char *suffix);

void printCmd(const char *cmd, const char *suffix);

uint8_t getIndex();

uint8_t getValIndex();

bool castCommand(const char *prefix, const char *val);

bool isCommand();

#endif //COMMANDS_H
