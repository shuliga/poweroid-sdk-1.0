//
// Created by SHL on 20.03.2017.
//

#define INCR(val, max) val < (max) ? (val)++ : val
#define DECR(val, min) val > (min) ? (val)-- : val

#include <Wire.h>
#include "global.h"
#include <MultiClick/MultiClick.h>
#include <Rotary/Rotary.h>
#include <ACROBOTIC_SSD1306/ACROBOTIC_SSD1306.h>
#include "controller.h"
#include "commands.h"
#include "datetime.h"


#ifndef NO_CONTROLLER

#define DISPLAY_BASE 1
#define DISPLAY_BOTTOM 7

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, STATES, SUSPEND, FLAG, TOKEN, DATE_TIME
} oldState = STORE, state = BROWSE;

static TimingState sleep_timer = TimingState(100000L);
static TimingState autoComplete_timer = TimingState(6000L);

static volatile bool control_touched = false;
static volatile uint8_t prop_idx = 0;
static volatile uint8_t state_idx = 0;
static volatile long prop_value;

static volatile int props_idx_max = 0;
static volatile int state_idx_max = 0;


static int8_t c_prop_idx = -1;
static int8_t c_state_idx = -1;
static uint8_t c_byte_value = 255;
static long c_prop_value = -1;
static long old_prop_value;
static bool dither = false;

static uint8_t timeDateBlock = 0;
static uint8_t dateTimePartIdx = 0;

static volatile long prop_min;
static volatile long prop_max;
static volatile uint8_t prop_measure;
static volatile bool requestForRefresh = false;
static volatile bool enablePropControl = false;


#if defined(ENC1_PIN) && defined(ENC2_PIN)
Rotary encoder = Rotary(ENC1_PIN, ENC2_PIN);
#endif

MultiClick encoderClick = MultiClick(ENC_BTN_PIN);


Controller::Controller(Context &_ctx, Commander &_cmd) : cmd(&_cmd), ctx(&_ctx) {}

void Controller::begin() {
    initDisplay();
#if defined(ENC1_PIN) && defined(ENC2_PIN)
    initEncoderInterrupts();
    props_idx_max = ctx->props_size - 1;
    state_idx_max = state_count - 1;
#endif
}

void Controller::initEncoderInterrupts() {

    cli();
#if defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) //ARDUINO_AVR_UNO_PRO
#define PCVECT PCINT1_vect
    PCICR |= (1 << PCIE1);
    PCMSK1 |= (1 << PCINT10) | (1 << PCINT11);
#else //ARDUINO_AVR_PRO
#define PCVECT PCINT2_vect
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
#endif
    sei();

}

void Controller::initDisplay() {
    if (oled.checkAndInit(FLIP_DISPLAY)) {
        oled.clearDisplay();
        oled.setBrightness(0);
    };
}

void Controller::outputState(bool relays) const {
    strcpy(BUFF, relays ? (const char *) ctx->RELAYS.relStatus() : ctx->id);
    padLineInBuff(BUFF, 1, 0);
    if (TOKEN_ENABLE || ctx->remoteMode){
        BUFF[15] = (unsigned char) (TOKEN_ENABLE ? COM_TOKEN + 48 : (ctx->remoteMode ? ((
                (ctx->connected ? CHAR_CONNECTED : CHAR_DISCONNECTED) + (ctx->passive ? 0 : 2))) : '\0'));
    }
    oled.setTextXY(0, 0);
    oled.putString(BUFF);
}

