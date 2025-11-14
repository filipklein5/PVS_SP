#ifndef REZIMOVLADANIELED_H
#define REZIMOVLADANIELED_H

#include "mbed.h"
#include "Rezim.h"

class RezimOvladanieLED : public Rezim {
private:
    PinName pins[3];
    PwmOut* led[3];
    DigitalOut* rgbLed[3];
    PinName rgbPins[3];
    UnbufferedSerial* konzola;

    uint8_t jas[3];
    int vybranaLED;
    uint8_t krok;
    std::chrono::milliseconds poslednyVypisMs;
    uint32_t vypisIntervalMs;
    int last_print_led;
    uint8_t last_print_jas;
    void printStatusIfChanged();
    bool blinkEnabled;
    uint32_t blinkIntervalMs;
    std::chrono::milliseconds lastBlinkMs;
    bool blinkState;
    int last_print_mode;
    uint32_t last_print_interval;
    uint8_t pwmResolution;
    uint32_t pwmPeriodMs;
    uint32_t pwmPeriodUs;
    uint32_t pwmStepMs;
    uint32_t pwmCounter;
    std::chrono::milliseconds lastPwmMs;
    Ticker pwmTicker;
    void pwmTick();
    enum class StavLED { VYP, STALA, PULZ };
    StavLED stav[3];
    float frekvencia[3];
    std::chrono::milliseconds startMs;
    std::chrono::time_point<Kernel::Clock> timerEnd[3];
    bool timerActive[3];
    bool interactiveMode;
    bool timerSettingMode;
    char timerBuf[12];
    uint8_t timerBufIdx;
    int last_print_stav;
    float last_print_freq;
    bool needStatusPrint;

public:
    RezimOvladanieLED(PinName piny[3], UnbufferedSerial* konzola_uart);
    ~RezimOvladanieLED() override;

    void inicializuj() override;
    void aktualizuj() override;
    void spracujUART(char c) override;
    void deinit() override;

    void stlacenieKratke() override;
    void stlacenieDlhe() override;
    void nastavJas(uint8_t idx, uint8_t hod);
    int getType() const override {return 1;}
};

#endif // REZIMOVLADANIELED_H
