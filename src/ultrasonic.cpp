//
// Created by SHL on 15.02.2019.
//

#include "ultrasonic.h"

#define US_TIMEOUT 20000 // approx equivalent to 3,5 m distance, MAX for current sensor
#define US_MS_TO_CM_RATIO 56.5

void Ultrasonic::begin(uint8_t n){
    trigger_pin = INA_PINS[n];
    echo_pin = IN_PINS[n];
    pinMode(trigger_pin, OUTPUT);
    pinMode(echo_pin, INPUT);

}

uint16_t Ultrasonic::getDistance(){
    delayMicroseconds(10);
    digitalWrite(trigger_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger_pin, LOW);
    unsigned long echo = pulseInLong(echo_pin, HIGH, US_TIMEOUT);
    return echo / US_MS_TO_CM_RATIO;
}

Ultrasonic ULTRASONIC;