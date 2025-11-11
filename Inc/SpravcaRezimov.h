#pragma once
#include "Rezim.h"
#include "mbed.h"
#include <vector>
#include <cstdint>

class SpravcaRezimov {
private:
    std::vector<Rezim*> zoznam;
    uint8_t aktualny;
    mbed::BufferedSerial* konzola;
public:
    SpravcaRezimov(mbed::BufferedSerial* konzola_uart);
    ~SpravcaRezimov();

    void pridajRezim(Rezim* r);
    void dalsiRezim();
    void vyberRezim(uint8_t idx);
    void aktualizuj();
    void stlacenieKratke();
    void stlacenieDlhe();
    void spracujUART(char c);
    uint8_t dajAktualny() const { return aktualny; }
};
