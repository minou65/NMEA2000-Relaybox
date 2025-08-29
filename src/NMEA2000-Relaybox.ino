
#define TESTMODE 0 // Enable test mode for relays (only first run, then normal operation). Set to 0 to disable test mode.

#include "common.h"
#include "webhandling.h"
#include "version.h"
#include "NMEAHandling.h"

#include "neotimer.h"


#if TESTMODE == 1

// Test mode variables
bool relayTestActive = true;
uint8_t relayTestIndex = 0;
bool relayTestTurnOff = false;
Neotimer relayTestTimer = Neotimer(1000); // 1 second timer

#endif // TESTMODE == 1



bool SaveConfiguration = false;
bool ChangedConfiguration = false;

char Version[] = VERSION_STR; // Manufacturer's Software version code

void setup() {

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    Serial.printf("Firmware version:%s\n", Version);

    webinit();
	N2kBegin();

    Serial.println("device initilized");

#if TESTMODE == 1
    relayTestTimer.start();
#endif // TESTMODE == 1

}

void loop() {
  
    webLoop();
    N2kLoop();
    ChangedConfiguration = false;

#if TESTMODE == 1
    // Relay test loop
    if (relayTestActive) {
        if (!relayTestTurnOff) {
            if (relayTestTimer.done()) {
                // Turn on next relay
                SetSwitchStatus(relayTestIndex, true);
                relayTestIndex++;
                relayTestTimer.start();

                // Count active relays
                uint8_t activeCount = 0;
                Relay* relay_ = &Relay1;
                while (relay_ != nullptr) {
                    if (relay_->isActive()) activeCount++;
                    relay_ = (Relay*)relay_->getNext();
                }

                // If all relays have been turned on, prepare to turn all off
                if (relayTestIndex >= activeCount) {
                    relayTestTurnOff = true;
                    relayTestTimer.start();
                }
            }
        }
        else {
            if (relayTestTimer.done()) {
                // Turn all relays off
                uint8_t idx = 0;
                Relay* relay_ = &Relay1;
                while (relay_ != nullptr) {
                    if (relay_->isActive()) {
                        SetSwitchStatus(idx, false);
                    }
                    relay_ = (Relay*)relay_->getNext();
                    idx++;
                }
                relayTestActive = false; // Stop test mode after one run
            }
        }
        return; // Skip normal relay processing during test
    }
#endif // TESTMODE == 1

    uint8_t output_ = 0;
    Relay* relay_ = &Relay1;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
            if (relay_->isTimerDone()) {
                SendSwitchStatus(output_, false);
            }
            relay_->process();
        }
        relay_ = (Relay*)relay_->getNext();
        output_++;
    }
}

void SetSwitchStatus(uint8_t output, bool ItemStatus) {
    Relay* relay_ = &Relay1;
    uint8_t relayIndex_ = 0;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
            if (relayIndex_ == output) {
                if (relay_->Status() != (ItemStatus ? N2kOnOff_On : N2kOnOff_Off)) {
                    if (ItemStatus) {
                        relay_->On();
                    }
                    else {
                        relay_->Off();
                    }
                    SendSwitchStatus(output, ItemStatus);
                }
            }
        }
        relay_ = (Relay*)relay_->getNext();
        relayIndex_++;
    }
}
