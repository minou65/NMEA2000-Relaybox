// CZoneSwitches.h

#ifndef _CZONESWITCHES_h
#define _CZONESWITCHES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define CzUpdatePeriod65284 2000
#define CzUpdatePeriod65283 500
#define CzUpdatePeriod127501 10000
#define CZoneMessage 0x9927 // 295 + reserved + industry code=marine
#define CzSwitchBank1SerialNum  0x1d // Serial Number switches 1-4 (00260128)
#define CzSwitchBank2SerialNum  0x1b // Serial Number switches 5-8  (00260126)
#define CzDipSwitch     200 //  CZone Dip switch value, CZone address
#define BinaryDeviceInstance 0x00 // Instance of 127501 switch state message
#define SwitchBankInstance 0x00   //Instance of 127502 change switch state message
#define NumberOfSwitches 8   // change to 4 for bit switch bank

void SetCZoneSwitchStateBroadcast130817(unsigned char);
void SetCZoneSwitchChangeAck65283(unsigned char); //acknowledgement of switch change or current state to MFD, PGN 65283 
void SetCZoneSwitchHeartbeat65284(unsigned char); // Tell the MDF the switch bank is still online
void SetCZoneSendConfigToMFD65290(unsigned char, uint8_t, uint8_t, uint8_t);

void SetCZoneSwitchState127501(unsigned char);  // N2K compatability
void SetCZoneSwitchChangeRequest127502(unsigned char, uint8_t, bool); // N2K compatability
void ParseCZoneMFDSwitchChangeRequest65280(const tN2kMsg&); // MFD requests change of a switch PGN 65280
void ParseCZoneConfigRequest65290(const tN2kMsg&);
void ParseMDF65284(const tN2kMsg&);
void ParseMDF65288(const tN2kMsg&);



#endif

