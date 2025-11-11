#include "RezimCasovacTerminal.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>

using namespace mbed;
using namespace std::chrono;

RezimCasovacTerminal::RezimCasovacTerminal(BufferedSerial* konzola_uart, uint32_t intervalMs)
    : startMs(Kernel::Clock::now()), zakladMs(0), pauza(false),
      poslednyVypisMs(Kernel::Clock::now()), intervalVypisuMs(intervalMs),
      alarmSekundy(0), alarmSpusteny(false), konzola(konzola_uart) {}

void RezimCasovacTerminal::inicializuj() {
    startMs = Kernel::Clock::now();
    zakladMs = 0;
    pauza = false;
    poslednyVypisMs = Kernel::Clock::now();
    alarmSpusteny = false;
    if (konzola) {
        const char* msg =
            "Rezim: Casovac (terminal)\r\n"
            "Prikazy: 'p' - pauza/obnov, 'r' - reset, 'a' + cislo + ENTER - nastav alarm (napr. a30<enter>)\r\n";
        konzola->write(msg, strlen(msg));
    }
}

uint32_t RezimCasovacTerminal::dajUbehnuteSekundy() const {
    if (pauza) return zakladMs / 1000;
    auto now = Kernel::Clock::now();
    return (zakladMs + duration_cast<milliseconds>(now - startMs).count()) / 1000;
}

void RezimCasovacTerminal::aktualizuj() {
    auto now = Kernel::Clock::now();
    uint32_t elapsedMs = zakladMs + (pauza ? 0 : duration_cast<milliseconds>(now - startMs).count());

    auto delta = duration_cast<milliseconds>(now - poslednyVypisMs).count();
    if (delta >= intervalVypisuMs) {
        poslednyVypisMs = now;
        uint32_t s = elapsedMs / 1000;
        uint32_t hrs = s / 3600;
        uint32_t mins = (s % 3600) / 60;
        uint32_t secs = s % 60;
        if (konzola) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "CAS %02lu:%02lu:%02lu\r\n", (unsigned long)hrs, (unsigned long)mins, (unsigned long)secs);
            konzola->write(buf, n);
        }
    }

    if (alarmSekundy > 0 && !alarmSpusteny && (elapsedMs / 1000) >= alarmSekundy) {
        alarmSpusteny = true;
        if (konzola) {
            const char* am = "!! ALARM: dosiahnuty nastaveny cas !!\r\n";
            konzola->write(am, strlen(am));
        }
    }
}

void RezimCasovacTerminal::spracujUART(char c) {
    static char numbuf[12];
    static uint8_t ni = 0;

    if (c == 'p') {
        pauza = !pauza;
        if (pauza) {
            auto now = Kernel::Clock::now();
            zakladMs += duration_cast<milliseconds>(now - startMs).count();
        } else {
            startMs = Kernel::Clock::now();
        }
        if (konzola) {
            const char* msg = pauza ? "STOPNUTY\r\n" : "POKRAČUJE\r\n";
            konzola->write(msg, strlen(msg));
        }
    } else if (c == 'r') {
        startMs = Kernel::Clock::now();
        zakladMs = 0;
        pauza = false;
        alarmSpusteny = false;
        if (konzola) {
            const char* msg = "RESET CASOVACA\r\n";
            konzola->write(msg, strlen(msg));
        }
    } else if (c == 'a') {
        ni = 0;
        memset(numbuf, 0, sizeof(numbuf));
    } else if (c >= '0' && c <= '9') {
        if (ni < sizeof(numbuf) - 1) numbuf[ni++] = c;
    } else if (c == '\n' || c == '\r') {
        if (ni > 0) {
            uint32_t v = (uint32_t)atoi(numbuf);
            nastavAlarm(v);
            if (konzola) {
                char out[64];
                int n = snprintf(out, sizeof(out), "ALARM NASTAVENY NA %lu s\r\n", (unsigned long)v);
                konzola->write(out, n);
            }
            ni = 0;
            memset(numbuf, 0, sizeof(numbuf));
        }
    }
}

void RezimCasovacTerminal::stlacenieKratke() { spracujUART('p'); }
void RezimCasovacTerminal::stlacenieDlhe() { spracujUART('r'); }
