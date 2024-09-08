// use the following Pins
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <esp_task_wdt.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>

#include "common.h"
#include "webhandling.h"
#include "neotimer.h"
#include "version.h"


const uint8_t maxRelays = 4;
const uint8_t relayPins[maxRelays] = { 2, 3, 4, 5 }; // Pins connected to the relay module
tN2kOnOff relayStatus[maxRelays];


uint8_t N2KSource = 22;
uint8_t BankInstance = 1;
bool SaveConfiguration = false;
bool ChangedConfiguration = false;

#define WDT_TIMEOUT 5

char Version[] = VERSION_STR; // Manufacturer's Software version code

tN2kSyncScheduler SwitchBankStatusScheduler(false, 2000, 500);
Neotimer WDtimer = Neotimer((WDT_TIMEOUT + 1) * 1000);

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127501L, // Temperature
    0
};

void OnN2kOpen() {
    SwitchBankStatusScheduler.UpdateNextTime();
}

void setup() {
    uint8_t _chipid[6];
    uint32_t _id = 0;

    // Generate unique number from chip id
    esp_efuse_mac_get_default(_chipid);
    for (int _i = 0; _i < 6; _i++) _id += (_chipid[_i] << (7 * _i));

    Serial.begin(115200);
    while (!Serial) {
        delay(1);
    }

    Serial.printf("Firmware version:%s\n", Version);

    // Initialize relay pins as outputs
    for (int _i = 0; _i < maxRelays; _i++) {
        pinMode(relayPins[_i], OUTPUT);
        digitalWrite(relayPins[_i], LOW); // Ensure all relays are off initially
        relayStatus[_i] = N2kOnOff_Off;
    }

    // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
    NMEA2000.SetN2kCANMsgBufSize(150);
    NMEA2000.SetN2kCANReceiveFrameBufSize(250);
    NMEA2000.SetN2kCANSendFrameBufSize(250);

    // Set Product information
    NMEA2000.SetProductInformation(
        "1", // Manufacturer's Model serial code
        100, // Manufacturer's product code
        "Relaybox",  // Manufacturer's Model ID
        Version,  // Manufacturer's Software version code
        Version // Manufacturer's Model version
    );

    NMEA2000.SetDeviceInformation(
        _id, // Unique number. Use e.g. Serial number.
        135, // Device function=Switch Interface. See DEVICE_FUNCTION (0 - 255) https://canboat.github.io/canboat/canboat.html#main
        110, // Device class=Switch Interface. DEVICE_CLASS (0 - 127) https://canboat.github.io/canboat/canboat.html#main
        2040 // Just choosen free from code list on MANUFACTURER_CODE (0 - 2047) https://canboat.github.io/canboat/canboat.html#main
    );

    NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode);
    NMEA2000.SetN2kSource(N2KSource);
    NMEA2000.EnableForward(false);

    // Here we tell library, which PGNs we transmit
    NMEA2000.ExtendTransmitMessages(TransmitMessages);

    NMEA2000.SetOnOpen(OnN2kOpen);
    NMEA2000.SetMsgHandler(HandleSwitchbankControl);

    NMEA2000.Open();

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch

    WDtimer.start();

    ChangedConfiguration = true;
}

void loop() {
    SendSwitchBankStatus();
    NMEA2000.ParseMessages();

    if(NMEA2000.GetN2kSource() != N2KSource){
		N2KSource = NMEA2000.GetN2kSource();
        SaveConfiguration = true;
	}

    ChangedConfiguration = false;

    if (WDtimer.repeat()) {
        esp_task_wdt_reset();
    }
}


void ControlRelay(tN2kOnOff state, uint8_t Index) {

    switch (state) {
        case N2kOnOff_Off:
            digitalWrite(relayPins[Index], LOW);
            relayStatus[Index] = N2kOnOff_Off;
            break;
        case N2kOnOff_On:
            digitalWrite(relayPins[Index], HIGH);
            relayStatus[Index] = N2kOnOff_On;
            break;
    }

}

// Callback function for handling switch bank control messages
void HandleSwitchbankControl(const tN2kMsg& N2kMsg) {

    tN2kBinaryStatus _BankStatus;

    if (ParseN2kSwitchbankControl(N2kMsg, BankInstance, _BankStatus)) {

        for (uint8_t _i = 0; _i < maxRelays; _i++) {
            tN2kOnOff _ItemStatus = N2kGetStatusOnBinaryStatus(_BankStatus, _i);

            if (_ItemStatus != relayStatus[_i]) {
                ControlRelay(_ItemStatus, _i);
            }
        }
    }
}

void SendSwitchBankStatus(){
    tN2kMsg _N2kMsg;
    tN2kBinaryStatus _BankStatus;

    if(!SwitchBankStatusScheduler.IsTime()) return;
    SwitchBankStatusScheduler.UpdateNextTime();

    N2kResetBinaryStatus(_BankStatus);
    for (uint8_t _i = 0; _i < maxRelays; _i++) {
        tN2kOnOff _ItemStatus = relayStatus[_i];
        N2kSetStatusBinaryOnStatus(_BankStatus, _ItemStatus, _i);
    }


    SetN2kBinaryStatus(_N2kMsg, BankInstance, _BankStatus);
    
}
