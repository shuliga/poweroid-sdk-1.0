//
// Created by SHL on 15.02.2019.
//
// Applies to sensor DYP-ME007Y
//

#include "ultrasonic.h"

#define US_TIMEOUT 20600 // approx equivalent to 3,5m (7,0m round trip) distance, MAX for current sensor
#define US_MS_TO_CM_RATIO 58.82
#define ECHO_DELAY 50

void Ultrasonic::begin(uint8_t n){
    trigger_pin = INA_PINS[n];
    echo_pin = IN_PINS[n];
    pinMode(trigger_pin, OUTPUT);
    digitalWrite(trigger_pin, LOW);
    pinMode(echo_pin, INPUT);

}
/*
 * Returns distance in cm
 */
uint16_t Ultrasonic::getDistance(){
    delayMicroseconds(ECHO_DELAY);
    cli();
    digitalWrite(trigger_pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger_pin, LOW);
    unsigned long echo = pulseIn(echo_pin, HIGH, US_TIMEOUT);
    sei();
    return echo / US_MS_TO_CM_RATIO + 3;
}

Ultrasonic ULTRASONIC;