#include "SPIFFS.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebSocketsServer.h>
#include <IotWebConf.h>
#include <IotWebConfESP32HTTPUpdateServer.h>
#include <ArduinoOTA.h>

#define CONFIG_VERSION "2"
#define NAME "AmpySynth"
#define DEFAULT_PASSWORD "amplience"

class AmpySynthNetwork
{
  private:
    int _resetPin;
    void _wifiConnected();
    void _handleRoot();

  public:
    AmpySynthNetwork(int resetPin): web(80), ws(81), iotWebConf(NAME, &dns, &web, DEFAULT_PASSWORD, CONFIG_VERSION) {
      _resetPin = resetPin;
    }
    void init();
    void loop();

    DNSServer dns;
    WebServer web;
    WebSocketsServer ws;
    HTTPUpdateServer updateServer;
    IotWebConf iotWebConf;

};