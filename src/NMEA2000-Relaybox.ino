

#include <esp_task_wdt.h>

#include "common.h"
#include "webhandling.h"
#include "version.h"
#include "CZoneSwitches.h"

bool SaveConfiguration = false;
bool ChangedConfiguration = false;

tN2kSyncScheduler SwitchBankStatusScheduler(false, 1000, 500);

#define WDT_TIMEOUT 5

char Version[] = VERSION_STR; // Manufacturer's Software version code

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    Serial.printf("Firmware version:%s\n", Version);

    webinit();

	CZoneSetup();

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
    Serial.println("device initilized");
}

void loop() {
    webLoop();

	CZoneLoop();

    ChangedConfiguration = false;

    esp_task_wdt_reset();
}

void SetOutput(uint8_t output, bool state) {
    Relay* relay_ = &Relay1;
	uint8_t relayIndex_ = 1
        ;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
            if (relayIndex_ == output) {
                if (state) {
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
