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
#include "neotimer.h"

#include <DNSServer.h>
#include <IotWebConfTParameter.h>
#include <IotWebConfAsyncClass.h>
#include <IotWebConfAsyncUpdateServer.h>
#include <IotWebRoot.h>

#if ESP32
// Liste der gültigen Pins für ESP32
const int validPins[] = { 0, 1, 2, 3, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39 };
#endif


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

uint8_t APModeOfflineTime = 0;
Neotimer APModeTimer = Neotimer();

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "NMEA2000-Relaybox";

// -- Method declarations.
void handleData(AsyncWebServerRequest* request);
void handleRoot(AsyncWebServerRequest* request);
bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper);
void convertParams();

// -- Callback methods.
void configSaved();
void wifiConnected();

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebServerWrapper asyncWebServerWrapper(&server);
AsyncUpdateServer AsyncUpdater;

IotWebConf iotWebConf(thingName, &dnsServer, &asyncWebServerWrapper, wifiInitialApPassword, CONFIG_VERSION);

char APModeOfflineValue[STRING_LEN];
iotwebconf::NumberParameter APModeOfflineParam = iotwebconf::NumberParameter("AP offline mode after (minutes)", "APModeOffline", APModeOfflineValue, NUMBER_LEN, "0", "0..30", "min='0' max='30', step='1'");

NMEAConfig Config = NMEAConfig();

Relay Relay1 = Relay("Relay1", -1);
Relay Relay2 = Relay("Relay2", -1);
Relay Relay3 = Relay("Relay3", -1);
Relay Relay4 = Relay("Relay4", -1);
Relay Relay5 = Relay("Relay5", -1);
Relay Relay6 = Relay("Relay6", -1);
Relay Relay7 = Relay("Relay7", -1);
Relay Relay8 = Relay("Relay8", -1);

class CustomHtmlFormatProvider : public iotwebconf::OptionalGroupHtmlFormatProvider {
protected:
    virtual String getFormEnd() {
        String s_ = OptionalGroupHtmlFormatProvider::getFormEnd();
        s_ += F("</br>Return to <a href='/'>home page</a>.");
        return s_;
    }
};
CustomHtmlFormatProvider customHtmlFormatProvider;

void webinit() {
    iotWebConf.setStatusPin(STATUS_PIN, ON_LEVEL);
    iotWebConf.setConfigPin(CONFIG_PIN);
    iotWebConf.setHtmlFormatProvider(&customHtmlFormatProvider);

    iotWebConf.addSystemParameter(&APModeOfflineParam);

    Relay1.setNext(&Relay2);
    Relay2.setNext(&Relay3);
    Relay3.setNext(&Relay4);
	Relay4.setNext(&Relay5);
	Relay5.setNext(&Relay6);
	Relay6.setNext(&Relay7);
	Relay7.setNext(&Relay8);

    iotWebConf.addParameterGroup(&Config);

    iotWebConf.addParameterGroup(&Relay1);
    iotWebConf.addParameterGroup(&Relay2);
    iotWebConf.addParameterGroup(&Relay3);
    iotWebConf.addParameterGroup(&Relay4);
	iotWebConf.addParameterGroup(&Relay5);
	iotWebConf.addParameterGroup(&Relay6);
	iotWebConf.addParameterGroup(&Relay7);
	iotWebConf.addParameterGroup(&Relay8);  

    iotWebConf.setupUpdateServer(
        [](const char* updatePath) { AsyncUpdater.setup(&server, updatePath); },
        [](const char* userName, char* password) { AsyncUpdater.updateCredentials(userName, password); });

    iotWebConf.setConfigSavedCallback(&configSaved);
    iotWebConf.setWifiConnectionCallback(&wifiConnected);
    iotWebConf.setFormValidator(&formValidator);

    iotWebConf.getApTimeoutParameter()->visible = true;

    iotWebConf.init();
    convertParams();

    // -- Set up required URL handlers on the web server.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { handleRoot(request); });
    server.on("/config", HTTP_ANY, [](AsyncWebServerRequest* request) {
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleConfig(&asyncWebRequestWrapper);
        }
    );
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse_P(200, "image/x-icon", favicon_ico, sizeof(favicon_ico));
        request->send(response);
        }
    );
    server.onNotFound([](AsyncWebServerRequest* request) {
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleNotFound(&asyncWebRequestWrapper);
        }
    );
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest* request) { 
        handleData(request); 
        }
    );
    server.onNotFound([](AsyncWebServerRequest* request) {
        AsyncWebRequestWrapper asyncWebRequestWrapper(request);
        iotWebConf.handleNotFound(&asyncWebRequestWrapper);
        }
    );

    if (APModeOfflineTime > 0) {
        APModeTimer.start(APModeOfflineTime * 60 * 1000);
    }

    Serial.println("Ready.");
}

