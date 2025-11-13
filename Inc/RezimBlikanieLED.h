#pragma once
#include "mbed.h"
#include "Rezim.h"
#include "Piny.h"

class RezimBlikanieLED : public Rezim {
private:
    PinName pins[8];
    DigitalOut* led[8];
    DigitalOut* rgbLed[3];
    PinName rgbPins[3];
    bool rgbLedOwn[3];
    int selectedRGB;
    UnbufferedSerial* konzola;
    uint32_t intervalMs;
    uint8_t pattern;
    int rezimPatternu;
    std::chrono::milliseconds poslednyMs;
    bool rgbState;
    int last_print_color;
    uint32_t last_print_interval;
    uint8_t last_print_pattern;
    void printStatusIfChanged();
    bool interactiveMode;

public:
    RezimBlikanieLED(PinName* piny, UnbufferedSerial* konzola_uart, uint32_t interval);
    ~RezimBlikanieLED() override;

    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override;
    void deinit() override;
    void stlacenieKratke() override {}
    void stlacenieDlhe() override {}
};
