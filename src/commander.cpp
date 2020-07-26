
#include "commons.h"
#include "context.h"
#include "commands.h"
#include "commander.h"

static const char *ORIGIN = "CMD";
const char *STATE_FORMAT_BUFF = "[%i] State %s: %s";

Commander::Commander(Context &_ctx) : ctx(&_ctx) {};

const char *Commander::printProperty(uint8_t i) {
    char _desc[64];
    flashStringHelperToChar(ctx->PROPERTIES[i].desc, _desc);
    sprintf(BUFF, "[%i] %s : %i", i, _desc,
            (int) (ctx->PROPERTIES[i].runtime / ctx->PROPERTIES[i].scale));
    return BUFF;
}

void Commander::printBinProperty(uint8_t i) {
    Serial.println(ctx->PROPERTIES[i].desc);
    for (uint8_t j = 0; j < sizeof(Property); j++) {
        Serial.write(*((uint8_t *) &ctx->PROPERTIES[i] + j));
    }
}

void Commander::listen() {
    if (Serial.available()) {
        cmd = Serial.readStringUntil('\n');
        if (isCommand()) {
#ifdef DEBUG
            writeLog('I', ORIGIN, 200 + ctx->passive, cmd.c_str());
#endif
#ifndef SPI
            digitalWrite(LED_PIN, HIGH);
#endif

#ifdef ALLOW_TOKEN

            if (TOKEN_ENABLE && cmd.startsWith(cu.cmd_str.CMD_SET_TOKEN)) {
                printCmdResponse(cmd, NULL);
                COM_TOKEN = cmd.substring((unsigned int) getValIndex()).toInt();
            }

            if (TOKEN_ENABLE && cmd.startsWith(cu.cmd_str.CMD_PASS_TOKEN)) {
                uint8_t token = cmd.substring((unsigned int) getValIndex()).toInt();
                ctx->hasToken = COM_TOKEN == token;
                if (ctx->hasToken) {
                    printCmdResponse(cmd, NULL);
                }

            }
#endif
            if (ctx->canRespond()) {

                castCommand(cu.cmd_str.CMD_GET_VER, ctx->version);

                castCommand(cu.cmd_str.CMD_GET_DHT, ctx->SENS.printDht());

#ifndef DO_NOT_PARSE_REMOTE
                if (REMOTE_ENABLE && cmd.startsWith(cu.cmd_str.CMD_REMOTE_STATE)) {
                    ctx->peerFound = 2;
                    if (cmd.indexOf(REMOTE_HOST) > 0 && ctx->passive) {
                        int8_t i = cmd.indexOf(',');
                        int8_t j = cmd.indexOf(',', i + 1);
                        int8_t k = cmd.indexOf(',', j + 1);
                        int8_t l =  cmd.indexOf(',', k + 1);
                        if (i > 0) {
                            ctx->props_size = cmd.substring(i + 1, j).toInt();
                            ctx->props_default_idx = cmd.substring(j + 1, k).toInt();
                            BANNER.mode = cmd.substring(k + 1, l).toInt();
                            if (l > k) {
                                cmd.substring(l + 1, cmd.indexOf('\r')).toCharArray(BANNER.data.text, LINE_SIZE);
                            }
                        }
                    }
                }
#endif

                if (cmd.startsWith(cu.cmd_str.CMD_RESET_PROPS)) {
                    printCmdResponse(cmd, NULL);
                    for (uint8_t i = 0; i < ctx->props_size; i++) {
                        ctx->PROPERTIES[i].runtime = ctx->PROPERTIES[i].val;
                    }
                    ctx->refreshProps = true;
                    ctx->propsUpdated = true;
                }

                if (cmd.startsWith(cu.cmd_str.CMD_SET_RELAY)) {
                    Relays relays = ctx->RELAYS;
                    int8_t i = ctx->passive ? relays.getMappedFromVirtual(getIndex()) : getIndex();
                    if (i >= 0 && i < relays.size()) {
                        ctx->refreshState = true;
                        bool power = cmd.indexOf(": 1") > 0;
                        relays.power(static_cast<uint8_t>(i), power);
                        printCmdResponse(cmd, idxToChar(static_cast<uint8_t>(power)));
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_SET_FLAGS)) {
                    int8_t  valIndex = getValIndex();
                    if ( valIndex > 1){
                        PWR_FLAGS = cmd.substring(getValIndex()).toInt();
                        ctx->PERS.storeFlags();
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_ALL_PROP)) {
                    for (uint8_t i = 0; i < ctx->props_size; i++) {
                        printCmdResponse(cmd, printProperty(i));
                    }
                }

                if (castCommand(cu.cmd_str.CMD_LOAD_PROPS, NULL)) {
                    ctx->PERS.loadProperties(ctx->PROPERTIES);
                    ctx->refreshProps = true;
                    ctx->propsUpdated = true;
                }

                if (castCommand(cu.cmd_str.CMD_STORE_PROPS, NULL)) {
                    storeProps();
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_PROP)) {
                    uint8_t i = getIndex();
                    if (i < ctx->props_size) {
                        printCmdResponse(cmd, printProperty(i));
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_SET_BIN_PROP)) {
                    strcpy(BUFF, Serial.readStringUntil('\n').c_str());
                    Serial.readBytes((uint8_t *) &ctx->remoteProperty, sizeof(Property));
                    ctx->refreshProps = true;
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_BIN_PROP)) {
                    uint8_t i = getIndex();
                    if (i < ctx->props_size) {
                        printCmd(cu.cmd_str.CMD_SET_BIN_PROP, idxToChar(i));
                        Serial.flush();
                        printBinProperty(i);
                        Serial.flush();
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_SET_PROP)) {
                    if (!ctx->passive) {
                        uint8_t i = getIndex();
                        int8_t idx = getValIndex();
                        if (i < ctx->props_size && idx > 0) {
                            long v = cmd.substring((unsigned int) idx).toInt();
                            ctx->PROPERTIES[i].runtime = v * ctx->PROPERTIES[i].scale;
                            ctx->refreshProps = true;
                            ctx->propsUpdated = true;
                            printCmdResponse(cmd, printProperty(i));
                        }
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_STATE)) {
                    uint8_t i = getIndex();
                    if (i < state_count) {
                        printCmdResponse(cmd, printState(i));
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_DISARM_STATE)) {
                    uint8_t i = (uint8_t) getIndex();
                    bool trigger = (bool) cmd.substring((unsigned int) getValIndex()).toInt();
                    disarmStateCmd(i, trigger);
                }

#ifndef SAVE_RAM
                if (cmd.startsWith(cu.cmd_str.CMD_GET_ALL_STATE)) {
                    for (uint8_t i = 0; i < state_count; i++) {
                        printCmdResponse(cmd, printState(i));
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_ALL_RELAY)) {
                    for (uint8_t i = 0; i < ctx->RELAYS.size(); i++) {
                        printCmdResponse(cmd, ctx->RELAYS.printRelay(i));
                    }
                }

                if (cmd.startsWith(cu.cmd_str.CMD_GET_ALL_SENSOR)) {
                    for (uint8_t i = 0; i < ctx->SENS.size(); i++) {
                        printCmdResponse(cmd, ctx->SENS.printSensor(i));
                    }
                }

#endif
            }

#ifndef SPI
            digitalWrite(LED_PIN, LOW);
#endif

        } else {
#ifdef DEBUG
            writeLog('I', ORIGIN, 210 + ctx->passive, cmd.c_str());
#endif
        }
    }
}

