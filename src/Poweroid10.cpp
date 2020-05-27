#include "Poweroid10.h"
#include <avr/wdt.h>
#include <I2C/I2C.h>

#if defined(__AVR_ATmega1284P__)
#define CONST_4_HZ_TIMER 19530  // 65536 - 19530; // 20000000L / 256 / FRQ - 1; // set timer value 20MHz/256/4Hz-1
#define TIMER_PRESCALER (1 << CS12) | (0 << CS11) | (0 << CS10)  // 256 prescaler
#else
#define CONST_4_HZ_TIMER 62499  // 65536 - 62499; // 16000000L / 64 / FRQ - 1; // set timer value 16MHz/64/4Hz-1
#define TIMER_PRESCALER (0 << CS12) | (1 << CS11) | (1 << CS10) // 64 prescaler
#endif

#ifdef DATETIME_H
#define RAD(D) ((D) * 3.141529) / 180
#define DEG(R) ((R)  * 180.0) / 3.141529
#define SPRING_EQUINOX_DAY 80
#define NOON_HOUR 13.35

int8_t hrs = 1;
int8_t min = 0;
int8_t sec = 0;
int8_t day = 1;
int8_t month = 1;
#endif //DATETIME_H


volatile uint8_t timerCounter = 0;
uint8_t pastTimerCounter = 0;

uint8_t timerFlags;
uint8_t timerCounter_1Hz;
uint8_t timerCounter_4Hz;

void initTimer_1() {
    cli();

    TCCR1A = 0;
    TCCR1B = 0;

    OCR1A = CONST_4_HZ_TIMER;

    TCCR1B |= (1 << WGM12);                 // turn on CTC mode. clear timer on compare match
    TCCR1B |= TIMER_PRESCALER;              // pre-scaler
    TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt

    sei();

}

void setTimerFlags() {
    if (timerCounter != pastTimerCounter) {
        for (uint8_t i = 0; i < TIMERS_COUNT; i++) {
            timerFlags = timerCounter % (1U << i) == 0 ? timerFlags | 1U << i : timerFlags;
        }
        pastTimerCounter = timerCounter;
    }
}

void setDaylight(){
    sec = RTC.get(DS1307_SEC, true);
    min = sec == 0 ? RTC.get(DS1307_MIN, false) : min;
    hrs = min == 0 ? RTC.get(DS1307_HR, false) : hrs;
    day = hrs == 1 ? RTC.get(DS1307_DATE, false) : day;
    month = day == 1 ? RTC.get(DS1307_MTH, false) : month;
    double sun_declination = (23 + 27.0 / 60.0) * sin(360.0 / 365.25 * (day + month * 30.42 - SPRING_EQUINOX_DAY));
    double ha = DEG(acos(cos(RAD(90.833)) / (cos(RAD(LATITUDE)) * cos(RAD(sun_declination))) -
                         tan(RAD(LATITUDE)) * tan(RAD(sun_declination))));
    double hrs_float = hrs + min / 60.0 + sec / 3600.0;
    DAYLIGHT = hrs_float >= NOON_HOUR - (ha / 15) && hrs_float <= NOON_HOUR + (ha / 15);
}

void setFlashCounters() {
    if (test_timer(TIMER_1HZ)) {
        timerCounter_1Hz++;
        timerCounter_1Hz = timerCounter_1Hz >= TIMERS_FLASH_COUNTS ? 0 : timerCounter_1Hz;
#ifdef DATETIME_H
        setDaylight();
#endif
    }

    if (test_timer(TIMER_4HZ)) {
        timerCounter_4Hz++;
        timerCounter_4Hz = timerCounter_4Hz >= TIMERS_FLASH_COUNTS ? 0 : timerCounter_4Hz;
    }

}

Pwr::Pwr(Context &ctx, Commander *_cmd, Controller *_ctrl, Bt *_bt) : CTX(&ctx), CMD(_cmd), CTRL(_ctrl), BT(_bt) {
    REL = &ctx.RELAYS;
    SENS = &ctx.SENS;
}

