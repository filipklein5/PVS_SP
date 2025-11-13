#ifndef REZIMOVLADANIELED_H
#define REZIMOVLADANIELED_H

#include "mbed.h"
#include "Rezim.h"

class RezimOvladanieLED : public Rezim {
private:
    PinName pins[3];
    PwmOut* led[3];
    // software-driven RGB outputs (use onboard LEDKY_RGB)
    DigitalOut* rgbLed[3];
    PinName rgbPins[3];
    UnbufferedSerial* konzola;

    uint8_t jas[3];
    int vybranaLED;
    uint8_t krok;
    std::chrono::milliseconds poslednyVypisMs;
    uint32_t vypisIntervalMs;
    // Keep last printed state to avoid duplicate prints
    int last_print_led;
    uint8_t last_print_jas;
    void printStatusIfChanged();
    // Blink mode
    bool blinkEnabled;
    uint32_t blinkIntervalMs;
    std::chrono::milliseconds lastBlinkMs;
    bool blinkState;
    int last_print_mode; // 0=steady,1=blink
    uint32_t last_print_interval;
    // software PWM
    uint8_t pwmResolution;
    uint32_t pwmPeriodMs;
    uint32_t pwmPeriodUs;
    uint32_t pwmStepMs;
    uint32_t pwmCounter;
    std::chrono::milliseconds lastPwmMs;
    // high-rate ticker for smooth PWM
    Ticker pwmTicker;
    void pwmTick();
    // 3-state LED control
    enum class StavLED { VYP, STALA, PULZ };
    StavLED stav[3];
    float frekvencia[3]; // Hz
    std::chrono::milliseconds startMs;
    // per-LED timed-on support (timerEnd is timepoint when LED should turn off)
    std::chrono::time_point<Kernel::Clock> timerEnd[3];
    bool timerActive[3];
    // interactive timer-setting state
    bool interactiveMode;
    bool timerSettingMode;
    char timerBuf[12];
    uint8_t timerBufIdx;
    // last printed status (to avoid spamming)
    int last_print_stav; // -99 uninit, else 0..2
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
