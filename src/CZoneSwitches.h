// CZoneSwitches.h

#ifndef _CZONESWITCHES_h
#define _CZONESWITCHES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <N2kMessages.h>

void CZoneSetup();
void CZoneOpen();
void CZoneLoop();

void CZoneHandleMessage(const tN2kMsg& N2kMsg);

void SendCZoneSwitchState127501(unsigned char DeviceInstance);

void SendCZoneSwitchChangeRequest127502(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus);
void ParseN2kPGN127502(const tN2kMsg& N2kMsg);

void SendCZSwitchStatus(uint8_t SwitchIndex, bool ItemStatus);
extern void SetCZRelayOutput(uint8_t output, bool ItemStatus) __attribute__ ((weak));


#endif






