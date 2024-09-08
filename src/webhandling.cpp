// 
// 
// 
#include <Arduino.h>
#include <ArduinoOTA.h>
#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>      
#endif

#include "common.h"
#include "webhandling.h"
#include "favicon.h"

#include <DNSServer.h>
#include <IotWebConfTParameter.h>
#include <IotWebConfAsyncClass.h>
#include <IotWebConfAsyncUpdateServer.h>
#include <IotWebRoot.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "A1"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN  GPIO_NUM_13

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN
#if ESP32 
#define ON_LEVEL HIGH
#else
#define ON_LEVEL LOW
#endif

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "NMEA2000-Relaybox";

// -- Method declarations.
void handleData(AsyncWebServerRequest* request);
void handleRoot(AsyncWebServerRequest* request);
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

extern const uint8_t maxRelays;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebServerWrapper asyncWebServerWrapper(&server);
AsyncUpdateServer AsyncUpdater;

IotWebConf iotWebConf(thingName, &dnsServer, &asyncWebServerWrapper, wifiInitialApPassword, CONFIG_VERSION);

NMEAConfig Config = NMEAConfig();
Relay* relays[maxRelays]; // Array von Relay-Pointern

void webinit() {


}


