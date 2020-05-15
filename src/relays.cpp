//
// Created by SHL on 25.03.2017.
//

#include "relays.h"
#include "commands.h"

static const char * REL_FMT="Rel[%i]: %i";

#ifndef MINI
static unsigned char status[6] = ".. ..";
#else
static unsigned char status[4] = ". .";
#endif

bool Relays::powered[ALL_RELAYS];

#ifndef MINI
uint8_t Relays::mappings[VIRTUAL_RELAYS] = {2, 3};
#else
uint8_t Relays::mappings[VIRTUAL_RELAYS] = {2};
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
        if (mapped && mappedIdx >= REL_COUNT)
        {
            powered[i + REL_COUNT] = _power;
            castRelay(static_cast<uint8_t>(mappedIdx));
        }
    }
}

uint8_t Relays::size()
{
    return REL_COUNT;
}

void Relays::castRelay(uint8_t idx){
    sprintf(BUFF, "%i: %i", idx, powered[idx]);
    printCmd(cu.cmd_str.CMD_SET_RELAY, BUFF);
}

unsigned char * Relays::relStatus()
{
    const uint8_t r_size = mapped ? ALL_RELAYS : REL_COUNT;
    for(uint8_t i = 0; i < r_size; ++i)
    {
        status[i + (i >= REL_COUNT ? 1 : 0)] = (powered[i] ? (unsigned char) 128 : (unsigned char)127);
    }
    return status;
}

void Relays::shutDown()
{
    for(uint8_t i=0; i < REL_COUNT; ++i)
    {
        digitalWrite(OUT_PINS[i], RELAY_ON_LOW);
    }
}

void Relays::castMappedRelays(){
    const uint8_t r_size = mapped ? ALL_RELAYS : REL_COUNT;
    for(uint8_t i = size(); i < r_size; ++i) {
        castRelay(i);
        delay(50);
    }
}

int8_t Relays::getMappedFromVirtual(uint8_t idx) {
    for (uint8_t i = 0; i < VIRTUAL_RELAYS; ++i) {
        int8_t mappedRelay = mappings[i];
        if (mappedRelay == idx) return i;
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