void Controller::process() {

    enablePropControl = ctx->canAccessLocally() || ctx->canCommunicate();

    oled.checkConnected();

    McEvent event = encoderClick.checkButton();

    switch (state) {

        case EDIT_PROP: {

            if (testControl(autoComplete_timer) || ctx->refreshProps) {
                if (loadProperty(prop_idx)) {
                    outputPropDescr(BUFF);
                    outputStatus(F("Edit value:"), old_prop_value);
                }
            }

            if (c_prop_value != prop_value || ctx->refreshProps) {
                updateProperty(prop_idx);
                outputPropVal(prop_measure, (int16_t) prop_value, true, true);
            }

            if (event == CLICK || autoComplete_timer.isTimeAfter(true)) {
                ctx->propsUpdated = true;
                goToBrowse();
            }

            if (event == DOUBLE_CLICK) {
                prop_value = old_prop_value;
                updateProperty(prop_idx);
                goToBrowse();
            }

            if (event == HOLD) {
                state = STORE;
            }
            break;
        }

        case BROWSE: {

            bool prop_id_changed = c_prop_idx != prop_idx;

            if (testControl(sleep_timer) || prop_id_changed || ctx->refreshProps || ctx->refreshState) {
                outputState(true);
                if (loadProperty(prop_idx)) {
                    outputPropDescr(BUFF);
                    outputPropVal(prop_measure, (int16_t) prop_value, false, true);
                    outputStatus(F("Property:"), prop_idx + 1);
                }
            }

            if (event == CLICK && canGoToEdit()) {
                goToEditProp(prop_idx);
            }

            if (event == HOLD || sleep_timer.isTimeAfter(true)) {
                state = SLEEP;
            }

            if (event == DOUBLE_CLICK) {
                state = ctx->canAccessLocally() ? STATES : FLAG;
            }

            break;
        }

        case STATES: {
            if (testControl(sleep_timer) || c_state_idx != state_idx) {
                strcpy(BUFF, getState(state_idx)->name);
                outputPropDescr(BUFF);
                strcpy(BUFF, getState(state_idx)->state);
                padLineCenteredInBuff(BUFF);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
                outputStatus(F("State:"), state_idx);
                c_state_idx = state_idx;
            }

            if (event == HOLD) {
                bool disarm = strcmp(getState(state_idx)->state, "DISARM") != 0;
                cmd->disarmStateCmd(state_idx, disarm);
                c_state_idx = -1;
            }

            if (event == CLICK || sleep_timer.isTimeAfter(true)) {
                state = BROWSE;
            }

            if (event == DOUBLE_CLICK) {
                state = FLAG;
            }
            break;
        }

        case FLAG: {
            if (testControl(sleep_timer)) {
                outputState(false);
                outputDescr("FLAGS", 1);
                prop_max = FLAGS_MAX;
                prop_value = PWR_FLAGS;
                prop_min = 0;
            }

            if (prop_value != c_byte_value) {
                PWR_FLAGS = (uint8_t) prop_value;
                itoa(PWR_FLAGS, BUFF, 2);
                padLineCenteredInBuff(BUFF);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
                c_byte_value = PWR_FLAGS;
                outputStatus(F("Decimal:"), PWR_FLAGS);
            }

            if (event == HOLD) {
                ctx->PERS.storeFlags();
                goToBrowse();
            }

            if (event == CLICK || sleep_timer.isTimeAfter(true)) {
                goToBrowse();
            }

            if (event == DOUBLE_CLICK) {
                state = DATE_TIME;
            }

            break;
        }

        case DATE_TIME: {
#ifndef DATETIME_H
            if (TOKEN_ENABLE)
                    state = TOKEN;
                else
                    goToBrowse();
#else
            bool isTime = timeDateBlock == 1;

            if (testControl(sleep_timer)) {
                outputState(false);
                timeDateBlock = 0;
                dateTimePartIdx = 0;
                prop_max = 1;
                prop_value = 0;
                prop_min = -1;
                DATETIME.getDateString(dateString);
                DATETIME.getTimeString(timeString);
                outputStatus(F("     "), 0);
            }

            if (prop_value != 0 ) {
                DATETIME.dialDateTimeString( isTime ? timeString : dateString, dateTimePartIdx, isTime, prop_value < 0, false);
                prop_value = 0;
            }

            if (test_timer(TIMER_2HZ)){
                outputDescr(isTime ? time : date, 1);
                if (isTime)
                    strcpy(BUFF, timeString);
                else
                    strcpy(BUFF, dateString);

                if (flash_symm(timerCounter_4Hz)){
                    DATETIME.screenDateTimePart(BUFF, dateTimePartIdx);
                }

                padLineCenteredInBuff(BUFF);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
            }

            if (event == HOLD) {
                DATETIME.setTimeFromString(timeString);
                DATETIME.setDateFromString(dateString);
                goToBrowse();
            }

            if (event == CLICK) {
                dateTimePartIdx++;
                if (dateTimePartIdx == 3){
                    dateTimePartIdx = 0;
                    timeDateBlock++;
                    if (timeDateBlock == 2){
                        timeDateBlock = 0;
                    }
                }
            }


            if (sleep_timer.isTimeAfter(true)) {
                timeDateBlock++;
                sleep_timer.reset();
                if (timeDateBlock = 2){
                    goToBrowse();
                }
            }

            if (event == DOUBLE_CLICK) {
                if (TOKEN_ENABLE)
                    state = TOKEN;
                else
                    goToBrowse();
            }
#endif
            break;
        }

        case TOKEN: {
            if (firstRun()) {
                outputState(false);
                outputDescr("TOKEN", 1);
                prop_max = TOKEN_MAX;
                prop_value = COM_TOKEN;
                prop_min = 0;
            }

            if (prop_value != c_byte_value) {
                COM_TOKEN = (uint8_t) prop_value;
                itoa(COM_TOKEN, BUFF, 10);
                oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
                c_byte_value = COM_TOKEN;
                outputStatus(F("Decimal:"), COM_TOKEN);
            }

            if (event == HOLD) {
                ctx->PERS.storeToken();
                goToBrowse();
            }

            if (event == CLICK || sleep_timer.isTimeAfter(true)) {
                goToBrowse();
            }

            break;
        }

        case STORE: {
            firstRun();
            if (ctx->canAccessLocally()) {
                cmd->storeProps();
            } else {
                printCmd(cu.cmd_str.CMD_STORE_PROPS, NULL);
            }
            outputStatus(F("Storing... "), prop_value);
            delay(500);
            goToBrowse();
            break;
        }

        case SLEEP: {
            if (firstRun()) {
                sleep_timer.reset();
                dither = false;
                switchDisplay(false);
                c_prop_idx = -1; // invalidate cache;
            }


            if (sleep_timer.isTimeAfter(true)) {
                if (dither) {
                    state = SUSPEND;
                } else {
                    dither = true;
                    sleep_timer.reset();
                }

            }

            // Output Sleep Screen
            if (test_timer(TIMER_2HZ) && oled.getConnected()) {
                if (BANNER.mode == 0) {
                    padLineCenteredInBuff(BANNER.data.text);
                    oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, dither);
                } else {
                    for (uint8_t i = 0; i < BANNER.mode; i++) {
                        int16_t val = BANNER.data.gauges[i].val;
                        int16_t min = BANNER.data.gauges[i].min;
                        int16_t max = BANNER.data.gauges[i].max;
                        int16_t g_min = BANNER.data.gauges[i].g_min;
                        int16_t g_max = BANNER.data.gauges[i].g_max;
                        int16_t col_min = normalizeGauge(min, g_min, g_max);
                        int16_t col_max = normalizeGauge(max, g_min, g_max);
                        uint8_t row = i == 0 ? 0 : DISPLAY_BOTTOM;
                        int8_t direction = i == 0 ? 1 : -1;
                        uint8_t char_col_min = col_min / 8 + 1;
                        uint8_t char_col_max = col_max / 8 + 1;
                        char FMT[9];
                        sprintf(FMT, "%%%dd%%%dd", char_col_min, char_col_max - char_col_min);
                        sprintf(BUFF, FMT, min, max);
                        oled.setTextXY(row, 0);
                        oled.putString(BUFF);
                        oled.outputLineGauge(row + direction, normalizeGauge(val, g_min, g_max), col_min, col_max,
                                             direction == -1);
                        sprintf(BUFF, "%d %s", val, MEASURES[BANNER.data.gauges[i].measure]);
                        padLineCenteredInBuff(BUFF);
                        if (BANNER.mode == 1) {
                            oled.outputTextXY(DISPLAY_BASE + 1, 64, BUFF, true, dither);
                        } else {
                            oled.setTextXY(row + (direction * 2), 0);
                            oled.putString(BUFF);
                        }
                    }
                }
            }

            // Exit SLEEP state on event
            if (event == CLICK) {
                switchDisplay(true);
                state = BROWSE;
            };

            if (control_touched && ctx->props_default_idx >= 0 && canGoToEdit()) {
                control_touched = false;
                switchDisplay(true);
                prop_idx = (uint8_t) ctx->props_default_idx;
                goToEditProp(prop_idx);
            }

            break;
        }

        case SUSPEND: {
            if (firstRun()) {
                oled.displayOff();
            }

            if (event == CLICK || control_touched) {
                control_touched = false;
                oled.displayOn();
                state = SLEEP;
            };

            break;
        }
    }
    if (ctx->refreshState) {
        if (state == SUSPEND || state == SLEEP) {
            state = SLEEP;
        } else {
            outputState(true);
        }
        ctx->refreshState = false;
    }
    ctx->refreshProps = false;

}

