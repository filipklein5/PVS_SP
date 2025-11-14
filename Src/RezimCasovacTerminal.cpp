#include "RezimCasovacTerminal.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <chrono>
#include "Konzola.h"

using namespace mbed;
using namespace std::chrono;

RezimCasovacTerminal::RezimCasovacTerminal(UnbufferedSerial* konzola_uart, uint32_t intervalMs) : startMs(Kernel::Clock::now()), zakladMs(0), pauza(false),
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
            "Prikazy:\r\n"
            "  p          - pauza / obnov (toggle)\r\n"
            "  r          - reset casovaca na 00:00:00\r\n"
            "  a<cislo><ENTER> - nastav alarm v sekundach (pr. a30<ENTER> nastaví alarm na 30s)\r\n"
            "Pr. postup: zadajte 'a', potom cislo a stlacte ENTER.\r\n";
        konzola_safe_write(konzola, msg, strlen(msg));
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
    if (!pauza && delta >= (int)intervalVypisuMs) {
        poslednyVypisMs = now;
        uint32_t s = elapsedMs / 1000;
        uint32_t hrs = s / 3600;
        uint32_t mins = (s % 3600) / 60;
        uint32_t secs = s % 60;
        if (konzola) {
            char buf[256];
            int n = 0;
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  REZIM: Casovac (terminal)            |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  CAS      : %02lu:%02lu:%02lu               |\r\n", (unsigned long)hrs, (unsigned long)mins, (unsigned long)secs); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  STAV     : %-24s |\r\n", (pauza ? "STOPNUTY" : "BEZI")); konzola_safe_write(konzola, buf, (size_t)n);
            if (alarmSekundy > 0) {
                uint32_t remaining = (alarmSekundy > (s) ? (alarmSekundy - s) : 0);
                uint32_t rmins = remaining / 60;
                uint32_t rsecs = remaining % 60;
                n = snprintf(buf, sizeof(buf), "|  ALARM    : %4lu s (zostava %02lu:%02lu) |\r\n", (unsigned long)alarmSekundy, (unsigned long)rmins, (unsigned long)rsecs); konzola_safe_write(konzola, buf, (size_t)n);
            } else {
                n = snprintf(buf, sizeof(buf), "|  ALARM    : (nezadany)                 |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            }
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
        }
    }

    if (alarmSekundy > 0 && !alarmSpusteny && (elapsedMs / 1000) >= alarmSekundy) {
        alarmSpusteny = true;
        if (konzola) {
            const char* am = "!! ALARM: dosiahnuty nastaveny cas !!\r\n";
            konzola_safe_write(konzola, am, strlen(am));
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
            poslednyVypisMs = now;
        } else {
            startMs = Kernel::Clock::now();
            poslednyVypisMs = startMs;
        }
        if (konzola) {
            char buf[128];
            int n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Casovac: %s                      |\r\n", (pauza ? "STOPNUTY" : "POKRAČUJE")); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
        }
    } else if (c == 'r') {
        startMs = Kernel::Clock::now();
        zakladMs = 0;
        pauza = false;
        alarmSpusteny = false;
        if (konzola) {
            char buf[128];
            int n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  RESET: Casovac vynulovany             |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
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
                char out[256];
                int n = snprintf(out, sizeof(out), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, out, (size_t)n);
                n = snprintf(out, sizeof(out), "|  ALARM NASTAVENY: %4lu s              |\r\n", (unsigned long)v); konzola_safe_write(konzola, out, (size_t)n);
                n = snprintf(out, sizeof(out), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, out, (size_t)n);
            }
            ni = 0;
            memset(numbuf, 0, sizeof(numbuf));
        }
    }
}

void RezimCasovacTerminal::stlacenieKratke() { spracujUART('p'); }
void RezimCasovacTerminal::stlacenieDlhe() { spracujUART('r'); }
