// use the following Pins
#define ESP32_CAN_TX_PIN GPIO_NUM_5  // Set CAN TX port to D5 
#define ESP32_CAN_RX_PIN GPIO_NUM_4  // Set CAN RX port to D4

#include <esp_mac.h>
#include <time.h>
#include <sys/time.h>
#include <cmath>

#include "handleNMEA.h"
#include "common.h"
#include <NMEA2000_CAN.h>
#include <N2kTimer.h>
#include <EventLogger.h>

uint8_t SwitchBankInstance = 1;
uint8_t N2KSource = 22;
uint8_t N2KSID = 255;
uint8_t RelayAddress = 1;

uint8_t NumberOfSwitches = 8;   // change to 4 for bit switch bank

bool TimeSet = false;
unsigned long LastTimeUpdate = 0;

namespace {
    portMUX_TYPE sunDataMux = portMUX_INITIALIZER_UNLOCKED;
    double currentLatitude = 0;
    double currentLongitude = 0;
    bool positionValid = false;
    unsigned long lastPositionUpdate = 0;
}

SunTimes::Result getSunTimes() {
    portENTER_CRITICAL(&sunDataMux);
    bool timeValid_ = TimeSet;
    bool valid_ = positionValid && millis() - lastPositionUpdate < 60000;
    double latitude_ = currentLatitude;
    double longitude_ = currentLongitude;
    portEXIT_CRITICAL(&sunDataMux);

    SunTimes::Result result_;
    if (!timeValid_) {
        return result_;
    }
    if (!valid_) {
        result_.status = SunTimes::Status::PositionUnavailable;
        return result_;
    }
    return SunTimes::calculate(time(nullptr), latitude_, longitude_);
}

void SetChangeSwitchState(uint8_t SwitchIndex, bool ItemStatus);
void ParseN2kSystemTime(const tN2kMsg& N2kMsg);
void ParseN2kGNSS(const tN2kMsg& N2kMsg);

tN2kSyncScheduler BinaryStatusTimer(false, 1000, 500);

tN2kBinaryStatus SwitchBankStatus = 0;

typedef struct {
    unsigned long PGN;
    void (*Handler)(const tN2kMsg& N2kMsg);
} tNMEA2000Handler;

tNMEA2000Handler NMEA2000Handlers[] = {
    {126992L,ParseN2kSystemTime},
    {127502L,ParseSwitchBankControl},
    {129029L,ParseN2kGNSS},
    {0,0}
};

// List here messages your device will transmit.
const unsigned long TransmitMessages[] PROGMEM = {
    127501L,
    127502L,
    0
};

const unsigned long ReciveMessages[] PROGMEM = {
    126992L,
    127502L,
    129029L,
    0
};

void SetSystemTime(uint16_t DaysSince1970, double SecondsSinceMidnight) {
    if (DaysSince1970 == 0xFFFF || !std::isfinite(SecondsSinceMidnight)
        || SecondsSinceMidnight < 0 || SecondsSinceMidnight > 86400) {
        LOG_DEBUG("Invalid NMEA2000 system time");
        return;
    }

    if (TimeSet && (millis() - LastTimeUpdate < 600000)) {
        return;
    }

    time_t timestamp_ = (DaysSince1970 * 86400UL) + (time_t)SecondsSinceMidnight;

    struct timeval tv_;
    tv_.tv_sec = timestamp_;
    tv_.tv_usec = 0;
    if (settimeofday(&tv_, NULL) != 0) {
        LOG_ERROR("Failed to set system time from NMEA2000");
        return;
    }

    portENTER_CRITICAL(&sunDataMux);
    TimeSet = true;
    portEXIT_CRITICAL(&sunDataMux);
    LastTimeUpdate = millis();

    char timeStr_[64];
    struct tm* timeinfo_ = gmtime(&timestamp_);
    strftime(timeStr_, sizeof(timeStr_), "%Y-%m-%d %H:%M:%S", timeinfo_);
    LOG_OK("System time set from NMEA2000: %s UTC", timeStr_);
}

void ParseN2kSystemTime(const tN2kMsg& N2kMsg) {
    unsigned char SID_;
    uint16_t SystemDate_;
    double SystemTime_;
    tN2kTimeSource TimeSource_;

    if (ParseN2kPGN126992(N2kMsg, SID_, SystemDate_, SystemTime_, TimeSource_)) {
        if (TimeSource_ == N2ktimes_GPS || TimeSource_ == N2ktimes_GLONASS) {
            SetSystemTime(SystemDate_, SystemTime_);
        }
    } else {
        LOG_ERROR("Failed to parse PGN 126992");
    }
}

