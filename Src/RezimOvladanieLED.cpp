#include "RezimOvladanieLED.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Konzola.h"
#include "Piny.h"
#include <cctype>

using namespace mbed;

RezimOvladanieLED::RezimOvladanieLED(PinName piny[3], UnbufferedSerial* konzola_uart)
    : konzola(konzola_uart), vybranaLED(0), krok(25), poslednyVypisMs(std::chrono::milliseconds(0)), vypisIntervalMs(500), last_print_led(-1), last_print_jas(0)
{
    for (int i=0;i<3;i++){
        pins[i] = piny[i];
        led[i] = nullptr;
        jas[i] = 0;
    }
    rgbPins[0] = LEDKY_RGB[0];
    rgbPins[1] = LEDKY_RGB[1];
    rgbPins[2] = LEDKY_RGB[2];
    for (int i=0;i<3;i++) rgbLed[i] = nullptr;
}

RezimOvladanieLED::~RezimOvladanieLED() {
    for (int i=0;i<3;i++) if (led[i]) delete led[i];
    for (int i=0;i<3;i++) if (rgbLed[i]) delete rgbLed[i];
    pwmTicker.detach();
}

void RezimOvladanieLED::inicializuj() {
    for (int i=0;i<3;i++){
        jas[i] = 0;
    }
    vybranaLED = 0;

    const uint8_t default_brightness = 128;
    jas[vybranaLED] = default_brightness;
    for (int i=0;i<3;i++){
        if (rgbLed[i]) { delete rgbLed[i]; rgbLed[i] = nullptr; }
        if (rgbPins[i] != NC) rgbLed[i] = new DigitalOut(rgbPins[i], 0);
    }
    pwmResolution = 100;
    pwmPeriodMs = 10;
    pwmPeriodUs = pwmPeriodMs * 1000;
    pwmCounter = 0;
    lastPwmMs = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    pwmTicker.detach();
    pwmTicker.attach(callback(this, &RezimOvladanieLED::pwmTick), 200us);

    if (konzola) {
        char buf[128];
        int n = snprintf(buf, sizeof(buf), "LED pins (RGB indices 0..2): %d, %d, %d\r\n", (int)pins[0], (int)pins[1], (int)pins[2]);
        konzola_safe_write(konzola, buf, (size_t)n);
    }

    if (konzola){
        const char* msg = "Rezim: Ovladanie LED (PWM) - spustene\r\n";
        konzola_safe_write(konzola, msg, strlen(msg));
    }
    blinkEnabled = false;
    blinkIntervalMs = 500;
    lastBlinkMs = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    blinkState = false;
    last_print_mode = 0;
    startMs = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    for (int i=0;i<3;i++){
        stav[i] = StavLED::VYP;
        frekvencia[i] = 1.0f;
    }
    last_print_stav = -99;
    last_print_freq = 0.0f;
    for (int i=0;i<3;i++) {
        timerActive[i] = false;
        timerEnd[i] = Kernel::Clock::now();
    }
    interactiveMode = true;
    timerSettingMode = false;
    timerBufIdx = 0;
    needStatusPrint = false;
}

void RezimOvladanieLED::aktualizuj() {
    auto teraz = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    if (needStatusPrint) {
        needStatusPrint = false;
        printStatusIfChanged();
    }
    if (!blinkEnabled) {
        return;
    } else {
        if ((teraz - lastBlinkMs).count() >= (int)blinkIntervalMs) {
            lastBlinkMs = teraz;
            blinkState = !blinkState;
            for (int i=0;i<3;i++){
                if (rgbLed[i]) {
                    if (i == vybranaLED) rgbLed[i]->write(blinkState ? 1 : 0);
                    else rgbLed[i]->write(0);
                }
            }
        }
    }
}

void RezimOvladanieLED::stlacenieKratke() {
    vybranaLED = (vybranaLED + 1) % 3;
    const uint8_t default_brightness = 64;
    if (jas[vybranaLED] == 0) {
        jas[vybranaLED] = default_brightness;
        pwmTick();
    }
    printStatusIfChanged();
}

void RezimOvladanieLED::stlacenieDlhe() {
    {
        int tmp = (int)jas[vybranaLED] + (int)krok;
        if (tmp > 255) tmp = 255;
        jas[vybranaLED] = (uint8_t)tmp;
    }
    pwmTick();
    printStatusIfChanged();
}

void RezimOvladanieLED::nastavJas(uint8_t idx, uint8_t hod){
    if (idx < 3) jas[idx] = hod;
    printStatusIfChanged();
}