uint8_t Controller::normalizeGauge(uint16_t val, uint16_t min, uint16_t max) {
    return static_cast<uint8_t>((val - min) * 127 / (max - min));
}


bool Controller::testControl(TimingState &timer) const {
    bool fr = firstRun();
    if (fr || control_touched) {
        timer.reset();
        control_touched = false;
        if (fr) {
            outputState(true);
        }
    }
    return fr;
}

void Controller::goToBrowse() const {
    // invalidate cache
    c_prop_idx = -1;
    c_prop_value = -1;
    c_byte_value = 255;

    state = BROWSE;
}

bool Controller::firstRun() const {
    bool fr = oldState != state;
    oldState = state;
    return fr;
}

void Controller::goToEditProp(uint8_t i) const {

    c_prop_value = -1; // invalidate cache
    c_prop_idx = -1; // invalidate cache
    old_prop_value = prop_value; // for reminder in status
    state = EDIT_PROP;
}

bool inline Controller::canGoToEdit() {
    return !ctx->passive || ctx->connected || !ctx->remoteMode;
}


bool Controller::loadProperty(uint8_t idx) const {
    c_prop_idx = idx;
    flashStringHelperToChar(ctx->PROPERTIES[idx].desc, BUFF);
    if (ctx->canAccessLocally()) {
        copyProperty(ctx->PROPERTIES[idx], idx);
    } else {
        if (ctx->connected) {
            if (ctx->refreshProps) {
                copyProperty(ctx->remoteProperty, idx);
                requestForRefresh = false;
            } else {
                if (!requestForRefresh) {
                    printCmd(cu.cmd_str.CMD_GET_BIN_PROP, idxToChar(idx));
                    requestForRefresh = true;
                }
                return false;
            }
        } else {
            BUFF[0] = '\0';
        }
    }
    return true;
}

