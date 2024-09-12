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

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "123456789";

extern void wifiInit();
extern void wifiLoop();

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
    uint8_t BankInstance() { return atoi(BankValue); };
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
    iotwebconf::NumberParameter BankParam = iotwebconf::NumberParameter("Bank Instance", instanceBank, RelayValue, NUMBER_LEN, "1", "1..255", "min='1' max='255' step='1'");
    iotwebconf::NumberParameter RelayParam = iotwebconf::NumberParameter("Relay Address", addressRelay, BankValue, NUMBER_LEN, "1", "1..28", "min='1' max='28' step='1'");
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


class Relay : public iotwebconf::ParameterGroup {
public:
    Relay(const char* relayId) : ParameterGroup(relayId, "Relay configuration") {
        snprintf(_outputPinID, STRING_LEN, "%s-outputpin", this->getId());
        this->addItem(&this->_OutputPinParam);
    }

    uint8_t OutputPin() { return atoi(_OutputPinValue); };

    void SetPinMode() {
        pinMode(OutputPin(), OUTPUT);
    };
    void Enable(){
        digitalWrite(OutputPin(), HIGH);
        _Status = N2kOnOff_On;
    }

    void Disable(){
        digitalWrite(OutputPin(), LOW);
        _Status = N2kOnOff_Off;

    }

    tN2kOnOff Status() {
        return _Status;
    }

private:
    iotwebconf::NumberParameter _OutputPinParam = iotwebconf::NumberParameter("OutputPin", _outputPinID, _OutputPinValue, NUMBER_LEN, "255", "1..255", "min='1' max='254' step='1'");
    char _OutputPinValue[NUMBER_LEN];
    char _outputPinID[STRING_LEN];
    tN2kOnOff _Status;
};

extern Relay* relays[MAX_RELAYS];

#endif

