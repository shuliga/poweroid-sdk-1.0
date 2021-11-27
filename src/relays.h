//
// Created by SHL on 25.03.2017.
//

#ifndef POWEROID_10_RELAYS_H
#define POWEROID_10_RELAYS_H

#include "commons.h"

#ifndef RELAY_ON_LOW
#define RELAY_ON_LOW false
#endif

#ifndef MINI
const uint8_t OUT_PINS[] = {PWR_A_PIN, PWR_B_PIN};
#else
const uint8_t OUT_PINS[] = {PWR_A_PIN};
#endif
#define REL_COUNT ARRAY_SIZE(OUT_PINS)

#define VIRTUAL_RELAYS REL_COUNT

#define REL_A 0
#ifndef MINI
#define REL_B 1
#endif

#define ALL_RELAYS REL_COUNT + VIRTUAL_RELAYS

class Relays {

private:

    static bool powered[ALL_RELAYS];

    static uint8_t mappings[VIRTUAL_RELAYS];

public:

    bool mapped = true;

    void power(uint8_t i, bool _power);

    uint8_t size();

    void shutDown();

    char * printRelay(uint8_t idx);

    unsigned char * relStatus();

    void castRelay(uint8_t idx);

    void castMappedRelays();

    int8_t getMappedFromVirtual(uint8_t i);

    bool isPowered(uint8_t idx);

};


#endif //POWEROID_10_RELAYS_H
