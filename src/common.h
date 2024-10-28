// common.h

#ifndef _COMMON_h
#define _COMMON_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define MAX_RELAYS 8


extern bool debugMode;


#define DEBUG_PRINT(x) if (debugMode) Serial.print(x) 
#define DEBUG_PRINTLN(x) if (debugMode) Serial.println(x)
#define DEBUG_PRINTF(...) if (debugMode) Serial.printf(__VA_ARGS__)

extern bool ChangedConfiguration;
extern bool SaveConfiguration;

extern uint8_t N2KSource;
extern uint8_t SwitchBankInstance;
extern uint8_t RelayAddress;
extern uint8_t NumberOfSwitches;

extern char Version[];

#define STRING_LEN 40
#define NUMBER_LEN 10


#endif

