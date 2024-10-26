// NMEAHandling.h

#ifndef _NMEAHANDLING_h
#define _NMEAHANDLING_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <N2kMessages.h>

void N2kBegin();
void N2kOpen();
void N2kLoop();

void HandleN2kMessages(const tN2kMsg& N2kMsg);

void  SendBinaryStatus(unsigned char DeviceInstance);

void SendSwitchBankControl(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus);
void ParseSwitchBankControl(const tN2kMsg& N2kMsg);

void SendSwitchStatus(uint8_t SwitchIndex, bool ItemStatus);
extern void SetSwitchStatus(uint8_t output, bool ItemStatus) __attribute__ ((weak));


#endif






