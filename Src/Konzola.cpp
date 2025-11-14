#include "Konzola.h"

static rtos::Mutex konzola_mutex;

void konzola_safe_write(UnbufferedSerial* konzola, const char* buf, size_t len) {
    if (!konzola || !buf || len == 0) return;
    konzola_mutex.lock();
    konzola->write(buf, len);
    konzola_mutex.unlock();
}