void Commander::storeProps() {
    ctx->PERS.storeProperties(ctx->PROPERTIES);
}

bool Commander::isConnected() {
    if (ctx->canRespond()){
        if (test_timer(TIMER_0_25HZ)) {
            sprintf(BUFF, "%s,%i,%i,%i,%s", REMOTE_HOST, ctx->props_size, ctx->props_default_idx, BANNER.mode,
                    BANNER.data.text);
            printCmd(cu.cmd_str.CMD_REMOTE_STATE, ctx->passive ? REMOTE_CONTROL : BUFF);
            connected = TOKEN_ENABLE ? true : ctx->peerFound > 0;
            ctx->peerFound > 0 ? ctx->peerFound-- : ctx->peerFound;
#ifdef DEBUG
            if (ctx->passive && !connected) {
            writeLog('W', ORIGIN, 410);
        }
#endif
        }
    } else {
        connected = false;
    }
    return connected;
}

const char *Commander::printState(uint8_t i) {
    if (ctx->canRespond()){
        sprintf(BUFF, STATE_FORMAT_BUFF, i, getState(i)->name, getState(i)->state);
        return BUFF;
    } else {
        return NULL;
    }
}

void Commander::disarmStateCmd(uint8_t i, bool disarm) {
    disarmState(i, disarm);
    ctx->PERS.storeState(i, disarm);
    if (ctx->canRespond()) {
        Serial.println(printState(i));
    }
}
