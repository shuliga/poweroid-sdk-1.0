//
// Created by SHL on 15.02.2019.
//

#include "ultrasonic.h"

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
    long echo = pulseInLong(echo_pin, HIGH);
    return echo / 56.5;
}

Ultrasonic ULTRASONIC;