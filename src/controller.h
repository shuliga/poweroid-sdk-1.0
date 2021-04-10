//
// Created by SHL on 20.03.2017.
//

#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#define FLIP_DISPLAY 1

static const int ONE_LINE = 16;

#include <MultiClick.h>
#include "commons.h"
#include "context.h"
#include "commander.h"

class Controller {
public:

    McEvent BUTTONS[PWR_SIZE];

    Controller(Context &_ctx, Commander &_cmd);

    void begin();

    void process();

    void adjustBrightness();

    static void setIndicators(uint8_t data);

private:
    Context *ctx;
    Commander *cmd;

    void initEncoderInterrupts();

    void initDisplay();

    void outputPropDescr(const char * _buff);

    void outputDescr(const char *_buff, uint8_t lines) const;

    void outputStatus(const __FlashStringHelper *txt, const long val);

    void outputPropVal(uint8_t measure_idx, int16_t _prop_val, bool brackets, bool measure);

    void switchDisplay(boolean inverse) const;

    void updateProperty(uint8_t idx) const;

    bool loadProperty(uint8_t idx) const;

    void goToEditProp(uint8_t i) const;

    bool firstRun() const;

    void goToBrowse() const;

    bool testControl(TimingState &timer) const;

    void copyProperty(Property &prop, uint8_t idx) const;

    void outputHeader(bool relays) const;

    bool canGoToEdit();

    static void padLineInBuff(char *_buff, uint8_t lines, uint8_t tail);

    static void padLineCenteredInBuff();

    static uint8_t normalizeGauge(uint16_t val, uint16_t min, uint16_t max);

    static void outputBuffCentered();

};


#endif //_CONTROLLER_H
