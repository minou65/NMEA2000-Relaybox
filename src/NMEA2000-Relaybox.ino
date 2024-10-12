// use the following Pins
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <esp_task_wdt.h>
#include <NMEA2000_CAN.h>

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

tN2kSyncScheduler SwitchBankStatusScheduler(false, 1000, 500);

#define WDT_TIMEOUT 5

char Version[] = VERSION_STR; // Manufacturer's Software version code

const unsigned long TransmitMessages[] PROGMEM = {
    127501L,
    127502L,
    0
};

const unsigned long ReciveMessages[] PROGMEM = {
    127501L,
    127502L,
    0
};

typedef struct {
    unsigned long PGN;
    void (*Handler)(const tN2kMsg& N2kMsg);
} tNMEA2000Handler;

tNMEA2000Handler NMEA2000Handlers[] = {
    {127502L,HandleSwitchbankControl},
    {0,0}
};

void OnN2kOpen() {
    SwitchBankStatusScheduler.UpdateNextTime();
}

void setup() {
    uint8_t chipid_[6];
    uint32_t id_ = 0;

    // Generate unique number from chip id
    esp_efuse_mac_get_default(chipid_);
    for (int i_ = 0; i_ < 6; i_++) id_ += (chipid_[i_] << (7 * i_));

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
        id_, // Unique number. Use e.g. Serial number.
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

    NMEA2000.SetMsgHandler(HandleN2kMessage);

    NMEA2000.Open();

    esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
    esp_task_wdt_add(NULL); //add current thread to WDT watch
    Serial.println("device initilized");
}

void loop() {
    webLoop();

	SendSwitchBankStatus();

    NMEA2000.ParseMessages();

    Relay* relay_ = &Relay1;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
            relay_->process();
        }
        relay_ = (Relay*)relay_->getNext();
    }

    ChangedConfiguration = false;

    esp_task_wdt_reset();
}

void ControlRelay(tN2kOnOff state_, Relay* relay_) {
    switch (state_) {
    case N2kOnOff_Off:
        relay_->Off();
        break;
    case N2kOnOff_On:
        relay_->On();
        break;
    }
}

void HandleN2kMessage(const tN2kMsg& N2kMsg_) {
    int iHandler_;
    for (iHandler_ = 0; NMEA2000Handlers[iHandler_].PGN != 0 && !(N2kMsg_.PGN == NMEA2000Handlers[iHandler_].PGN); iHandler_++);
    if (NMEA2000Handlers[iHandler_].PGN != 0) {
        NMEA2000Handlers[iHandler_].Handler(N2kMsg_);
    }
}

// Callback function for handling switch bank control messages
void HandleSwitchbankControl(const tN2kMsg& N2kMsg_) {
    tN2kBinaryStatus BinaryStatus_;

    if (ParseN2kSwitchbankControl(N2kMsg_, DeviceInstance, BinaryStatus_)) {

        Relay* relay_ = &Relay1;
        uint8_t index_ = 0;
        while (relay_ != nullptr) {
            if (relay_->isActive()) {
                tN2kOnOff ItemStatus_ = N2kGetStatusOnBinaryStatus(BinaryStatus_, index_ + (RelayAddress));

                if (ItemStatus_ != relay_->Status()) {
                    ControlRelay(ItemStatus_, relay_);
                }
            }

            relay_ = (Relay*)relay_->getNext();
            index_++;
        }
    }
}

void SendSwitchBankStatus() {
    tN2kMsg N2kMsg_;
    tN2kBinaryStatus BinaryStatus_;

    if (!SwitchBankStatusScheduler.IsTime()) return;
    SwitchBankStatusScheduler.UpdateNextTime();

    N2kResetBinaryStatus(BinaryStatus_);

    Relay* relay_ = &Relay1;
    uint8_t index_ = 0;
    Serial.println("SendSwitchBankStatus");
	Serial.printf("    DeviceInstance:%d\n", DeviceInstance);
	Serial.printf("    RelayAddress:%d\n", RelayAddress);
    while (relay_ != nullptr) {
        if (relay_->isActive()) {
			Serial.printf("    Relay:%d\n", index_);
			Serial.printf("        Status:%d\n", relay_->Status());
            //tN2kOnOff ItemStatus_ = relay_->Status();
            tN2kOnOff ItemStatus_ = N2kOnOff_On;
            N2kSetStatusBinaryOnStatus(BinaryStatus_, ItemStatus_, index_ + (RelayAddress));
        }

        relay_ = (Relay*)relay_->getNext();
        index_++;
    }
    
    SetN2kBinaryStatus(N2kMsg_, DeviceInstance, BinaryStatus_);
    NMEA2000.SendMsg(N2kMsg_);
}