void ParseN2kGNSS(const tN2kMsg& N2kMsg) {
    unsigned char SID_;
    uint16_t DaysSince1970_;
    double SecondsSinceMidnight_;
    double Latitude_;
    double Longitude_;
    double Altitude_;
    tN2kGNSStype GNSStype_;
    tN2kGNSSmethod GNSSmethod_;
    unsigned char nSatellites_;
    double HDOP_;
    double PDOP_;
    double GeoidalSeparation_;
    unsigned char nReferenceStations_;
    tN2kGNSStype ReferenceStationType_;
    uint16_t ReferenceSationID_;
    double AgeOfCorrection_;

    if (ParseN2kPGN129029(N2kMsg, SID_, DaysSince1970_, SecondsSinceMidnight_,
        Latitude_, Longitude_, Altitude_, GNSStype_, GNSSmethod_,
        nSatellites_, HDOP_, PDOP_, GeoidalSeparation_,
        nReferenceStations_, ReferenceStationType_,
        ReferenceSationID_, AgeOfCorrection_)) {
        SetSystemTime(DaysSince1970_, SecondsSinceMidnight_);
        bool valid_ = GNSSmethod_ >= N2kGNSSm_GNSSfix && GNSSmethod_ <= N2kGNSSm_RTKFloat
            && std::isfinite(Latitude_) && std::isfinite(Longitude_)
            && Latitude_ >= -90 && Latitude_ <= 90 && Longitude_ >= -180 && Longitude_ <= 180;
        portENTER_CRITICAL(&sunDataMux);
        positionValid = valid_;
        if (valid_) {
            currentLatitude = Latitude_;
            currentLongitude = Longitude_;
            lastPositionUpdate = millis();
        }
        portEXIT_CRITICAL(&sunDataMux);
        if (!valid_) {
            LOG_DEBUG("GNSS position unavailable for sunrise/sunset calculation");
        }
    } else {
        LOG_ERROR("Failed to parse PGN 129029");
    }
}

void PrintBinaryStatus(tN2kBinaryStatus status) {
    char buffer_[65]; // 64 Bits + Nullterminator
    for (int i_ = 63; i_ >= 0; i_--) {
        buffer_[63 - i_] = (status & (1ULL << i_)) ? '1' : '0';
    }
    buffer_[64] = '\0'; // Nullterminator
    WEB_LOG_DEBUG("BinaryStatus: %s", buffer_);
}

void setupNMEA() {
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

    NMEA2000.SetOnOpen(openNMEA);
    NMEA2000.SetMsgHandler(HandleN2kMessages);

    NMEA2000.Open();
}

void openNMEA() {
	BinaryStatusTimer.UpdateNextTime();
}

void loopNMEA() {
    NMEA2000.ParseMessages();

	if (BinaryStatusTimer.IsTime()) {
		BinaryStatusTimer.UpdateNextTime();
        SendBinaryStatusReport(SwitchBankInstance);
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
	WEB_LOG_DEBUG("SetChangeSwitchState");
    String status_ = ItemStatus == N2kOnOff_On ? "On" : "Off";
    WEB_LOG_DEBUG_AT(1, "Switch %d is %s", SwitchIndex, status_.c_str());

    SetSwitchStatus(SwitchIndex, ItemStatus);

    //send out change and status to other N2k devices on network
    SendBinaryStatusReport(SwitchBankInstance);
    SendSwitchBankControl(SwitchBankInstance, SwitchIndex, ItemStatus);
}

//************************************************************************************************************
//  PGN127501 sent from the switching device to the network to inform of a change in switch state
//  The message is sent in response to a change in switch state and is sent to all other devices on the network
//************************************************************************************************************

void SendBinaryStatusReport(unsigned char DeviceInstance) {
	WEB_LOG_DEBUG("SendBinaryStatusReport");
    tN2kMsg N2kMsg_;
	//PrintBinaryStatus(SwitchBankStatus);
    SetN2kBinaryStatus(N2kMsg_, DeviceInstance, SwitchBankStatus);
    NMEA2000.SendMsg(N2kMsg_);
}

//************************************************************************************************************
//  PGN127502 sent from the switching device to the network to inform of a change in switch state
//  The message is sent in response to a change in switch state and is sent to all other devices on the network
//************************************************************************************************************

void SendSwitchBankControl(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus){
	WEB_LOG_DEBUG("SendSwitchBankControl");
	WEB_LOG_DEBUG_AT(1, "Switch %d is %s", SwitchIndex, ItemStatus == N2kOnOff_On ? "On" : "Off");
    tN2kMsg N2kMsg_;
	tN2kOnOff State_ = ItemStatus ? N2kOnOff_On : N2kOnOff_Off;
    N2kSetStatusBinaryOnStatus(SwitchBankStatus, State_, SwitchIndex + RelayAddress);
	PrintBinaryStatus(SwitchBankStatus);
    SetN2kSwitchbankControl(N2kMsg_, SwitchBankInstance, SwitchBankStatus);
    NMEA2000.SendMsg(N2kMsg_);
}

void ParseSwitchBankControl(const tN2kMsg& N2kMsg) {
	WEB_LOG_DEBUG("ParseSwitchBankControl");

	tN2kBinaryStatus BinaryStatus_;
	N2kResetBinaryStatus(BinaryStatus_);

	if (ParseN2kSwitchbankControl(N2kMsg, SwitchBankInstance, BinaryStatus_)) {
		for (uint8_t Index_ = 0; Index_ < NumberOfSwitches; Index_++) {
			if (Index_ > 63) break; // only 64 bits in the message
			tN2kOnOff State_ = N2kGetStatusOnBinaryStatus(BinaryStatus_, Index_ + RelayAddress);
            if (State_ != N2kOnOff_Unavailable) {
				N2kSetStatusBinaryOnStatus(SwitchBankStatus, State_, Index_ + RelayAddress);
				PrintBinaryStatus(SwitchBankStatus);
                SetChangeSwitchState(Index_, (State_ == N2kOnOff_On ? true : false));

            }
		}
	}
}


void SendSwitchStatus(uint8_t SwitchIndex, bool ItemStatus) {
    SetChangeSwitchState(SwitchIndex, ItemStatus);
}


