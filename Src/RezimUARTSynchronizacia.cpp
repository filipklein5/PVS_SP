#include "RezimUARTSynchronizacia.h"
#include <cstring>
#include "Konzola.h"

using namespace mbed;

RezimUARTSynchronizacia::RezimUARTSynchronizacia(PinName piny[8], UnbufferedSerial* peer_serial, UnbufferedSerial* konzola_uart, uint32_t txInterval)
    : peerSerial(peer_serial), konzola(konzola_uart), stavLED(0), txIntervalMs(txInterval),
      poslednyTx(std::chrono::milliseconds(0))
{
    for(int i=0;i<8;i++){
        pins[i] = piny[i];
        led[i] = nullptr;
    }
}

RezimUARTSynchronizacia::~RezimUARTSynchronizacia() {
    for(int i=0;i<8;i++) if(led[i]) delete led[i];
}

void RezimUARTSynchronizacia::inicializuj() {
    for(int i=0;i<8;i++){
        if(led[i]) { delete led[i]; led[i]=nullptr; }
        led[i] = new DigitalOut(pins[i],0);
    }
    poslednyTx = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    if(konzola){
        const char* msg="Rezim: UART synchronizacia - spustene\r\n";
        konzola_safe_write(konzola, msg, strlen(msg));
    }
}

void RezimUARTSynchronizacia::aktualizuj() {
    for(int i=0;i<8;i++) if(led[i]) led[i]->write((stavLED & (1<<i))?1:0);

    auto teraz = std::chrono::duration_cast<std::chrono::milliseconds>(Kernel::Clock::now().time_since_epoch());
    if(peerSerial && (teraz - poslednyTx).count()>=txIntervalMs){
        poslednyTx = teraz;
        uint8_t b=stavLED;
        peerSerial->write(reinterpret_cast<const char*>(&b),1);
    }
}

void RezimUARTSynchronizacia::spracujUART(char c){
    stavLED = static_cast<uint8_t>(static_cast<unsigned char>(c));
    if(konzola){
        char buf[32];
        int n=snprintf(buf,sizeof(buf),"Prijaty stav: 0x%02X\r\n",(unsigned)stavLED);
        konzola_safe_write(konzola, buf, (size_t)n);
    }
}
