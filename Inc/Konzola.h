#ifndef KONZOLA_H
#define KONZOLA_H

#include "mbed.h"

// Thread-safe wrapper for writing to a UnbufferedSerial pointer/reference.
// This uses an rtos::Mutex to avoid interleaved writes from multiple threads.
void konzola_safe_write(UnbufferedSerial* konzola, const char* buf, size_t len);

inline void konzola_safe_write(UnbufferedSerial& konzola, const char* buf, size_t len) {
    konzola_safe_write(&konzola, buf, len);
}

#endif // KONZOLA_H
