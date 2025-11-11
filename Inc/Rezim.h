#ifndef REZIM_H
#define REZIM_H

#include "mbed.h"

class Rezim {
public:
    virtual ~Rezim() = default;
    virtual void inicializuj() = 0;
    virtual void aktualizuj() = 0;
    virtual void spracujUART(char c) {}
    virtual void stlacenieKratke() {}
    virtual void stlacenieDlhe() {}
};

#endif