#include "Arduino.h"

#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"
#include "HTTPClient.h"
#include "SPI.h"
#include "Wire.h"

#include "I2Cdev.h"
#include "TCA6424A.h"

#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorRTTTL.h"

#include "FastLED.h" 

#include <IotWebConf.h>
#include <IotWebConfESP32HTTPUpdateServer.h>

#define CONFIG_VERSION "2"

#define SDA 17
#define SCL 5

#define WEBCONF_RESET_PIN 32


#define LED_DATA_PIN 2
#define COLOR_ORDER GRB
#define LED_TYPE WS2812
#define NUM_LEDS 16


const char tune[] PROGMEM = "JurassicPark:d=32,o=6,b=28:p,b5,a#5,8b5,16p,b5,a#5,8b5,16p,b5,a#5,16b.5,c#,16c#.,e,8e,16p,d#,b5,16c#.,a#5,16f#5,d#,b5,8c#,16p,f#,b5,16e.,d#,16d#.,c#,8c#.,1p";

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;
IotWebConf iotWebConf("AmpySynth", &dnsServer, &server, "amplience", CONFIG_VERSION);

AudioOutputI2S *out;
AudioGeneratorRTTTL *rtttl;
AudioFileSourcePROGMEM *file;

TCA6424A tca;

struct CRGB leds[NUM_LEDS];

void handleRoot();

void setup() {
  Serial.begin(115200);
  delay(1000);

  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
  iotWebConf.init();
  iotWebConf.setConfigPin(WEBCONF_RESET_PIN);
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  LEDS.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 250);


  Serial.println("Initializing I2C devices...");
  Wire.begin(SDA, SCL);
  tca.initialize();
  Serial.println("Testing device connections...");
  if (!tca.testConnection()) {
    Serial.println("TCA6424A Connection Failed");
  }
  tca.setBankDirection(0, TCA6424A_INPUT);
  tca.setBankDirection(1, TCA6424A_INPUT);
  tca.setBankDirection(2, TCA6424A_INPUT);
  Serial.println("Initialised TCA6424A as input");

  out = new AudioOutputI2S();
  out -> SetGain(0.05);
  out -> SetPinout(26,25,22);

  file = new AudioFileSourcePROGMEM( tune, strlen_P(tune) );
  rtttl = new AudioGeneratorRTTTL();
  rtttl->begin(file, out);
}

void loop() {
  iotWebConf.doLoop();


  if (rtttl->isRunning()) {
    if (!rtttl->loop()) {
      rtttl->stop();
    }
  }

  //bool val = tca.readPin(TCA6424A_P00);

  uint8_t thisHue = beat8(20,255);
  fill_rainbow(leds, NUM_LEDS, thisHue, 16);
  FastLED.show();
}

void handleRoot()
{
  if (iotWebConf.handleCaptivePortal()) {
    return;
  }
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
  s += "<title>AmpySynth!</title></head><body>";
  s += "Go to <a href='config'>configure page</a> to change values.";
  s += "</body></html>\n";

  server.send(200, "text/html", s);
}