void webLoop() {
    // -- doLoop should be called as frequently as possible.
    iotWebConf.doLoop();
    ArduinoOTA.handle();

    if (ChangedConfiguration) {
        Serial.println(F("Parameters are changed,save them"));

        Config.SetSource(N2KSource);

        iotWebConf.saveConfig();
        ChangedConfiguration = false;
    }

    if (APModeTimer.done()) {
        Serial.println(F("AP mode offline time reached"));
        iotWebConf.goOffLine();
        APModeTimer.stop();
    }

    if (AsyncUpdater.isFinished()) {
        Serial.println(F("Firmware update finished"));
        delay(1000);
        ESP.restart();
    }
}

void wifiConnected() {
    ArduinoOTA.begin();
}

bool isValidPin(int pin) {
    for (int validPin_ : validPins) {
        if (pin == validPin_) {
            return true;
        }
    }
    return false;
}

bool formValidator(iotwebconf::WebRequestWrapper* webRequestWrapper) {

    bool valid_ = true;

    Relay* relay_ = &Relay1;
    while (relay_ != nullptr) {
        if (relay_->isActive()) {

            uint8_t pin_ = atoi(webRequestWrapper->arg(relay_->gpioParam.getId()).c_str());
            
            Serial.println("Validating pin " + String(pin_));

            if (!isValidPin(pin_)) {
                relay_->gpioParam.errorMessage = "Invalid pin number. See esp32 documentation";
                valid_ = false;
            }
        }
        relay_ = (Relay*)relay_->getNext();
    }

    return valid_;
}

void convertParams() {

    N2KSource = Config.Source();
    BinaryDeviceInstance = Config.DeviceID() - 1;
    RelayAddress = Config.RelayAddress();

    APModeOfflineTime = atoi(APModeOfflineValue);
}

void configSaved() {
    convertParams();
    ChangedConfiguration = true;
}

void handleData(AsyncWebServerRequest* request) {

	String json_ = "{";
	json_ += "\"rssi\":" + String(WiFi.RSSI()) ;
	Relay* relay_ = &Relay1;
	uint8_t i_ = 1;
	while (relay_ != nullptr) {
		if (relay_->isActive()) {
			json_ += ",\"relay" + String(i_) + "\":\"" + (relay_->isEnable() ? "On" : "Off") + "\"";
		}
		relay_ = (Relay*)relay_->getNext();
		i_++;
	}
	json_ += "}";
	request->send(200, "application/json", json_);
}

class MyHtmlRootFormatProvider : public HtmlRootFormatProvider {
public:
    String getHtmlTableRowClass(String name, String htmlclass, String id) {
        String s_ = F("<tr><td align=\"left\">{n}</td><td align=\"left\"><span id=\"{id}\" class=\"{c}\"></span></td></tr>\n");
        s_.replace("{n}", name);
        s_.replace("{c}", htmlclass);
        s_.replace("{id}", id);
        return s_;
    }

protected:
    virtual String getStyleInner() {
        String s_ = HtmlRootFormatProvider::getStyleInner();
        s_ += F(".led {display: inline-block; width: 15px; height: 15px; border-radius: 50%; margin-right: 5px; }\n");
        s_ += F(".led.off {background-color: grey;}\n");
        //s_ += F(".led.on {background-color: green;}\n");
        s_ += F(".led.on {background-color: #00FF00;}\n"); // Leuchtendes Grün
        s_ += F(".led.delayedoff {background-color: orange;}\n");
        return s_;
    }

