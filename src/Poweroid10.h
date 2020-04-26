#ifndef POWEROID_SDK_10_H
#define POWEROID_SDK_10_H

#include "commons.h"
#include "context.h"
#include "properties.h"
#include "relays.h"
#include "sensors.h"
#include "bluetooth.h"
#include "context.h"
#include "commander.h"
#include "controller.h"
#include "indicators.h"
#ifdef RTCM
#include "datetime.h"
#endif

#define SDK_VERSION "1.0"

const char SIGNATURE[] = "PWR";

extern void applyTimings();
extern void fillOutput();
extern void processSensors();
extern void runPowerStates();

const int DEFAULT_BAUD = 9600;

class Pwr {
public:
    Context *CTX;
    Relays *REL;
    Sensors *SENS;
    Commander *CMD;
    Controller *CTRL;

    Bt *BT;

    Pwr(Context &ctx, Commander *_cmd, Controller *_ctrl, Bt *_bt);

    void begin();

    void run();

    void printVersion();

    void power(uint8_t i, bool i1);

private:

    bool firstRun = true;

    void initPins();

    void processChangedStates();

    void loadDisarmedStates();

};

#ifdef DEBUG
void initTimer();
#endif

#endif //POWEROID_SDK_10_H
