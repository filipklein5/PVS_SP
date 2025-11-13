#pragma once
#include "Rezim.h"
#include "mbed.h"
#include <vector>

class SpravcaRezimov {
private:
    std::vector<Rezim*> zoznam;
    uint8_t aktualny;
    UnbufferedSerial* konzola;

public:
    SpravcaRezimov(UnbufferedSerial* konzola_uart);
    ~SpravcaRezimov();

    void pridajRezim(Rezim* r);
    void dalsiRezim();
    void vyberRezim(uint8_t idx);
    void aktualizuj();
    void stlacenieKratke();
    void stlacenieDlhe();
    void spracujUART(char c);

    Rezim* getAktualnyRezim() { 
        if (zoznam.empty()) return nullptr; 
        return zoznam[aktualny]; 
    }

    uint8_t getAktualnyIdx() const { return aktualny; }
};
