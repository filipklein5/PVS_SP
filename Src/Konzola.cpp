#include "Konzola.h"

// Keep a single mutex for all console writes in the app
static rtos::Mutex konzola_mutex;

void konzola_safe_write(UnbufferedSerial* konzola, const char* buf, size_t len) {
    if (!konzola || !buf || len == 0) return;
    // Lock for thread-safety. This MUST NOT be called from an ISR.
    konzola_mutex.lock();
    konzola->write(buf, len);
    konzola_mutex.unlock();
}
