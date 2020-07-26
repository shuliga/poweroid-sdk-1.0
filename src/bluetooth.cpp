#include <avr/wdt.h>
#include "bluetooth.h"
#include "commons.h"

/*
 * Log codes
 *
 * 100 Command echo
 * 101 Command echo, passive mode
 * 200 Server mode
 * 210 Client mode (speed)
 * 211 Connected to host
 * 410 Disconnected
*/
static const char *ORIGIN = "BT";

Bt::Bt(const char *id) {
    name = id;
}

void Bt::begin() {

    delay(2500);

    if(REMOTE_ENABLE){
        if(REMOTE_SERVER){
            Serial.begin(LOW_SPEED ? LOW_SPEED_BAUD : HC_06_BAUD);

            if(!TOKEN_ENABLE){
                String ver = execCommand(F("AT+VERSION"), 0, false);
                if (!ver.startsWith(BT_VER_05)) {
                    execCommand(F("AT+NAME"), name, false);
                }
            }

            host = true;
        } else {
            Serial.begin(HC_05_AT_BAUD);
//
//            writeLog('I', ORIGIN, 210, HC_05_AT_BAUD);
//
//            Serial.println("AT");
//            delay(200);
//            cleanSerial();
//
//            String ver = execBtAtCommand(F("AT+VERSION"));
//            if (ver.startsWith(BT_VER_05)) {
//                applyBt05();
//            }
        }
        remote_on = true;
        Serial.end();
        Serial.begin(HIGH_SPEED_BAUD);
    } else {
        Serial.begin(LOW_SPEED_BAUD);
    }
    Serial.println();
#ifdef DEBUG
    Serial.println("BT passed");
#endif
}

void Bt::cleanSerial() const {
    while (Serial.available()) { Serial.read(); } // Cleanup rotten UART buffer

}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd) {
    return execBtAtCommand(cmd, 0, 0);
}

String Bt::execBtAtCommand(const __FlashStringHelper *cmd, const char *cmd2, unsigned long timeout) {
#ifdef SSERIAL
    SSerial.print("AT_CMD->");
    SSerial.println(cmd);
#endif

    pushCommand(cmd, cmd2, true);

    unsigned long t = Serial.getTimeout();
    Serial.setTimeout(timeout == 0 ? t : timeout );

    String res = Serial.readString();
    while(Serial.available()){
#if defined(SSERIAL) and defined(DEBUG)
        SSerial.println(Serial.readString());
#else
        Serial.readString();
#endif
    }

#if defined(SSERIAL) and defined(DEBUG)
    SSerial.println(res);
#endif

    Serial.setTimeout(t);
    return res;
}

void Bt::applyBt05() {
#ifdef WATCH_DOG
        wdt_reset();
#endif

//    execBtAtCommand(F("AT+ORGL"));
    execBtAtCommand(F("AT+NAME="), name, 200);
    execBtAtCommand(F("AT+RMAAD"));
    execBtAtCommand(F("AT+ROLE=1")); // ser role to master
    execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output low level indicates successful connection
//    execBtAtCommand(F(HC_05_AT_BAUD_AT)); // Set baud rate
    execReset();
    execBtAtCommand(F("AT+CMODE=0")); // connect only to predefined address
    execBtAtCommand(F("AT+CLASS=1F00")); // iquire only devices with type 1F00
    execBtAtCommand(F("AT+INQM=1,2,3")); // inquire rssi mode, 2 items, timeout 1.28 * 3 sec

    String devices = execBtAtCommand(F("AT+INQ"), 0, 4000); // Inquire devices, until OK

    int dp1 = devices.indexOf('\r');
    if (dp1 < 0)
        return;

    String device = devices.substring(0, (unsigned int) dp1);
#ifdef SSERIAL
    SSerial.println();
    SSerial.print("Device found: ");
    SSerial.println(device);
#endif
    if (device.startsWith("+INQ:")) {
#ifdef WATCH_DOG
        wdt_reset();
#endif
        String peer_address = device.substring((unsigned int) (device.indexOf(':') + 1), (unsigned int) device.indexOf(','));
        peer_address.replace(':', ',');

        execBtAtCommand(F("AT+PSWD="), PASSWD, 0);
        execBtAtCommand(F("AT+BIND="), peer_address.c_str(), 0);
        execBtAtCommand(F("AT+POLAR=1,1")); // PIN09 output HIGH level indicates successful connection
        execReset();
        execBtAtCommand(F("AT+POLAR=1,0")); // PIN09 output LOW level indicates successful connection
    }

}

void Bt::execReset() {
    execBtAtCommand(F("AT+RESET"));
    delay(1000);
    execBtAtCommand(F("AT+INIT"));
}

String Bt::execCommand(const __FlashStringHelper *fshcmd,  const char *cmd2, bool crlf) {
    cleanSerial();
    pushCommand(fshcmd, cmd2, crlf);
    Serial.flush();
    delay(200);
    return Serial.readString();
}

void Bt::pushCommand(const __FlashStringHelper *fshcmd, const char *cmd2, bool crlf) {
    Serial.print(fshcmd);
    if (cmd2 != 0) {
        Serial.print(cmd2);
    }
    if (crlf) {
        Serial.println();
    }

}
