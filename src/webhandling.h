// webhandling.h

#ifndef _WEBHANDLING_h
#define _WEBHANDLING_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <IotWebConf.h>
#include <N2kMessages.h>
#include <IotWebConfOptionalGroup.h>

#include "neotimer.h"


// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "123456789";

extern void webinit();
extern void webLoop();

static WiFiClient wifiClient;
extern IotWebConf iotWebConf;

class NMEAConfig : public iotwebconf::ParameterGroup {
public:
    NMEAConfig() : ParameterGroup("nmeaconfig", "NMEA configuration") {
        snprintf(instanceID, STRING_LEN, "%s-instance", this->getId());
        snprintf(sidID, STRING_LEN, "%s-sid", this->getId());
        snprintf(sourceID, STRING_LEN, "%s-source", this->getId());
        snprintf(instanceBank, STRING_LEN, "%s-bank", this->getId());
        snprintf(addressRelay, STRING_LEN, "%s-releay", this->getId());

        this->addItem(&this->InstanceParam);
        this->addItem(&this->SIDParam);
        this->addItem(&this->BankParam);
        this->addItem(&this->RelayParam);

        iotWebConf.addHiddenParameter(&SourceParam);

    }

    uint8_t Instance() { return atoi(InstanceValue); };
    uint8_t SID() { return atoi(SIDValue); };
    uint8_t DeviceID() { return atoi(BankValue); };
    uint8_t RelayAddress() { return atoi(RelayValue); };
    uint8_t Source() { return atoi(SourceValue); };

    void SetSource(uint8_t source) {
        String s_;
        s_ = (String)source;
        strncpy(SourceParam.valueBuffer, s_.c_str(), NUMBER_LEN);
    }


private:
    iotwebconf::NumberParameter InstanceParam = iotwebconf::NumberParameter("Instance", instanceID, InstanceValue, NUMBER_LEN, "255", "1..255", "min='1' max='254' step='1'");
    iotwebconf::NumberParameter SIDParam = iotwebconf::NumberParameter("SID", sidID, SIDValue, NUMBER_LEN, "255", "1..255", "min='1' max='255' step='1'");
    iotwebconf::NumberParameter BankParam = iotwebconf::NumberParameter("Device Address", instanceBank, BankValue, NUMBER_LEN, "1", "1..254", "min='1' max='254' step='1'");
    iotwebconf::NumberParameter RelayParam = iotwebconf::NumberParameter("Relay Address", addressRelay, RelayValue, NUMBER_LEN, "1", "1..28", "min='1' max='28' step='1'");
    iotwebconf::NumberParameter SourceParam = iotwebconf::NumberParameter("Source", sourceID, SourceValue, NUMBER_LEN, "22", nullptr, nullptr);

    char InstanceValue[NUMBER_LEN];
    char SIDValue[NUMBER_LEN];
    char BankValue[NUMBER_LEN];
    char RelayValue[NUMBER_LEN];
    char SourceValue[NUMBER_LEN];


    char instanceID[STRING_LEN];
    char sidID[STRING_LEN];
    char instanceBank[STRING_LEN];
    char addressRelay[STRING_LEN];
    char sourceID[STRING_LEN];
};


class Relay : public iotwebconf::ChainedParameterGroup {
public:
    Relay(const char* id, const uint8_t gpio_) : ChainedParameterGroup(id, id) {

        snprintf(_gpioID, STRING_LEN, "%s-gpio", this->getId());
        snprintf(_offID, STRING_LEN, "%s-offtime", this->getId());

        this->addItem(&this->gpioParam);
        this->addItem(&this->offParam);
    }

    void begin() {
        uint8_t gpio_ = GPIO();
        if ((gpio_ == 255)) return;
        pinMode(gpio_, OUTPUT);
        Off();
    };

    void process() {
		if (OffTimer.done()) {
			Serial.println("Relay Off");
			OffTimer.stop();
			Off();
		}
    }

    int8_t GPIO() { return atoi(_gpioValue); };

    void On() {
        uint8_t gpio_ = GPIO();
        if ((gpio_ == 255)) return;
        digitalWrite(gpio_, HIGH);
        _Status = N2kOnOff_On;
		if (atoi(_offValue) > 0) {
			OffTimer.start(atoi(_offValue) * 1000);
		}
    }

    void Off() {
        uint8_t gpio_ = GPIO();
        if ((gpio_ == 255)) return;
        digitalWrite(gpio_, LOW);
        _Status = N2kOnOff_Off;
    }

    tN2kOnOff Status() {
        return _Status;
    }

	bool isEnable() {
        return Status() == N2kOnOff_On;
	}

	bool isTimerDone() {
		return OffTimer.done();
	}

    uint16_t offTime() {
        return atoi(_offValue);
    };

    iotwebconf::NumberParameter gpioParam = iotwebconf::NumberParameter("GPIO", _gpioID, _gpioValue, NUMBER_LEN, "-1", "0..255", "min='-1' max='255' step='1'");
    iotwebconf::NumberParameter offParam = iotwebconf::NumberParameter("Off Time (s)", _offID, _offValue, NUMBER_LEN, "0", "0..3000", "min='0' max='3000' step='1'");


private:

    char _gpioValue[NUMBER_LEN];
    char _gpioID[STRING_LEN];

    char _offValue[NUMBER_LEN];
    char _offID[STRING_LEN];

    tN2kOnOff _Status;

	Neotimer OffTimer = Neotimer();
};
extern Relay Relay1;
extern Relay Relay2;
extern Relay Relay3;
extern Relay Relay4;
extern Relay Relay5;
extern Relay Relay6;
extern Relay Relay7;
extern Relay Relay8;



#endif