void RezimOvladanieLED::spracujUART(char c) {
    if ((unsigned char)c == 27) {
        interactiveMode = false;
        if (konzola) konzola_safe_write(konzola, "EXIT MENU\r\n", 11);
        return;
    }
    if (timerSettingMode) {
        if (c >= '0' && c <= '9') {
            if (timerBufIdx < (sizeof(timerBuf)-1)) timerBuf[timerBufIdx++] = c;
            return;
        }
        if (c == '\r' || c == '\n') {
            timerBuf[timerBufIdx] = '\0';
            if (timerBufIdx > 0) {
                uint32_t secs = (uint32_t)atoi(timerBuf);
                auto now = Kernel::Clock::now();
                timerEnd[vybranaLED] = now + std::chrono::milliseconds((long long)secs * 1000LL);
                timerActive[vybranaLED] = true;
                stav[vybranaLED] = StavLED::STALA;
                pwmTick();
                if (konzola) {
                    char out[128];
                    int n = snprintf(out, sizeof(out), "+----------------------------------------+\r\n"); konzola_safe_write(konzola,out,(size_t)n);
                    n = snprintf(out, sizeof(out), "|  TIMER: LED %d ON for %lu s           |\r\n", vybranaLED, (unsigned long)secs); konzola_safe_write(konzola,out,(size_t)n);
                    n = snprintf(out, sizeof(out), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola,out,(size_t)n);
                }
            }
            timerSettingMode = false;
            timerBufIdx = 0;
            return;
        }
        return;
    }
    char lc = (char)tolower((unsigned char)c);
    if (lc == 'r' || lc == 'g' || lc == 'b') {
        int found = -1;
        if (lc == 'g') found = 0;
        else if (lc == 'b') found = 1;
        else if (lc == 'r') found = 2;
        if (found >= 0) {
            vybranaLED = found;
            if (blinkEnabled) {
                blinkState = false;
                for (int i=0;i<3;i++) if (rgbLed[i]) rgbLed[i]->write(i==vybranaLED ? 1 : 0);
            } else {
                if (stav[vybranaLED] == StavLED::VYP) stav[vybranaLED] = StavLED::STALA;
                else if (stav[vybranaLED] == StavLED::STALA) stav[vybranaLED] = StavLED::PULZ;
                else stav[vybranaLED] = StavLED::VYP;
                if (stav[vybranaLED] != StavLED::VYP && jas[vybranaLED] == 0) jas[vybranaLED] = 200;
                    pwmTick();
            }
            printStatusIfChanged();
        }
        return;
    }
    if (lc == 'p') {
        stav[vybranaLED] = StavLED::PULZ;
        printStatusIfChanged();
        return;
    }
    if (lc == 't') {
        for (int i=0;i<3;i++){
            if (konzola) {
                char buf[64];
                int n = snprintf(buf, sizeof(buf), "Testing LED index %d (pin %d)\r\n", i, (int)pins[i]);
                konzola_safe_write(konzola, buf, (size_t)n);
            }
            float old = 0.0f;
            if (rgbLed[i]) {
                rgbLed[i]->write(1);
                ThisThread::sleep_for(150ms);
                rgbLed[i]->write(0);
                ThisThread::sleep_for(150ms);
            }
        }
        printStatusIfChanged();
        return;
    }
    if (lc == 'z') {
        timerSettingMode = true;
        timerBufIdx = 0;
        memset(timerBuf,0,sizeof(timerBuf));
        if (konzola) konzola_safe_write(konzola, "Zadaj cas v sekundach a stlac ENTER: ", 34);
        return;
    }
    if (lc == 'd') {
        for (int i=0;i<3;i++){
            if (konzola) {
                char buf[64];
                int n = snprintf(buf, sizeof(buf), "Digital test LED index %d (pin %d)\r\n", i, (int)pins[i]);
                konzola_safe_write(konzola, buf, (size_t)n);
            }
            DigitalOut dout(rgbPins[i]);
            dout = 1;
            ThisThread::sleep_for(200ms);
            dout = 0;
            ThisThread::sleep_for(200ms);
        }
        printStatusIfChanged();
        return;
    }
    if (c == '+'){
        if (blinkEnabled) {
            if (blinkIntervalMs > 50) blinkIntervalMs -= 50;
        } else {
            if (stav[vybranaLED] == StavLED::PULZ) {
                frekvencia[vybranaLED] += 0.1f;
                if (frekvencia[vybranaLED] > 5.0f) frekvencia[vybranaLED] = 5.0f;
            } else {
                int step = 16;
                int tmp = (int)jas[vybranaLED] + step;
                if (tmp > 255) tmp = 255;
                jas[vybranaLED] = (uint8_t)tmp;
            }
        }
        printStatusIfChanged();
    } else if (c == '-'){
        if (blinkEnabled) {
            blinkIntervalMs += 50;
        } else {
            if (stav[vybranaLED] == StavLED::PULZ) {
                frekvencia[vybranaLED] -= 0.1f;
                if (frekvencia[vybranaLED] < 0.1f) frekvencia[vybranaLED] = 0.1f;
            } else {
                int step = 16;
                int tmp = (int)jas[vybranaLED] - step;
                if (tmp < 0) tmp = 0;
                jas[vybranaLED] = (uint8_t)tmp;
            }
        }
        printStatusIfChanged();
    }
}

