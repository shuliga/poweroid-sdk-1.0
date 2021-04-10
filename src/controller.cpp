//
// Created by SHL on 20.03.2017.
//

#include "global.h"
#include <MultiClick/MultiClick.h>
#include <Rotary/Rotary.h>
#include <ACROBOTIC_SSD1306/ACROBOTIC_SSD1306.h>
#include <I2C.h>
#include "controller.h"
#include "commands.h"

#ifdef RTCM
#include "datetime.h"
#endif

#define I2C_CONTROLLER_ADDR 0x25

#define INCR(val, max) val < (max) ? (val)++ : val
#define DECR(val, min) val > (min) ? (val)-- : val

#ifndef NO_CONTROLLER

#define DISPLAY_BASE 1
#define DISPLAY_BOTTOM 7
#define DISPLAY_LAST_COL 127
#define DISPLAY_CENTER_COL  63

static enum State {
    EDIT_PROP, BROWSE, STORE, SLEEP, STATES, SUSPEND, FLAG, TOKEN, DATE_TIME
} oldState = STORE, state = BROWSE;

static TimingState sleep_timer = TimingState(100000L);
static TimingState autoComplete_timer = TimingState(6000L);

static volatile bool control_touched = false;
static volatile bool wake_up = false;
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
static uint8_t prev_banner_mode = 0;

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

void processEncoder(uint8_t input){
    control_touched = true;
    wake_up = true;
    if (!requestForRefresh) {
        switch (state) {

            case FLAG:
            case TOKEN:
            case DATE_TIME:
            case EDIT_PROP: {
                input == DIR_CW ? INCR(prop_value, prop_max) : DECR(prop_value, prop_min);
                break;
            }

            case BROWSE: {
                if (enablePropControl) {
                    input == DIR_CW ? INCR(prop_idx, props_idx_max) : DECR(prop_idx, 0);
                }
                break;
            }

            case STATES: {
                input == DIR_CW ? INCR(state_idx, state_idx_max) : DECR(state_idx, 0);
                break;
            }

        }
    }

}

Controller::Controller(Context &_ctx, Commander &_cmd) : cmd(&_cmd), ctx(&_ctx) {}

void Controller::begin() {
    initDisplay();
#if defined(ENC1_PIN) && defined(ENC2_PIN) && !defined(I2C_CONTROLLER)
    initEncoderInterrupts();
#endif
    props_idx_max = ctx->props_size - 1;
    state_idx_max = state_count - 1;

#ifdef DEBUG
    Serial.println("CONTROLLER passed");
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
        adjustBrightness();
    };
}

void Controller::adjustBrightness() {
    oled.setBrightness(DAYLIGHT ? 255 : 0);
}

void Controller::outputHeader(bool relays) const {
    strcpy(BUFF, relays ? (const char *) ctx->RELAYS.relStatus() : ctx->id);
    padLineInBuff(BUFF, 1, 0);
    if (relays) {
        strncpy(BUFF + ctx->RELAYS.size() * 2 + 2, CUSTOM_HEADER, 3);
    }
    if (TOKEN_ENABLE || ctx->remoteMode) {
        BUFF[15] = (unsigned char) (TOKEN_ENABLE ? COM_TOKEN + 48 : (ctx->remoteMode ? ((
                (ctx->connected ? CHAR_CONNECTED : CHAR_DISCONNECTED) + (ctx->passive ? 0 : 2))) : '\0'));
    }
    oled.setTextXY(0, 0);
    oled.putString(BUFF);
}

void Controller::outputBuffCentered() {
    oled.outputTextXY(DISPLAY_BASE + 2, DISPLAY_CENTER_COL, BUFF, true, dither);
}

void Controller::setIndicators(uint8_t data) {
#ifdef I2C_CONTROLLER
    I2c.write((uint8_t)I2C_CONTROLLER_ADDR, data);
#endif
}

