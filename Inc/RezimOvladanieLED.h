#ifndef REZIMOVLADANIELED_H
#define REZIMOVLADANIELED_H

#include "mbed.h"
#include "Rezim.h"

class RezimOvladanieLED : public Rezim {
private:
    PinName pins[3];
    PwmOut* led[3];
    BufferedSerial* konzola;

    uint8_t jas[3];
    int vybranaLED;
    uint8_t krok;

public:
    RezimOvladanieLED(PinName piny[3], BufferedSerial* konzola_uart);
    ~RezimOvladanieLED() override;

    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override;

    void stlacenieKratke() override;
    void stlacenieDlhe() override;
    void nastavJas(uint8_t idx, uint8_t hod);
};

#endif // REZIMOVLADANIELED_H
