#ifndef BT_H
#define BT_H

#include "context.h"

#ifdef SSERIAL
#include <SoftwareSerial.h>
#endif

#define HC_05_AT_BAUD 38400
#define HC_05_AT_BAUD_FAST 57600
#define HC_05_AT_UART "AT+UART="
#define HC_05_AT_BAUD_AT HC_05_AT_UART STRINGIZE(HC_05_AT_BAUD_FAST) ",1,0"
#define HC_06_BAUD 115200
#define HC_12_AT_BAUD 9600
#define HC_12_BAUD 19200

#define HIGH_SPEED_BAUD 115200
#define LOW_SPEED_BAUD 9600


#define BT_VER_05 "+VERSION:hc01.comV2.1"
#define BT_VER_06 "hc01.comV2.0"
#define PASSWD "1234"


#define HISPEED 115200

const int TIMEOUT_STEP = 100;

class Bt {

public:
    const char *name;
    bool remote_on;
    bool host;

  Bt(const char *id);

  void begin();

private:

    String execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout);
    String execBtAtCommand(const __FlashStringHelper *cmd);

    void applyBt05();

    void execReset();

    bool checkPeerType(const char *conn_type);

    void cleanSerial() const;

    String execCommand(const __FlashStringHelper *fshcmd, const char *cmd2, bool crlf);

    void pushCommand(const __FlashStringHelper *fshcmd, const char *cmd2, bool crlf);
};

#endif // BT_H
