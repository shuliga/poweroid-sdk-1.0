#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define SIGNATURE_SIZE 4

struct Persistence {

    const char *const ORIGIN = "EEPROM";
    uint8_t size;
    char signature[SIGNATURE_SIZE];
    const char *given_sign_chr;

    Property *props;

    Persistence(const char *_sign, Property *props, uint8_t props_size);

    void storeProperties(Property *props);

    void storeState(uint8_t id, bool state);

    void storeProperty(uint8_t i, long val);

    void storeFlags();

    void storeToken();

    bool loadState(uint8_t id);

    void loadProperties(Property *prop);

    void checkFactoryReset(Property *props);

    void begin();
};


#endif