void Controller::copyProperty(Property &prop, uint8_t idx) const {
    long scale = prop.scale;
    prop_value = (prop.runtime / scale);
    prop_min = (prop.minv / scale);
    prop_max = (prop.maxv / scale);
    prop_measure = prop.measure;
}

void Controller::updateProperty(uint8_t idx) const {
    if (ctx->canAccessLocally()) {
        ctx->PROPERTIES[idx].runtime = prop_value * ctx->PROPERTIES[idx].scale;
    } else {
        if (ctx->connected) {
            sprintf(BUFF, "%i:%lu", idx, prop_value);
            printCmd(cu.cmd_str.CMD_SET_PROP, BUFF);
        }
    }
    c_prop_value = prop_value;
}

void Controller::switchDisplay(boolean inverse) const {
    oled.displayOff();
    inverse ? oled.setInverseDisplay() : oled.setNormalDisplay();
    oled.clearDisplay();
    oled.displayOn();
}

void Controller::outputPropDescr(const char *_buff) {
    if (canGoToEdit()) {
        outputDescr(_buff, 2);
    }
}

void Controller::outputDescr(const char *_buff, uint8_t lines) const {
    oled.setTextXY(DISPLAY_BASE, 0);
    strcpy(BUFF, _buff);
    padLineInBuff(BUFF, lines, 0);
    oled.putString(BUFF);
}

