#ifndef COMMANDER_H
#define COMMANDER_H


class Commander {
public:

    Commander(Context &_ctx);

    void listen();

    void storeProps();

    void disarmStateCmd(uint8_t i, bool disarm);

    bool isConnected();

    const char *printState(uint8_t i);

private:

    bool connected = false;

    Context *ctx;

    const char * printProperty(uint8_t i);

    void printBinProperty(uint8_t i);

};

#endif // COMMANDER_H
