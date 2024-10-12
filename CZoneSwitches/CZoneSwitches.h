// CZoneSwitches.h

#ifndef _CZONESWITCHES_h
#define _CZONESWITCHES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <N2kMessages.h>
#include <N2kTimer.h>


#define CZoneMessage 0x9927 // 295 + reserved + industry code=marine
#define CzSwitchBank1SerialNum  0x1d // Serial Number switches 1-4 (00260128)
#define CzSwitchBank2SerialNum  0x1b // Serial Number switches 5-8  (00260126)
#define CzDipSwitch     200 //  CZone Dip switch value, CZone address
#define BinaryDeviceInstance 0x00 // Instance of 127501 switch state message
#define SwitchBankInstance 0x00   //Instance of 127502 change switch state message
#define NumberOfSwitches 8   // change to 4 for bit switch bank


void CZoneSetup();
void CZoneOpen();
void CZoneLoop();

void CZoneHandleMessage(const tN2kMsg& N2kMsg);

void ParseCZoneMFDSwitchChangeRequest65280(const tN2kMsg& N2kMsg);

void SendCZoneSwitchChangeAck65283(unsigned char CzSwitchBankSerialNum);

void SendCZoneSwitchHeartbeat65284(unsigned char CzSwitchBankSerialNum);
void ParseMDF65284(const tN2kMsg& N2kMsg);

void ParseMDF65288(const tN2kMsg& N2kMsg);

void SendCZoneSendConfigToMFD65290(unsigned char CzSwitchBankSerial, uint8_t CZoneConfig0, uint8_t CZoneConfig1, uint8_t CZoneConfig2);
void ParseCZoneConfigRequest65290(const tN2kMsg& N2kMsg);

void SendCZoneSwitchStateBroadcast130817(unsigned char CzSwitchBankSerialNum);

void SendCZoneSwitchState127501(unsigned char DeviceInstance);

void SendCZoneSwitchChangeRequest127502(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus);
void ParseN2kPGN127502(const tN2kMsg& N2kMsg);


#endif






