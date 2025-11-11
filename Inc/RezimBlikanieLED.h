#pragma once
#include "mbed.h"
#include "Rezim.h"

class RezimBlikanieLED : public Rezim {
private:
    PinName pins[8];
    DigitalOut* led[8];
    BufferedSerial* konzola;
    uint32_t intervalMs;
    uint8_t pattern;
    int rezimPatternu;
    std::chrono::milliseconds poslednyMs;

public:
    RezimBlikanieLED(PinName* piny, BufferedSerial* konzola_uart, uint32_t interval);
    ~RezimBlikanieLED() override;

    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override {}
    void stlacenieKratke() override {}
    void stlacenieDlhe() override {}
};
