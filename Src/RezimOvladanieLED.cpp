#include "RezimOvladanieLED.h"
#include <cstdio>

using namespace mbed;

RezimOvladanieLED::RezimOvladanieLED(PinName piny[3], BufferedSerial* konzola_uart) : konzola(konzola_uart), vybranaLED(0), krok(25) {
    for (int i=0;i<3;i++){
        pins[i] = piny[i];
        led[i] = nullptr;
        jas[i] = 0;
    }
}

RezimOvladanieLED::~RezimOvladanieLED() {
    for (int i=0;i<3;i++) if (led[i]) delete led[i];
}

void RezimOvladanieLED::inicializuj() {
    for (int i=0;i<3;i++){
        if (led[i]) { delete led[i]; led[i] = nullptr; }
        led[i] = new PwmOut(pins[i]);
        led[i]->write(0.0f);
        jas[i] = 0;
    }
    vybranaLED = 0;

    if (konzola){
        const char* msg = "Rezim: Ovladanie LED (PWM) - spustene\r\n";
        konzola->write(msg, strlen(msg));
    }
}

void RezimOvladanieLED::aktualizuj() {
    for (int i=0;i<3;i++){
        if (led[i]){
            float duty = jas[i]/255.0f;
            led[i]->write(duty);
        }
    }

    if (konzola){
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "Vybrana LED %d, jas: %d\r\n", vybranaLED, jas[vybranaLED]);
        konzola->write(buf, n);
    }
}

void RezimOvladanieLED::stlacenieKratke() {
    vybranaLED = (vybranaLED + 1) % 3;
}

void RezimOvladanieLED::stlacenieDlhe() {
    jas[vybranaLED] += krok;
    if (jas[vybranaLED] > 255) jas[vybranaLED] = 255;
}

void RezimOvladanieLED::nastavJas(uint8_t idx, uint8_t hod){
    if (idx < 3) jas[idx] = hod;
}

void RezimOvladanieLED::spracujUART(char c) {
    if (c == '+'){
        jas[vybranaLED] += krok;
        if (jas[vybranaLED] > 255) jas[vybranaLED] = 255;
    } else if (c == '-'){
        if (jas[vybranaLED] < krok) jas[vybranaLED] = 0;
        else jas[vybranaLED] -= krok;
    }
}