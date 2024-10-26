

#include <esp_task_wdt.h>

#include "common.h"
#include "webhandling.h"
#include "version.h"
#include "NMEAHandling.h"

bool SaveConfiguration = false;
bool ChangedConfiguration = false;

#define WDT_TIMEOUT 5

char Version[] = VERSION_STR; // Manufacturer's Software version code

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    Serial.printf("Firmware version:%s\n", Version);

    webinit();

	N2kBegin();

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
    Serial.println("device initilized");
}

void loop() {
    webLoop();

	N2kLoop();

    ChangedConfiguration = false;
	uint8_t output_ = 1;
    Relay* relay_ = &Relay1;
    while (relay_ != nullptr) {
		if (relay_->isActive()) {
			relay_->process();
            if ((relay_->offTime() > 0) && (relay_->isTimerDone())) {
				SendSwitchStatus(output_, false);
			}
		}
		relay_ = (Relay*)relay_->getNext();
		output_++;
	}

    esp_task_wdt_reset();
}

void SetSwitchStatus(uint8_t output, bool ItemStatus) {
    Relay* relay_ = &Relay1;
	uint8_t relayIndex_ = 1;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
            if (relayIndex_ == output) {
                if (ItemStatus) {
                    relay_->On();
                }
                else {
                    relay_->Off();
                }
            }
		}
		relay_ = (Relay*)relay_->getNext();;
		relayIndex_++;
	}
}
