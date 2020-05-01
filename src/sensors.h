//
// Created by SHL on 20.03.2017.
//

#ifndef _SENSORS_H
#define _SENSORS_H

#include "commons.h"
#include "timings.h"

#define DHTTYPE DHT22

#ifndef DHT_PIN
#define DHT_PIN IN1_PIN
#endif

#define SEN_1 0
#define SEN_2 1
#define SEN_3 2

#define MAX_ANALOG_VALUE 1023

extern const uint8_t IN_PINS[];
extern const uint8_t INA_PINS[];

class Sensors {
public:

    const char *const ORIGIN = "SENSORS";

    void updateDHT();

    float getTemperature() const;

    float getHumidity() const;

    void process();

    bool checkInstalled(uint8_t pin, bool inst);

    void initSensors();

    void setInstalled();

    bool isDhtInstalled();

    bool isSensorOn(uint8_t index);

    uint16_t getSensorVal(uint8_t index);

    bool isSensorVal(uint8_t index, uint8_t val);

    uint8_t size();

    const char *printDht();

    void setDHT(int8_t _temp, uint8_t _humid);

    const char *printSensor(uint8_t i);

    const int16_t getNormalizedSensor(uint8_t i, int16_t min, int16_t max, uint16_t raw_min = 0, uint16_t  raw_max = MAX_ANALOG_VALUE);

    int8_t getInt(float f) const;

private:

    float temp;

    float humid;

    void searchDHT();


    void updateDhtDirect();
};


#endif //_SENSORS_H
