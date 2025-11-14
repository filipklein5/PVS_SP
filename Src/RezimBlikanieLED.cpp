#include "RezimBlikanieLED.h"
#include <cstdlib>
#include "Konzola.h"
#include <cctype>

using namespace mbed;

RezimBlikanieLED::RezimBlikanieLED(PinName* piny, UnbufferedSerial* konzola_uart, uint32_t interval) : konzola(konzola_uart), intervalMs(interval), pattern(0x01), rezimPatternu(0), poslednyMs(std::chrono::milliseconds(0)) {
    for (int i=0;i<8;i++){
        pins[i] = piny[i];
        led[i] = nullptr;
    }
    rgbPins[0] = LEDKY_RGB[0];
    rgbPins[1] = LEDKY_RGB[1];
    rgbPins[2] = LEDKY_RGB[2];
    for (int i=0;i<3;i++) rgbLed[i] = nullptr;
    selectedRGB = -1;
    rgbState = false;
    last_print_color = -2;
    last_print_interval = intervalMs;
}

RezimBlikanieLED::~RezimBlikanieLED() {
    for (int i=0;i<8;i++) if(led[i]) delete led[i];
    for (int i=0;i<3;i++) if(rgbLed[i]) delete rgbLed[i];
}

void RezimBlikanieLED::inicializuj() {
    for (int i=0;i<8;i++){
        if (led[i]) { delete led[i]; led[i] = nullptr; }
        bool reused = false;
        for (int r=0;r<3;r++){
            if (pins[i] == rgbPins[r] && rgbPins[r] != NC) {
                // create rgbLed[r] if not yet
                if (!rgbLed[r]) rgbLed[r] = new DigitalOut(rgbPins[r], 0);
                led[i] = rgbLed[r];
                reused = true;
                break;
            }
        }
        if (!reused) led[i] = new DigitalOut(pins[i], 0);
    }
    for (int i=0;i<3;i++){
        if (!rgbLed[i] && rgbPins[i] != NC) rgbLed[i] = new DigitalOut(rgbPins[i], 0);
    }
    selectedRGB = -1;
    rezimPatternu = 2;
    poslednyMs = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    pattern = 0x01;
    if (konzola) {
        const char* msg = "Blikanie LED - menu:\r\n  r/g/b - vyber farby, 0/1/2 - pattern, +/- - uprav interval, ESC - ukonci menu\r\n";
        konzola_safe_write(konzola, msg, strlen(msg));
    }
    interactiveMode = true;
    printStatusIfChanged();
}

void RezimBlikanieLED::deinit() {
    selectedRGB = -1;
    for (int i=0;i<8;i++){
        if (led[i]) led[i]->write((pattern & (1<<i)) ? 1 : 0);
    }
    for (int i=0;i<3;i++) if (rgbLed[i]) rgbLed[i]->write(0);
}
void RezimBlikanieLED::aktualizuj() {
    auto teraz = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    if ((teraz - poslednyMs).count() < intervalMs) return;
    poslednyMs = teraz;

    if (selectedRGB >= 0 && selectedRGB < 3) {
        rgbState = !rgbState;
        if (rgbLed[selectedRGB]) rgbLed[selectedRGB]->write(rgbState ? 1 : 0);
    } else {
        if (rezimPatternu==0){
            bool msb = (pattern & 0x80);
            pattern = (uint8_t)((pattern<<1) | (msb?1:0));
        } else if (rezimPatternu==1){
            static int dir=1, pos=0;
            pattern = (uint8_t)(1<<pos);
            pos += dir;
            if (pos==0 || pos==7) dir=-dir;
        } else if (rezimPatternu==2) {
            int avail[3]; int cnt=0;
            for (int r=0;r<3;r++) if (rgbLed[r]) avail[cnt++] = r;
            if (cnt == 0) return;
            static int idx = -1;
            idx = (idx + 1) % cnt;
            int active = avail[idx];
            for (int r=0;r<3;r++){
                if (rgbLed[r]) rgbLed[r]->write(r == active ? 1 : 0);
            }
            for (int i=0; i<8;i++){
                if (!led[i]) continue;
                bool isRgb = false;
                for (int r=0;r<3;r++) if (rgbLed[r] && led[i] == rgbLed[r]) { isRgb = true; break; }
                if (!isRgb) led[i]->write(0);
            }
            printStatusIfChanged();
            return;
        } else {
            pattern = (uint8_t)(rand() & 0xFF);
            if (pattern==0) pattern=0xA5;
        }

        for (int i=0;i<8;i++){
            if (led[i]) led[i]->write((pattern & (1<<i)) ? 1 : 0);
        }
    }
    printStatusIfChanged();
}

void RezimBlikanieLED::spracujUART(char c) {
    if (!interactiveMode) return;
    if ((unsigned char)c == 27) {
        interactiveMode = false;
        if (konzola) konzola_safe_write(konzola, "Vystup z menu\r\n", 14);
        printStatusIfChanged();
        return;
    }
    char lc = (char)tolower((unsigned char)c);
    if (lc == '+' ) {
        if (intervalMs > 50) intervalMs = intervalMs - 50;
        printStatusIfChanged();
        return;
    }
    if (lc == '-') {
        intervalMs = intervalMs + 50;
        printStatusIfChanged();
        return;
    }

    if (lc == 'r') {
        selectedRGB = 2;
        for (int k=0;k<3;k++) if (rgbLed[k]) rgbLed[k]->write(k == selectedRGB ? 1 : 0);
        rgbState = true;
        printStatusIfChanged();
        return;
    }
    if (lc == 'g') {
        selectedRGB = 0;
        for (int k=0;k<3;k++) if (rgbLed[k]) rgbLed[k]->write(k == selectedRGB ? 1 : 0);
        rgbState = true;
        printStatusIfChanged();
        return;
    }
    if (lc == 'b') {
        selectedRGB = 1;
        for (int k=0;k<3;k++) if (rgbLed[k]) rgbLed[k]->write(k == selectedRGB ? 1 : 0);
        rgbState = true;
        printStatusIfChanged();
        return;
    }
}

void RezimBlikanieLED::printStatusIfChanged() {
    int color = (selectedRGB >=0 ? selectedRGB : -1);
    if (last_print_color != color || last_print_interval != intervalMs || last_print_pattern != pattern) {
        last_print_color = color;
        last_print_interval = intervalMs;
        last_print_pattern = pattern;
        if (konzola) {
            const char* names[] = {"ZELENA","MODRA","CERVENA"};
            char buf[256];
            int n = 0;
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Rezim: Blikanie LED                  |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            if (color == -1) {
                n = snprintf(buf, sizeof(buf), "|  Mode      : VZOR (8-LED pattern)      |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
            } else {
                n = snprintf(buf, sizeof(buf), "|  Mode      : RGB selected              |\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
                n = snprintf(buf, sizeof(buf), "|  Selected  : %-20s |\r\n", names[color]); konzola_safe_write(konzola, buf, (size_t)n);
            }
            n = snprintf(buf, sizeof(buf), "|  Interval  : %4lums                     |\r\n", (unsigned long)intervalMs); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "|  Pattern   : 0x%02X                       |\r\n", pattern); konzola_safe_write(konzola, buf, (size_t)n);
            n = snprintf(buf, sizeof(buf), "+----------------------------------------+\r\n\r\n"); konzola_safe_write(konzola, buf, (size_t)n);
        }
    }
}