void Controller::process() {

    enablePropControl = ctx->canAccessLocally() || ctx->canCommunicate();

    oled.checkConnected();

    McEvent event = encoderClick.checkButton();

#ifdef I2C_CONTROLLER

    uint8_t ctrlBuff[2];
    I2c.read(I2C_CONTROLLER_ADDR, 2, ctrlBuff);

    uint8_t  inc_dec = ctrlBuff[0] & 0x03;
    if (inc_dec > 0 ){
        processEncoderInput(inc_dec == 2 ? DIR_CW : DIR_CCW);
    }

    for (uint8_t i = 0 ; i < PWR_SIZE; i++){
        uint8_t butt = (ctrlBuff[1] >> (2 * i)) & 0x03;
        if (state != SUSPEND){
            BUTTONS[i] = butt == 1 ? CLICK : (butt == 2 ? DOUBLE_CLICK : butt == 3 ? HOLD : NOTHING);
        }
        wake_up = wake_up | butt > 0;
    }

    #endif

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
                outputHeader(true);
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
                outputBuffCentered();
                outputStatus(F("State:"), state_idx);
                c_state_idx = state_idx;
            }

            if (event == HOLD) {
                cmd->disarmStateCmd(state_idx, !isDisarmedState(state_idx));
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
                outputHeader(false);
                outputDescr("FLAGS", 1);
                prop_max = FLAGS_MAX;
                prop_value = PWR_FLAGS;
                prop_min = 0;
            }

            if (prop_value != c_byte_value) {
                PWR_FLAGS = (uint8_t) prop_value;
                itoa(PWR_FLAGS, BUFF, 2);
                outputBuffCentered();
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
                outputHeader(false);
                timeDateBlock = 0;
                dateTimePartIdx = 0;
                prop_max = 1;
                prop_value = 0;
                prop_min = -1;
                DATETIME.getDateString(dateString);
                DATETIME.getTimeString(timeString);
                outputStatus(F("     "), 0);
            }

            if (prop_value != 0) {
                DATETIME.dialDateTimeString(isTime ? timeString : dateString, dateTimePartIdx, isTime, prop_value < 0,
                                            false);
                prop_value = 0;
            }

            if (test_timer(TIMER_2HZ)) {
                outputDescr(isTime ? time : date, 1);
                if (isTime)
                    strcpy(BUFF, timeString);
                else
                    strcpy(BUFF, dateString);

                if (flash_symm(timerCounter_4Hz)) {
                    DATETIME.screenDateTimePart(BUFF, dateTimePartIdx);
                }
                outputBuffCentered();
            }

            if (event == HOLD) {
                DATETIME.setTimeFromString(timeString);
                DATETIME.setDateFromString(dateString);
                goToBrowse();
            }

            if (event == CLICK) {
                dateTimePartIdx++;
                if (dateTimePartIdx == 3) {
                    dateTimePartIdx = 0;
                    timeDateBlock++;
                    if (timeDateBlock == 2) {
                        timeDateBlock = 0;
                    }
                }
            }


            if (sleep_timer.isTimeAfter(true)) {
                timeDateBlock++;
                sleep_timer.reset();
                if (timeDateBlock = 2) {
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
                outputHeader(false);
                outputDescr("TOKEN", 1);
                prop_max = TOKEN_MAX;
                prop_value = COM_TOKEN;
                prop_min = 0;
            }

            if (prop_value != c_byte_value) {
                COM_TOKEN = (uint8_t) prop_value;
                itoa(COM_TOKEN, BUFF, 10);
                outputBuffCentered();
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
                if (prev_banner_mode != BANNER.mode) {
                    oled.clearDisplay();
                    prev_banner_mode = BANNER.mode;
                }
                switch (BANNER.mode) {
                    case 0: {
                        strcpy(BUFF, BANNER.data.text);
                        outputBuffCentered();
                        break;
                    }
                    case 1:
                    case 2:
                    case 3:
                    case 4: {
                        for (int8_t i = 0; i < (BANNER.mode > 2 ? BANNER.mode - 2 : BANNER.mode); ++i) {
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
                            if (BANNER.mode == 1) {
                                outputBuffCentered();
                            } else {
                                padLineCenteredInBuff();
                                oled.setTextXY(row + (direction * 2), 0);
                                oled.putString(BUFF);
                                if (BANNER.mode > 2) {
                                    strcpy(BUFF, EXTRA_BUFF[i]);
                                    padLineCenteredInBuff();
                                    oled.setTextXY(row + (direction * 3), 0);
                                    oled.putString(BUFF);
                                }
                            }
                        }
                        break;
                    }
                }
            }

            // Exit SLEEP state on event
            if (event == CLICK) {
                switchDisplay(true);
                goToBrowse();
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

            if (event == CLICK || wake_up) {
                wake_up = false;
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
            outputHeader(true);
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
            outputHeader(true);
        }
    }
    return fr;
}

void Controller::goToBrowse() const {
    // invalidate cache
    c_prop_idx = -1;
    c_prop_value = -1;
    c_byte_value = 255;

    dither = false;
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
    uint8_t prop_size = static_cast<uint8_t>(val > 0 ? log10((double) val) + 1 : (val == 0 ? 1 : log10((double) -val) +
                                                                                                 2));
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

void Controller::padLineCenteredInBuff() {
    const uint8_t text_size = strlen(BUFF);
    const uint8_t text_start = (LINE_SIZE - text_size) / 2;
    for (uint8_t i = 0; i < LINE_SIZE; i++) {
        if (i < text_size) {
            uint8_t text_base = text_size - 1 - i;
            BUFF[text_base + text_start] = BUFF[text_base];
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
    outputBuffCentered();
}

#if defined(ENC1_PIN) && defined(ENC2_PIN) && !defined(I2C_CONTROLLER)

ISR(PCVECT) {
    unsigned char input = encoder.process();
    if (input != NOTHING) {
        processEncoder(input);
    }
}

#endif

#endif // NO_CONTROLLER
