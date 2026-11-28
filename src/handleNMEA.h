// handleNMEA.h

#ifndef _HANDLENMEA_h
#define _HANDLENMEA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <N2kMessages.h>
#include "SunTimes.h"

SunTimes::Result getSunTimes();

void setupNMEA();
void openNMEA();
void loopNMEA();

void HandleN2kMessages(const tN2kMsg& N2kMsg);

void  SendBinaryStatusReport(unsigned char DeviceInstance);

void SendSwitchBankControl(unsigned char DeviceInstance, uint8_t SwitchIndex, bool ItemStatus);
void ParseSwitchBankControl(const tN2kMsg& N2kMsg);

void SendSwitchStatus(uint8_t SwitchIndex, bool ItemStatus);
extern void SetSwitchStatus(uint8_t output, bool ItemStatus) __attribute__ ((weak));


#endif






