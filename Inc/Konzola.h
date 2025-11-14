#ifndef KONZOLA_H
#define KONZOLA_H

#include "mbed.h"

void konzola_safe_write(UnbufferedSerial* konzola, const char* buf, size_t len);

inline void konzola_safe_write(UnbufferedSerial& konzola, const char* buf, size_t len) {
    konzola_safe_write(&konzola, buf, len);
}

#endif // KONZOLA_H
