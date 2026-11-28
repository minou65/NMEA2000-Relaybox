/*
Name: NMEA2000-Relaybox

Description: 
NMEA2000 Relaybox firmware for ESP32/ESP8266. Provides web interface for configuration 
and control of relays, and handles NMEA2000 communication.

Author: andy

WEMOS D1 MINI ESP32
*/
#define TESTMODE 0 // Enable test mode for relays (only first run, then normal operation). Set to 0 to disable test mode.

#include <Arduino.h>

#include "common.h"
#include "handleWeb.h"
#include "version.h"
#include <NMEA2000_esp32.h>
#include "handleNMEA.h"

#include <neotimer.h>


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

    LOG_INFO("Firmware version: %s", Version);

    setupWeb();
	setupNMEA();

    if (!SunTimes::setTimezone("CET-1CEST,M3.5.0,M10.5.0/3")) {
        LOG_ERROR("Failed to configure sun time timezone");
    }

    LOG_OK("Device initialized");

#if TESTMODE == 1
    relayTestTimer.start();
#endif // TESTMODE == 1

}

void loop() {
  
    loopWeb();
    loopNMEA();
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
                uint8_t activeCount_ = 0;
                Relay* relay_ = &Relay1;
                while (relay_ != nullptr) {
                    if (relay_->isActive()) activeCount_++;
                    relay_ = (Relay*)relay_->getNext();
                }

                // If all relays have been turned on, prepare to turn all off
                if (relayTestIndex >= activeCount_) {
                    relayTestTurnOff = true;
                    relayTestTimer.start();
                }
            }
        }
        else {
            if (relayTestTimer.done()) {
                // Turn all relays off
                uint8_t idx_ = 0;
                Relay* relay_ = &Relay1;
                while (relay_ != nullptr) {
                    if (relay_->isActive()) {
                        SetSwitchStatus(idx_, false);
                    }
                    relay_ = (Relay*)relay_->getNext();
                    idx_++;
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
