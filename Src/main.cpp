#include "mbed.h"
#include "Piny.h"
#include "PinNames.h"
#include "SpravcaRezimov.h"
#include "RezimBlikanieLED.h"
#include "RezimOvladanieLED.h"
#include "RezimUARTSynchronizacia.h"
#include "RezimCasovacTerminal.h"
#include <chrono>
#include <cstring>

using namespace mbed;
using namespace std::chrono;

DigitalIn tlacidlo(USER_BUTTON, PullUp);
constexpr uint32_t DLHE_MS = 1200;
UnbufferedSerial konzola(USBTX, USBRX, 9600);

SpravcaRezimov spravca(&konzola);

EventQueue queue;
Thread evtThread;

void process_char(char c) {
	spravca.spracujUART(c);
}

void rx_callback() {
	char c;
	while (konzola.readable()) {
		ssize_t n = konzola.read(&c, 1);
		if (n <= 0) break;
		if (c != '\r' && c != '\n') {
			queue.call(process_char, c);
		}
	}
}

int main() {
	evtThread.start(callback(&queue, &EventQueue::dispatch_forever));

	konzola.attach(callback(rx_callback), SerialBase::RxIrq);

	char rx;
	while (konzola.readable() && konzola.read(&rx, 1) > 0) {
		if (rx != '\r' && rx != '\n') {
			queue.call(process_char, rx);
		}
	}

	const char* modeNames[] = {
		"0 - Blikanie LED",
		"1 - Ovladanie LED (PWM)",
		"2 - UART synchronizacia",
		"3 - Casovac (terminal)"
	};

	auto printMenu = [&](int active)->void {
		const char* clear = "\x1b[2J\x1b[H";
		konzola.write(clear, strlen(clear));

		char buf[256];
		int n = 0;
		n = snprintf(buf, sizeof(buf), "+------------------------------------------------------------+\r\n"); konzola.write(buf, n);
		n = snprintf(buf, sizeof(buf), "|  MULTIMODOVE ZARIADENIE (MBED)                            |\r\n"); konzola.write(buf, n);
		n = snprintf(buf, sizeof(buf), "+------------------------------------------------------------+\r\n"); konzola.write(buf, n);
		konzola.write("| Dostupne rezimy:                                           |\r\n", 60);
		for (int i=0;i<4;i++){
			if (i == active) n = snprintf(buf, sizeof(buf), "|  -> %s", modeNames[i]);
			else n = snprintf(buf, sizeof(buf), "|     %s", modeNames[i]);
			int len = (int)strlen(buf);
			if (len < 60) {
				memset(buf + len, ' ', 60 - len);
				buf[60] = '\0';
				strcat(buf, "\r\n");
				n = (int)strlen(buf);
			}
			konzola.write(buf, n);
		}
		n = snprintf(buf, sizeof(buf), "+------------------------------------------------------------+\r\n"); konzola.write(buf, n);
		konzola.write("| Ovládanie: r/g/b - vybrať farbu | p - pulz | +/- - jas/frekvencia |\r\n", 75);
		konzola.write("| t - test LED, d - digital test | Kr. tlac: dalsi rezim | Dl. tlac: reset |\r\n", 86);
		n = snprintf(buf, sizeof(buf), "+------------------------------------------------------------+\r\n\r\n"); konzola.write(buf, n);
		n = snprintf(buf, sizeof(buf), "Aktualny rezim: %d. Napiste prikaz alebo pouzite tlacidlo...\r\n", active);
		konzola.write(buf, n);
	};

	printMenu(0);

	spravca.pridajRezim(new RezimBlikanieLED(const_cast<PinName*>(LEDKY), &konzola, 200));
	spravca.pridajRezim(new RezimOvladanieLED(const_cast<PinName*>(LEDKY_PWM), &konzola));
	spravca.pridajRezim(new RezimUARTSynchronizacia(const_cast<PinName*>(LEDKY), nullptr, &konzola, 200));
	spravca.pridajRezim(new RezimCasovacTerminal(&konzola, 1000));

	bool boloStlacene = false;
	auto zaciatokStl = Kernel::Clock::now();

	while (true) {
		spravca.aktualizuj();

		bool pressed = (tlacidlo.read() == 0);
		auto teraz = Kernel::Clock::now();

		if (pressed) {
			if (!boloStlacene) {
				ThisThread::sleep_for(20ms);
				if (tlacidlo.read() == 0) {
					boloStlacene = true;
					zaciatokStl = teraz;
				}
			}
		} else {
			if (boloStlacene) {
				auto dlzka = chrono::duration_cast<chrono::milliseconds>(teraz - zaciatokStl).count();
				boloStlacene = false;

				if (dlzka >= DLHE_MS) {
					const char* msg = "DLHE STLACENIE: resetujem system pomocou NVIC_SystemReset()\r\n";
					konzola.write(msg, strlen(msg));
					ThisThread::sleep_for(50ms);
					NVIC_SystemReset();
				} else {
					spravca.dalsiRezim();
					Rezim* r = spravca.getAktualnyRezim();
					int active = 0;
					if (r) active = r->getType() >= 0 ? r->getType() : 0;
					printMenu(active);
					if (r && r->getType() == 1) r->stlacenieKratke();
				}
			}
		}

		ThisThread::sleep_for(10ms);
	}

	return 0;
}