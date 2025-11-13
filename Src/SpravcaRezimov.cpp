#include "SpravcaRezimov.h"
#include <cstdio>
#include <cstring>
#include "Konzola.h"

using namespace mbed;

SpravcaRezimov::SpravcaRezimov(UnbufferedSerial* konzola_uart) : aktualny(0), konzola(konzola_uart) {}

SpravcaRezimov::~SpravcaRezimov() {
    for (auto r : zoznam) delete r;
    zoznam.clear();
}

void SpravcaRezimov::pridajRezim(Rezim* r) {
    zoznam.push_back(r);
    if (zoznam.size() == 1) {
        aktualny = 0;
        zoznam[0]->inicializuj();
    }
}

void SpravcaRezimov::dalsiRezim() {
    if (zoznam.empty()) return;
    // deinit current
    if (aktualny < zoznam.size()) zoznam[aktualny]->deinit();
    aktualny = (aktualny + 1) % zoznam.size();
    zoznam[aktualny]->inicializuj();
        if (konzola) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "Prepnute na rezim %u\r\n", (unsigned)aktualny);
            konzola_safe_write(konzola, buf, (size_t)n);
        }
}

void SpravcaRezimov::vyberRezim(uint8_t idx) {
    if (idx < zoznam.size()) {
        // deinit current
        if (aktualny < zoznam.size()) zoznam[aktualny]->deinit();
        aktualny = idx;
        zoznam[aktualny]->inicializuj();
        if (konzola) {
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "Vybrany rezim %u\r\n", (unsigned)aktualny);
            konzola_safe_write(konzola, buf, (size_t)n);
        }
    } else {
        if (konzola) {
            const char* msg = "Neplatne cislo rezimu\r\n";
            konzola_safe_write(konzola, msg, strlen(msg));
        }
    }
}

void SpravcaRezimov::aktualizuj() {
    if (!zoznam.empty()) zoznam[aktualny]->aktualizuj();
}

void SpravcaRezimov::stlacenieKratke() {
    if (!zoznam.empty()) zoznam[aktualny]->stlacenieKratke();
}

void SpravcaRezimov::stlacenieDlhe() {
    if (!zoznam.empty()) zoznam[aktualny]->stlacenieDlhe();
}

void SpravcaRezimov::spracujUART(char c) {
    if (c >= '0' && c <= '9') {
        uint8_t req = c - '0';
        if (req < zoznam.size()) {
            vyberRezim(req);
            return;
        } else {
            if (konzola) {
                const char* msg = "Cislo mimo rozsahu rezimov\r\n";
                konzola_safe_write(konzola, msg, strlen(msg));
            }
            return;
        }
    }
    if (!zoznam.empty()) zoznam[aktualny]->spracujUART(c);
}