// use the following Pins
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include "CZoneSwitches.h"
#include "common.h"
#include <NMEA2000_CAN.h>
#include <N2kTimer.h>

bool debugMode = true;

#define DEBUG_PRINT(x) if (debugMode) Serial.print(x) 
#define DEBUG_PRINTLN(x) if (debugMode) Serial.println(x)
#define DEBUG_PRINTF(...) if (debugMode) Serial.printf(__VA_ARGS__)


uint8_t BinaryDeviceInstance = 1; // Instance of 127501 switch state message
uint8_t SwitchBankInstance = 1;   //Instance of 127502 change switch state message
uint8_t N2KSource = 22;
uint8_t N2KSID = 255;
uint8_t RelayAddress = 1;

#define NumberOfSwitches 8   // change to 4 for bit switch bank

void SetChangeSwitchState(uint8_t SwitchIndex, bool ItemStatus);

tN2kSyncScheduler BinaryStatusTimer(false, 1000, 500);

tN2kBinaryStatus SwitchBankStatus = 0;

typedef struct {
    unsigned long PGN;
    void (*Handler)(const tN2kMsg& N2kMsg);
} tNMEA2000Handler;

tNMEA2000Handler NMEA2000Handlers[] = {
    {127502L,ParseSwitchBankCommand},
    {0,0}
};

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    65283L,
    127501L,
    127502L,
    0
};

const unsigned long ReciveMessages[] PROGMEM = {
    127501L,
    127502L,
    0
};

void PrintBinaryStatus(tN2kBinaryStatus status) {
    char buffer[65]; // 64 Bits + Nullterminator
    for (int i = 63; i >= 0; i--) {
        buffer[63 - i] = (status & (1ULL << i)) ? '1' : '0';
    }
    buffer[64] = '\0'; // Nullterminator
    Serial.printf("DisplayStatus: %s\n", buffer);
}

void N2kBegin() {
    uint8_t chipid_[6];
    uint32_t id_ = 0;

    // Generate unique number from chip id
    esp_efuse_mac_get_default(chipid_);
    for (int i_ = 0; i_ < 6; i_++) id_ += (chipid_[i_] << (7 * i_));

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
        Version, // Manufacturer's Model version
        3 // Load Equivalency
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

    NMEA2000.SetOnOpen(N2kOpen);
    NMEA2000.SetMsgHandler(HandleN2kMessages);

    NMEA2000.Open();
}

void N2kOpen() {
	BinaryStatusTimer.UpdateNextTime();
}

void N2kLoop() {
    NMEA2000.ParseMessages();

	if (BinaryStatusTimer.IsTime()) {
		BinaryStatusTimer.UpdateNextTime();
        SendBinaryStatus(BinaryDeviceInstance);
	}

    if (NMEA2000.GetN2kSource() != N2KSource) {
        N2KSource = NMEA2000.GetN2kSource();
        SaveConfiguration = true;
    }
}

void HandleN2kMessages(const tN2kMsg& N2kMsg) {
    uint16_t Handler_;
    for (Handler_ = 0; NMEA2000Handlers[Handler_].PGN != 0 && !(N2kMsg.PGN == NMEA2000Handlers[Handler_].PGN); Handler_++);
    if (NMEA2000Handlers[Handler_].PGN != 0) {
        NMEA2000Handlers[Handler_].Handler(N2kMsg);
    }
}

void SetChangeSwitchState(uint8_t SwitchIndex, bool ItemStatus) {
	DEBUG_PRINTLN("SetChangeSwitchState");
    String status_ = ItemStatus == N2kOnOff_On ? "On" : "Off";
    DEBUG_PRINTF("    Switch %d is %s\n", SwitchIndex, status_.c_str());

    SetSwitchStatus(SwitchIndex, ItemStatus);

    //send out change and status to other N2k devices on network
    SendBinaryStatus(BinaryDeviceInstance);
    SendSwitchBankCommand(BinaryDeviceInstance, SwitchIndex, ItemStatus);
}

void SetN2kSwitchBankCommand(tN2kMsg& N2kMsg, unsigned char DeviceBankInstance, tN2kBinaryStatus BankStatus){
    SetN2kPGN127502(N2kMsg, DeviceBankInstance, BankStatus);
}

//************************************************************************************************************
//  PGN127501 sent from the switching device to the network to inform of a change in switch state
//  The message is sent in response to a change in switch state and is sent to all other devices on the network
//************************************************************************************************************

void SendBinaryStatus(unsigned char DeviceInstance) {
	DEBUG_PRINTLN("SendBinaryStatus");
    tN2kMsg N2kMsg_;
	//PrintBinaryStatus(SwitchBankStatus);
    SetN2kBinaryStatus(N2kMsg_, DeviceInstance, SwitchBankStatus);
    NMEA2000.SendMsg(N2kMsg_);
}

//************************************************************************************************************
//  PGN127502 sent from the switching device to the network to inform of a change in switch state
//  The message is sent in response to a change in switch state and is sent to all other devices on the network
//************************************************************************************************************

void SendSwitchBankCommand(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus){
	DEBUG_PRINTLN("SendSwitchBankCommand");
    tN2kMsg N2kMsg_;
    tN2kBinaryStatus BinaryStatus_;
    N2kResetBinaryStatus(BinaryStatus_);
    N2kSetStatusBinaryOnStatus(BinaryStatus_, ItemStatus ? N2kOnOff_On : N2kOnOff_Off, SwitchIndex);
    //send out to other N2k switching devices on network ( pgn 127502 )
    SetN2kSwitchBankCommand(N2kMsg_, SwitchBankInstance, BinaryStatus_);
    NMEA2000.SendMsg(N2kMsg_);
}

void ParseSwitchBankCommand(const tN2kMsg& N2kMsg) {
	DEBUG_PRINTLN("ParseSwitchBankCommand");

	tN2kBinaryStatus BinaryStatus_;
	N2kResetBinaryStatus(BinaryStatus_);

	if (ParseN2kSwitchbankControl(N2kMsg, BinaryDeviceInstance, BinaryStatus_)) {
		for (uint8_t Index_ = 0; Index_ < NumberOfSwitches; Index_++) {
			if (Index_ > 63) break; // only 64 bits in the message
			tN2kOnOff State_ = N2kGetStatusOnBinaryStatus(BinaryStatus_, Index_ + 1);
            if (State_ != N2kOnOff_Unavailable) {
				N2kSetStatusBinaryOnStatus(SwitchBankStatus, State_, Index_ + 1);
				// PrintBinaryStatus(SwitchBankStatus);
                SetChangeSwitchState(Index_ + 1, (State_ == N2kOnOff_On ? true : false));

            }
		}
	}
}


void SendSwitchStatus(uint8_t SwitchIndex, bool ItemStatus) {
    SetChangeSwitchState(SwitchIndex, ItemStatus);
}