    virtual String getScriptInner() {
        String s_ = HtmlRootFormatProvider::getScriptInner();

        s_.replace("{millisecond}", "1000");
 
        s_ += F("function updateLED(element, status) {\n");
        s_ += F("   if (element) {\n");
        s_ += F("       element.classList.remove('on', 'off', 'delayedoff');\n");
        s_ += F("       element.classList.add(status);\n");
        s_ += F("   }\n");
        s_ += F("}\n");

        s_ += F("function updateData(jsonData) {\n");
        s_ += F("   document.getElementById('RSSIValue').innerHTML = jsonData.rssi + \"dBm\" \n");

        Relay* relay_ = &Relay1;
        uint8_t i_ = 1;
        while (relay_ != nullptr) {
            if (relay_->isActive()) {
                s_ += "   updateLED(document.getElementById('relay" + String(i_) + "'), jsonData.relay" + String(i_) + ".toLowerCase());\n";
            }
            relay_ = (Relay*)relay_->getNext();
            i_++;
        }

        s_ += F("}\n");

        return s_;
    }
};

void handleRoot(AsyncWebServerRequest* request) {
    AsyncWebRequestWrapper asyncWebRequestWrapper(request);
    if (iotWebConf.handleCaptivePortal(&asyncWebRequestWrapper)) {
        return;
    }

    std::string content_;
    MyHtmlRootFormatProvider fp_;

    content_ += fp_.getHtmlHead(iotWebConf.getThingName()).c_str();
    content_ += fp_.getHtmlStyle().c_str();
    content_ += fp_.getHtmlHeadEnd().c_str();
    content_ += fp_.getHtmlScript().c_str();

    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRow().c_str();
    content_ += fp_.getHtmlTableCol().c_str();

    content_ += String(F("<fieldset align=left style=\"border: 1px solid\">\n")).c_str();
    content_ += String(F("<table border=\"0\" align=\"center\" width=\"100%\">\n")).c_str();
    content_ += String(F("<tr><td align=\"left\"> </td></td><td align=\"right\"><span id=\"RSSIValue\">no data</span></td></tr>\n")).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.getHtmlFieldset("Relays").c_str();
    content_ += fp_.getHtmlTable().c_str();
    Relay* relay_ = &Relay1;
    uint8_t i_ = 1;
    while (relay_ != nullptr) {
		if (relay_->isActive()) {
			content_ += fp_.getHtmlTableRowClass(String(relay_->label) + ":", "led off", "relay" + String(i_)).c_str();
		}

        relay_ = (Relay*)relay_->getNext();
        i_++;
    }
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.getHtmlFieldset("Network").c_str();
    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRowText("MAC Address:", WiFi.macAddress()).c_str();
    content_ += fp_.getHtmlTableRowText("IP Address:", WiFi.localIP().toString().c_str()).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlFieldsetEnd().c_str();

    content_ += fp_.addNewLine(2).c_str();

    content_ += fp_.getHtmlTable().c_str();
    content_ += fp_.getHtmlTableRowText("Go to <a href = 'config'>configure page</a> to change configuration.").c_str();
    //content_ += fp_.getHtmlTableRowText("Go to <a href = 'webserial'>sensor monitoring</a> page.").c_str();
    content_ += fp_.getHtmlTableRowText(fp_.getHtmlVersion(Version)).c_str();
    content_ += fp_.getHtmlTableEnd().c_str();

    content_ += fp_.getHtmlTableColEnd().c_str();
    content_ += fp_.getHtmlTableRowEnd().c_str();
    content_ += fp_.getHtmlTableEnd().c_str();
    content_ += fp_.getHtmlEnd().c_str();

    AsyncWebServerResponse* response = request->beginChunkedResponse("text/html", [content_](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {

        std::string chunk_ = "";
        size_t len_ = min(content_.length() - index, maxLen);
        if (len_ > 0) {
            chunk_ = content_.substr(index, len_ - 1);
            chunk_.copy((char*)buffer, chunk_.length());
        }
        if (index + len_ <= content_.length())
            return chunk_.length();
        else
            return 0;

        });
    response->addHeader("Server", "ESP Async Web Server");
    request->send(response);
}