void Controller::outputStatus(const __FlashStringHelper *txt, const long val) {
    flashStringHelperToChar(txt, BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, 0);
    uint8_t prop_size = static_cast<uint8_t>(val > 0 ? log10((double) val) + 1 : (val == 0 ? 1 : log10((double) -val) + 2));
    padLineInBuff(BUFF, 1, prop_size);
    oled.putString(BUFF);
    oled.setTextXY(DISPLAY_BOTTOM, (unsigned char) (LINE_SIZE - prop_size));
    oled.putNumber(val);
}

void Controller::padLineInBuff(char *_buff, uint8_t lines, uint8_t tail) {
    uint8_t t = LINE_SIZE * lines - tail;
    for (uint8_t i = 0; i < strlen(_buff); i++) {
        BUFF[i] = _buff[i];
    }
    for (uint8_t i = strlen(_buff); i < t; i++) {
        BUFF[i] = ' ';
    }
    BUFF[t] = '\0';
}

void Controller::padLineCenteredInBuff(char *_buff) {
    const uint8_t text_size = strlen(_buff);
    const uint8_t text_start = (LINE_SIZE - text_size) / 2;
    for (uint8_t i = 0; i < LINE_SIZE; i++) {
        if (i < text_size) {
            uint8_t text_base = text_size - 1 - i;
            BUFF[text_base + text_start] = _buff[text_base];
        }
        uint8_t index = LINE_SIZE - i - 1;
        if (index < text_start || index >= (LINE_SIZE + text_size) / 2) {
            BUFF[index] = ' ';
        }
    }
    BUFF[LINE_SIZE] = 0;
}

void Controller::outputPropVal(uint8_t measure_idx, int16_t _prop_val, bool brackets, bool measure) {
    const char *fmt =
            brackets && measure ? "[%i]%s" : (brackets & !measure ? "[%i]" : (!brackets && measure ? "%i%s"
                                                                                                   : "%i"));
    if (canGoToEdit()) {
        sprintf(BUFF, fmt, _prop_val, MEASURES[measure_idx]);
    } else {
        noInfoToBuff();
    }
    padLineCenteredInBuff(BUFF);
    oled.outputTextXY(DISPLAY_BASE + 2, 64, BUFF, true, false);
}

#ifdef ENC1_PIN
#ifdef ENC2_PIN

ISR(PCVECT) {
    unsigned char input = encoder.process();
    if (input != NOTHING) {
        control_touched = true;
        if (!requestForRefresh) {
            switch (state) {

                case FLAG:
                case TOKEN:
                case DATE_TIME:
                case EDIT_PROP: {
                    input == DIR_CW ? DECR(prop_value, prop_min) : INCR(prop_value, prop_max);
                    break;
                }

                case BROWSE: {
                    if(enablePropControl) {
                        input == DIR_CW ? DECR(prop_idx, 0) : INCR(prop_idx, props_idx_max);
                    }
                    break;
                }

                case STATES: {
                    input == DIR_CW ? DECR(state_idx, 0) : INCR(state_idx, state_idx_max);
                    break;
                }

            }
        }
    }
}

#endif
#endif

#endif // NO_CONTROLLER