void Pwr::begin() {

#ifdef WATCH_DOG
    wdt_disable();
#endif

    I2c.begin();
    I2c.setSpeed(true);

    Serial.begin(DEFAULT_BAUD);

    CTX->PERS.begin();

    BT->begin();
    CTX->passive = !BT->host;
    CTX->remoteMode = BT->remote_on;

    initPins();

#ifdef SSERIAL
    SSerial.begin(DEFAULT_BAUD);
    SSerial.println("SSerial-started");
#endif

    printVersion();

#ifdef WATCH_DOG
    wdt_enable(WDTO_8S);
#endif

    SENS->initSensors();

    loadDisarmedStates();

    REL->mapped = !CTX->passive;

#ifdef DEBUG
    writeLog('I', "PWR", 200 + CTX->passive, (unsigned long)0);
#endif

#ifdef WATCH_DOG
    wdt_enable(WDTO_2S);
#endif

#ifndef NO_CONTROLLER
    CTRL->begin();
#endif

    Serial.setTimeout(SERIAL_READ_TIMEOUT);

    initTimer_1();
}

void Pwr::run() {

    setTimerFlags();
    setFlashCounters();

    applyTimings();

#ifdef WATCH_DOG
    wdt_reset();
#endif

    bool newConnected = false;
    bool updateConnected = false;

    SENS->process();

    processSensors();

    CMD->listen();

    if (BT && CTX->remoteMode) {
        newConnected = CMD->isConnected();
        updateConnected = newConnected != CTX->connected;
        CTX->refreshState = CTX->refreshState || updateConnected;
        CTX->connected = newConnected;
    }

    if (CTX->canAccessLocally() && test_timer(TIMER_2HZ)) {
        fillOutput();
    }

#ifndef NO_CONTROLLER
    CTRL->process();
    if (!(timerCounter_1Hz % (TIMERS_FLASH_COUNTS - 1))) {
        CTRL->adjustBrightness();
    }
#endif

    if (updateConnected && newConnected && CTX->canRespond()) {
        REL->castMappedRelays();
    }
    if (firstRun) {
#ifdef DEBUG
        writeLog('I', SIGNATURE, 100 + CTX->passive, (unsigned long)0);
#endif
        firstRun = false;
    }

#ifndef CONTROLLER_ONLY
    runPowerStates();
#endif

    if (CTX->remoteMode && CTX->passive && !CTX->connected) {
        REL->shutDown();
    }

    processChangedStates();

    timerFlags = 0;
    CTX->propsUpdated = false;
}

void Pwr::printVersion() {
    if (CTX->canRespond()) {
        Serial.println(CTX->version);
    }
}

void Pwr::initPins() {

#ifndef SPI
    pinMode(LED_PIN, OUTPUT);
#endif

    for (uint8_t i = 0; i < REL->size(); i++) {
        pinMode(OUT_PINS[i], OUTPUT);
    }
    REL->shutDown();

    for (uint8_t i = 0; i < SENS->size(); i++) {
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }

#ifdef INDICATORS_H
    INDICATORS.init();
#endif
}

void Pwr::loadDisarmedStates() {
    for (uint8_t i = 0; i < state_count; i++) {
        bool disarm = CTX->PERS.loadState(i);
        disarmState(i, disarm);
#ifdef DEBUG
        if (disarm) {
            Serial.println(printState(i));
        }
#endif
    }
    CTX->refreshState = true;
}

void Pwr::power(uint8_t i, bool power) {
    if (CTX->canAccessLocally()) {
        CTX->refreshState = CTX->refreshState || (REL->isPowered(i) != power);
        REL->power(i, power);
    }
}

void Pwr::processChangedStates() {
    for (uint8_t i = 0; i < state_count; i++) {
        if (changedState[i]) {
            Serial.println(CMD->printState(i));
            changedState[i] = false;
        }
    }
}

ISR(TIMER1_COMPA_vect) {
    timerCounter++;
    timerCounter = timerCounter > TIMER_COUNTER_MAX ? 0 : timerCounter;
}
