//
// Created by SHL on 25.03.2017.
//

#include "relays.h"
#include "commands.h"

const char * REL_FMT="Rel[%i]: %i";
unsigned char status[6] = ".. ..";

bool Relays::powered[RELAYS];

#ifndef MINI
int8_t Relays::mappings[VIRTUAL_RELAYS] = {2, 3};
#else
int8_t Relays::mappings[VIRTUAL_RELAYS] = {2};
#endif

void Relays::power(uint8_t i, bool _power)
{
    if (i < size() && powered[i] != _power)
    {
        powered[i] = _power;

#ifndef SSERIAL
        digitalWrite(OUT_PINS[i], static_cast<uint8_t>(_power != RELAY_ON_LOW));
#endif
        int8_t mappedIdx = mappings[i];
        if (mapped && mappedIdx >= 0)
        {
            powered[mappedIdx] = _power;
            castRelay(static_cast<uint8_t>(mappedIdx));
        }
    }
}

uint8_t Relays::size()
{
    return ARRAY_SIZE(OUT_PINS);
}

void Relays::castRelay(uint8_t idx){
    sprintf(BUFF, "%i: %i", idx, powered[idx]);
    printCmd(cu.cmd_str.CMD_SET_RELAY, BUFF);
}

unsigned char * Relays::relStatus()
{
    const uint8_t r_size = mapped ? RELAYS : size();
    for(uint8_t i = 0; i < r_size; ++i)
    {
        status[i + (i >= size() ? 1 : 0)] = (powered[i] ? (unsigned char) 128 : (unsigned char)127);
    }
    return status;
}

void Relays::shutDown()
{
    for(uint8_t i=0; i < size(); ++i)
    {
        digitalWrite(OUT_PINS[i], RELAY_ON_LOW);
    }
}

void Relays::castMappedRelays(){
    const uint8_t r_size = mapped ? RELAYS : size();
    for(uint8_t i = size(); i < r_size; ++i) {
        castRelay(i);
        delay(50);
    }
}

int8_t Relays::getMappedFromVirtual(uint8_t i) {
    for (uint8_t idx = 0; idx < VIRTUAL_RELAYS; idx++) {
        int8_t mappedRelay = mappings[idx];
        if (mappedRelay == i) return idx;
    }
    return -1;
}

bool Relays::isPowered(uint8_t idx){
    return powered[idx];
}

char * Relays::printRelay(uint8_t idx)
{
    sprintf(BUFF, REL_FMT, idx, powered[idx]);
#ifdef SSERIAL
    SSerial.println(BUFF);
    SSerial.flush();
#endif
    return BUFF;
}
