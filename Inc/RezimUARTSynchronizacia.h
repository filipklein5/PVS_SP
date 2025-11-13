#ifndef REZIMUARTSYNCHRONIZACIA_H
#define REZIMUARTSYNCHRONIZACIA_H

#include "mbed.h"
#include "Rezim.h"

class RezimUARTSynchronizacia : public Rezim {
private:
    PinName pins[8];
    DigitalOut* led[8];
    UnbufferedSerial* peerSerial;
    UnbufferedSerial* konzola;
    uint8_t stavLED;
    std::chrono::milliseconds poslednyTx;
    uint32_t txIntervalMs;

public:
    RezimUARTSynchronizacia(PinName piny[8], UnbufferedSerial* peer_serial, UnbufferedSerial* konzola_uart, uint32_t txInterval);
    ~RezimUARTSynchronizacia() override;

    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override;

    void stlacenieKratke() override {}
    void stlacenieDlhe() override {}
};

#endif