void RezimOvladanieLED::printStatusIfChanged() {
    int mode = blinkEnabled ? 1 : 0;
    int stavInt = (int)stav[vybranaLED];
    if (last_print_led != vybranaLED || last_print_stav != stavInt || last_print_freq != frekvencia[vybranaLED] || last_print_mode != mode || last_print_jas != jas[vybranaLED]) {
        last_print_led = vybranaLED;
        last_print_stav = stavInt;
        last_print_mode = mode;
        last_print_freq = frekvencia[vybranaLED];
        last_print_jas = jas[vybranaLED];
        if (konzola) {
            const char* names[] = {"ZELENA","MODRA","CERVENA"};
            const char* stavStr = (stav[vybranaLED] == StavLED::STALA) ? "STALA" : (stav[vybranaLED] == StavLED::VYP ? "VYP" : "PULZ");
            char buf[256];
            int n = 0;
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Rezim: Ovladanie LED (PWM)            |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Farba     : %-20s |\r\n", names[vybranaLED]); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Stav      : %-20s |\r\n", stavStr); konzola_safe_write(konzola, buf, (size_t)n);
            if (stav[vybranaLED] == StavLED::STALA) {
                n = snprintf(buf, sizeof(buf), "|  Jas       : %3u / 255             |\r\n", jas[vybranaLED]); konzola_safe_write(konzola, buf, (size_t)n);
            } else if (stav[vybranaLED] == StavLED::PULZ) {
                n = snprintf(buf, sizeof(buf), "|  Frekvencia: %7.2f Hz              |\r\n", frekvencia[vybranaLED]); konzola_safe_write(konzola, buf, (size_t)n);
            } else {
                n = snprintf(buf, sizeof(buf), "|  Jas       : %3u / 255             |\r\n", jas[vybranaLED]); konzola_safe_write(konzola, buf, (size_t)n);
            }
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
        }
    }
}

void RezimOvladanieLED::pwmTick() {
    auto now_tp = Kernel::Clock::now();
    for (int ti = 0; ti < 3; ++ti) {
        if (timerActive[ti] && now_tp >= timerEnd[ti]) {
            timerActive[ti] = false;
            stav[ti] = StavLED::VYP;
            needStatusPrint = true;
        }
    }
    auto teraz_us = std::chrono::duration_cast<std::chrono::microseconds>(Kernel::Clock::now().time_since_epoch());
    long now_us = (long)teraz_us.count();
    for (int i=0;i<3;i++){
        if (!rgbLed[i]) continue;
        float duty = 0.0f;
        if (stav[i] == StavLED::VYP) duty = 0.0f;
        else if (stav[i] == StavLED::STALA) {
            const float gamma = 2.4f;
            float normalized = (float)jas[i] / 255.0f;
            duty = powf(normalized, gamma);
            if (jas[i] >= 240) duty = 1.0f;
        }
        else if (stav[i] == StavLED::PULZ) {
            float t = (now_us - startMs.count()*1000) / 1000000.0f;
            duty = 0.5f * (1.0f + sinf(2.0f * 3.14159265f * frekvencia[i] * t));
        }
        if (duty < 0.0f) duty = 0.0f;
        if (duty > 1.0f) duty = 1.0f;
        uint32_t phase = (uint32_t)(now_us % pwmPeriodUs);
        uint32_t on_us = (uint32_t)(duty * pwmPeriodUs);
        rgbLed[i]->write(phase < on_us ? 1 : 0);
    }
}

void RezimOvladanieLED::deinit() {
    pwmTicker.detach();
    for (int i=0;i<3;i++) if (rgbLed[i]) rgbLed[i]->write(0);
}