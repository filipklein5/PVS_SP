#include "RezimBlikanieLED.h"
#include <cstdlib>

using namespace mbed;

RezimBlikanieLED::RezimBlikanieLED(PinName* piny, BufferedSerial* konzola_uart, uint32_t interval)
    : konzola(konzola_uart), intervalMs(interval), pattern(0x01), rezimPatternu(0),
      poslednyMs(std::chrono::milliseconds(0))
{
    for(int i=0;i<8;i++){
        pins[i] = piny[i];
        led[i] = nullptr;
    }
}

RezimBlikanieLED::~RezimBlikanieLED() {
    for(int i=0;i<8;i++) if(led[i]) delete led[i];
}

void RezimBlikanieLED::inicializuj() {
    for(int i=0;i<8;i++){
        if(led[i]) { delete led[i]; led[i] = nullptr; }
        led[i] = new DigitalOut(pins[i],0);
    }
    poslednyMs = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    pattern = 0x01;
    if(konzola){
        const char* msg = "Rezim: Blikanie LED - spustene\r\n";
        konzola->write(msg, strlen(msg));
    }
}

void RezimBlikanieLED::aktualizuj() {
    auto teraz = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    if((teraz - poslednyMs).count() < intervalMs) return;
    poslednyMs = teraz;

    if(rezimPatternu==0){
        bool msb = (pattern & 0x80);
        pattern = (uint8_t)((pattern<<1) | (msb?1:0));
    } else if(rezimPatternu==1){
        static int dir=1, pos=0;
        pattern = (uint8_t)(1<<pos);
        pos += dir;
        if(pos==0 || pos==7) dir=-dir;
    } else {
        pattern = (uint8_t)(rand() & 0xFF);
        if(pattern==0) pattern=0xA5;
    }

    for(int i=0;i<8;i++){
        if(led[i]) led[i]->write((pattern & (1<<i)) ? 1 : 0);
    }
}