#pragma once
#include "mbed.h"

#define LED_GREEN   PC_7
#define LED_BLUE    PB_7
#define LED_RED     PG_2
#define USER_BUTTON PC_13

extern PinName LEDKY[8];
extern PinName LEDKY_RGB[3];
extern PinName UART_SYNC_TX;
extern PinName UART_SYNC_RX;
