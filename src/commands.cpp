//
// Created by SHL on 26.08.2018.
//

#include "commons.h"
#include "commands.h"

static const char *ORIGIN = "CMD";

Cu cu{
    cu.cmd_str = {"get_ver",
                  "get_dht",
                  "get_state_",
                  "disarm_state_",
#ifdef ALLOW_TOKEN
                  "set_token",
                  "pass_token",
#endif
                  "get_prop_",
                  "get_bin_prop_",
                  "set_bin_prop_",
                  "set_prop_",
                  "load_props",
                  "store_props",
                  "reset_props",
                  "set_relay_",
                  "remote_state ",
                  "get_all_prop",
#ifndef SAVE_RAM
                    "get_all_state",
                    "get_all_sensor",
                    "get_all_relay",
#endif
    }
};

const char *cmd_arrow = " -> ";

String cmd;

uint8_t getValIndex() { return cmd.lastIndexOf(':') + 1; }

void printCmdResponse(const String &cmd, const char *suffix) {
    Serial.print(cmd);
    printCmd(cmd_arrow, suffix);
}

void printCmd(const char *cmd, const char *suffix) {
    Serial.print(cmd);
    suffix != NULL ? Serial.println(suffix) : Serial.println();
    Serial.flush();
#ifdef DEBUG
    writeLog('I', ORIGIN, 100, cmd);
#endif
}

uint8_t getIndex() {
    uint8_t li = (uint8_t) cmd.lastIndexOf(':');
    return (uint8_t) cmd.substring((unsigned int) (cmd.lastIndexOf('_') + 1), li == -1 ? cmd.length() : li).toInt();
}

bool isCommand() {
    for (uint8_t i = 0; i < ARRAY_SIZE(cu.cmd_array); ++i) {
        if (cmd.startsWith(cu.cmd_array[i]) && cmd.indexOf(cmd_arrow) < 0) {
            return true;
        }
    }
    return false;
}

bool castCommand(const char *prefix, const char *val) {
    if (cmd.startsWith(prefix)) {
        printCmdResponse(cmd, val);
        return true;
    }
    return false;
}