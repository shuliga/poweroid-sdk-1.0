//
// Created by SHL on 26.03.2017.
//

#ifndef POWEROID_10_CONTEXT_H
#define POWEROID_10_CONTEXT_H

#include "sensors.h"
#include "relays.h"
#include "persistence.h"

typedef struct RunState {
    RunState() = default;

    uint8_t idx;
    char *name;
    char *state;

};

extern const uint8_t state_count;
extern bool changedState[];

void disarmState(uint8_t i, bool _disarm);
RunState *getState(uint8_t i);

typedef struct Context {
    Context(const char *_signature, const char *_id,
            Property *_factory_props, const uint8_t _props_size, int8_t _defaultPropIdx)
            : signature(_signature), PROPERTIES(_factory_props),
              props_size(_props_size), id(_id), SENS(), RELAYS(),
              PERS(Persistence(_signature, _factory_props, _props_size)),
              props_default_idx(_defaultPropIdx) {
        sprintf(version, "%s %s-%s", id, signature, PWR_BOARD_VERSION);
    }

    const char *signature;
    char version[32];

    Sensors SENS;
    Relays RELAYS;

    Property *PROPERTIES;
    Property remoteProperty;
    uint8_t props_size;
    const char *id;

    Persistence PERS;

    int8_t props_default_idx;
    bool hasToken;
    bool refreshProps;
    bool propsUpdated;
    bool refreshState;
    bool passive;
    bool connected = false;
    bool remoteMode = false;
    int8_t peerFound;

    boolean canAccessLocally(){
        return !remoteMode || !passive;
    }

    boolean canCommunicate(){
        return connected || peerFound;
    }

    boolean canRespond(){
        return !TOKEN_ENABLE || hasToken;
    }

};

#endif //POWEROID_10_CONTEXT_H
