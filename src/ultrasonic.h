//
// Created by SHL on 15.02.2019.
//

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include "commons.h"
#include "sensors.h"

class Ultrasonic{
public:
    void begin(uint8_t n);
    uint16_t getDistance();
private:
    uint8_t trigger_pin;
    uint8_t echo_pin;
};

extern Ultrasonic ULTRASONIC;

#endif //ULTRASONIC_H
