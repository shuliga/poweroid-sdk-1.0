//
// Created by SHL on 20.03.2017.
//

#include "global.h"
#include "sensors.h"
#include "commands.h"
#include <DHT/DHT.h>
#include <stdint.h>

// Console output codes
// 100 -
// 200 - DHT installed (pin)
// 201 - Sensor installed (pin)
// 500 - DHT not installed
// 501 - Sensor not installed (pin)

const long INST_DELAY = 50L;

#ifndef MINI
const uint8_t IN_PINS[] = {IN1_PIN, IN2_PIN, IN3_PIN};
const uint8_t INA_PINS[] = {INA1_PIN, INA2_PIN, INA3_PIN};
#else
const uint8_t IN_PINS[] = {IN1_PIN, IN2_PIN};
const uint8_t INA_PINS[] = {INA1_PIN, INA2_PIN};
#endif

static const uint8_t  SENS_COUNT = ARRAY_SIZE(IN_PINS);
static bool installed[SENS_COUNT];

static bool dht_installed;
static DHT dht(DHT_PIN, DHTTYPE);


void Sensors::searchDHT() {
    dht.begin();
    float val = dht.readHumidity();
    if (!isnan(val)) {
#ifdef DEBUG
        writeLog('I', ORIGIN, 200, DHT_PIN);
#endif
        for(int i = 0; i < SENS_COUNT; i++){
            if (IN_PINS[i] == DHT_PIN){
                installed[i] = true;
                updateDhtDirect();
            }
        }
        dht_installed = true;
    }
}

void Sensors::updateDHT() {
    if (dht_installed && test_timer(TIMER_0_25HZ)) {
        updateDhtDirect();
    }
}

void Sensors::updateDhtDirect(){
    temp = dht.readTemperature();
    humid = dht.readHumidity();
}

float Sensors::getTemperature() const {
    return temp;
}

float Sensors::getHumidity() const {
    return humid;
}

bool Sensors::checkInstalled(uint8_t pin, bool inst) {
    return inst || digitalRead(pin) == LOW;
}

void Sensors::initSensors() {
#if defined(__AVR_ATmega1284P__)
#define INTERNAL_REF INTERNAL1V1
#else
#define INTERNAL_REF INTERNAL
#endif

    analogReference(REF_1V1 ? INTERNAL_REF : DEFAULT);
    for(uint8_t i = 0; i < ARRAY_SIZE(INA_PINS); i++){
        pinMode(INA_PINS[i], INPUT);
    }
    for(uint8_t i = 0; i < ARRAY_SIZE(IN_PINS); i++){
        pinMode(IN_PINS[i], INPUT_PULLUP);
    }
    delay(1000L);
    searchDHT();
}

void Sensors::process() {
    setInstalled();
    updateDHT();
}

void Sensors::setInstalled() {
    for (uint8_t i = 0; i < SENS_COUNT; i++) {
        installed[i] = checkInstalled(IN_PINS[i], installed[i]);
#ifdef DEBUG
        writeLog('I', ORIGIN, installed[i] ? 201 : 501, i);
#endif
    }
}

bool Sensors::isDhtInstalled() {
    return dht_installed;
}

bool Sensors::isSensorOn(uint8_t index) {
    return digitalRead(IN_PINS[index]) == LOW && installed[index];
}

uint16_t Sensors::getSensorVal(uint8_t index) {
    return analogRead(digitalPinToAnalogPin(INA_PINS[index]));
}

bool Sensors::isSensorVal(uint8_t index, uint8_t val) {
    return getSensorVal(index) == val;
}

uint8_t Sensors::size() {
    return ARRAY_SIZE(IN_PINS);
}

const char *Sensors::printDht() {
    if (dht_installed ) {
        sprintf(BUFF, "%i~C %i%%", getInt(temp), getInt(humid));
    } else {
        noInfoToBuff();
    }
    return BUFF;
}

int8_t Sensors::getInt(float f) const { return (int8_t) floor(f + 0.5); }

const char *Sensors::printSensor(uint8_t i) {
    sprintf(BUFF, "Sens:%i %s", i, installed[i] ? "inst" : "n/a");
    return BUFF;
}


const int16_t Sensors::getNormalizedSensor(uint8_t i, int16_t min, int16_t max, uint16_t raw_min, uint16_t  raw_max) {
    int16_t raw_val = getSensorVal(i) - raw_min ;
    float ratio = static_cast<const float >(max - min) / (raw_max - raw_min);
    return static_cast<const int16_t>((raw_val < 0.0 ? 0.0 : raw_val) * ratio + min);
}