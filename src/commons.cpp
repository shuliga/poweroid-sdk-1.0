#include "commons.h"

#ifdef WATCH_DOG
#include <avr/wdt.h>
#endif

#ifdef SSERIAL
AltSoftSerial SSerial = AltSoftSerial();
#endif

unsigned long hash(byte *data, unsigned long size) {
    unsigned long hash = 19;
    for (unsigned long i = 0; i < size; i++) {
        byte c = *data++;
        if (c != 0) { // usually when doing on strings this wouldn't be needed
            hash *= c;
        }
        hash += 7;
    }
    return hash;
}

char * idxToChar(uint8_t idx){
    static char num[5];
    return itoa(idx, num, 10);
}

uint8_t flashStringHelperToChar(const __FlashStringHelper *ifsh, char *dst) {
    PGM_P p = reinterpret_cast<PGM_P>(ifsh);
    uint8_t n = 0;
    while (1) {
        unsigned char c = (unsigned char) pgm_read_byte(p++);
        dst[n++] = c;
        if (c == 0) break;
    }
    return n;
}

void noInfoToBuff() { strcpy(BUFF, NO_INFO_STR); }


#ifdef WATCH_DOG

// Tweak to enable Watchdog working, disables watchdog after initiation
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));
void get_mcusr(void){
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}
#endif