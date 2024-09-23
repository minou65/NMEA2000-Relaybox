// use the following Pins
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <esp_task_wdt.h>
#include <N2kMessages.h>
#include <NMEA2000_CAN.h>
#include <vector>

#include "common.h"
#include "webhandling.h"
#include "neotimer.h"
#include "version.h"

void HandleSwitchbankControl(const tN2kMsg& N2kMsg);
void SendSwitchBankStatus();

uint8_t N2KSource = 22;
uint8_t DeviceInstance = 0;
uint8_t RelayAddress = 0;
bool SaveConfiguration = false;
bool ChangedConfiguration = false;

#define WDT_TIMEOUT 5

char Version[] = VERSION_STR; // Manufacturer's Software version code

tN2kSyncScheduler SwitchBankStatusScheduler(false, 2000, 500);

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127501L,
    0
};

const unsigned long ReciveMessages[] PROGMEM = {
    127502L,
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
    
    webinit();

    // Reserve enough buffer for sending all messages. This does not work on small memory devices like Uno or Mega
    NMEA2000.SetN2kCANMsgBufSize(8);
    NMEA2000.SetN2kCANReceiveFrameBufSize(150);
    NMEA2000.SetN2kCANSendFrameBufSize(150);

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

    // Here we tell library, which PGNs we transmit and recive
    NMEA2000.ExtendTransmitMessages(TransmitMessages);
    NMEA2000.ExtendReceiveMessages(ReciveMessages);

    NMEA2000.SetOnOpen(OnN2kOpen);
    NMEA2000.SetMsgHandler(HandleSwitchbankControl);

    NMEA2000.Open();

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
    Serial.println("device initilized");
}

void loop() {
    SendSwitchBankStatus();
    NMEA2000.ParseMessages();
    webLoop();

    Relay* _relay = &Relay1;
    while (_relay != nullptr) {
        if (_relay->isActive()) {
            _relay->process();
        }
		_relay = (Relay*)_relay->getNext();
    }

    if(NMEA2000.GetN2kSource() != N2KSource){
        N2KSource = NMEA2000.GetN2kSource();
        SaveConfiguration = true;
    }

    ChangedConfiguration = false;

    esp_task_wdt_reset();
}

void ControlRelay(tN2kOnOff state, Relay* relay) {
    switch (state) {
        case N2kOnOff_Off:
            relay->Off();
            break;
        case N2kOnOff_On:
            relay->On();
            break;
    }
}

// Callback function for handling switch bank control messages
void HandleSwitchbankControl(const tN2kMsg& N2kMsg) {
    tN2kBinaryStatus _BinaryStatus;

    if (ParseN2kSwitchbankControl(N2kMsg, DeviceInstance, _BinaryStatus)) {

        Relay* _relay = &Relay1;
		uint8_t _index = 0;
		while (_relay != nullptr) {
            if (_relay->isActive()) {
                tN2kOnOff _ItemStatus = N2kGetStatusOnBinaryStatus(_BinaryStatus, _index + (RelayAddress));

                if (_ItemStatus != _relay->Status()) {
                    ControlRelay(_ItemStatus, _relay);
                }
            }

			_relay = (Relay*)_relay->getNext();
			_index++;
		}
    }
}

void SendSwitchBankStatus() {
    tN2kMsg _N2kMsg;
    tN2kBinaryStatus _BinaryStatus;

    if(!SwitchBankStatusScheduler.IsTime()) return;
    SwitchBankStatusScheduler.UpdateNextTime();

    N2kResetBinaryStatus(_BinaryStatus);

    Relay* _relay = &Relay1;
    uint8_t _index = 0;
	while (_relay != nullptr) {
		if (_relay->isActive()) {
			tN2kOnOff _ItemStatus = _relay->Status();
			N2kSetStatusBinaryOnStatus(_BinaryStatus, _ItemStatus, _index + (RelayAddress));
		}

		_relay = (Relay*)_relay->getNext();
		_index++;
	}
    Serial.println("SendSwitchBankStatus");
    SetN2kBinaryStatus(_N2kMsg, DeviceInstance, _BinaryStatus);
    NMEA2000.SendMsg(_N2kMsg);
}
