#ifndef PINY_H
#define PINY_H

#include "mbed.h"

#define LED1     PC_7
#define LED2     PB_7
#define LED3     PG_2
#define USER_BUTTON  PC_13

constexpr PinName LEDKY[8] = {PA_5, PA_6, PA_7, PB_0, PB_1, PB_5, PB_6, PB_7};
constexpr PinName LEDKY_RGB[3] = {LED1, LED2, LED3};
constexpr PinName LEDKY_PWM[3] = {PA_0, PA_1, PA_2};

constexpr PinName UART_SYNC_TX = NC;
constexpr PinName UART_SYNC_RX = NC;

#endif
