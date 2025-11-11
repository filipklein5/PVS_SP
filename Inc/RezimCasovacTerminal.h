#pragma once
#include "mbed.h"
#include "Rezim.h"
#include <chrono>

class RezimCasovacTerminal : public Rezim {
public:
    RezimCasovacTerminal(BufferedSerial* konzola_uart, uint32_t intervalMs);
    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override;
    void stlacenieKratke() override;
    void stlacenieDlhe() override;

private:
    std::chrono::time_point<Kernel::Clock> startMs;
    uint32_t zakladMs;
    bool pauza;
    std::chrono::time_point<Kernel::Clock> poslednyVypisMs;
    uint32_t intervalVypisuMs;
    uint32_t alarmSekundy;
    bool alarmSpusteny;
    BufferedSerial* konzola;

    uint32_t dajUbehnuteSekundy() const;
    void nastavAlarm(uint32_t s) { alarmSekundy = s; alarmSpusteny = false; }
};
