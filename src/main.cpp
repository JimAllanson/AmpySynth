#include "Arduino.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "AmpySynth.h"

#include "FS.h"
#include "SD.h"
#include "SPIFFS.h"
#include "HTTPClient.h"
#include "SPI.h"
#include "Wire.h"

#include "I2Cdev.h"
#include "TCA6424A.h"

#include "FastLED.h" 

#include <ESP32Encoder.h>

#include <IotWebConf.h>
#include <IotWebConfESP32HTTPUpdateServer.h>

#include <MozziGuts.h>
#include <Oscil.h> 
#include <tables/sin2048_int8.h> 
#include <tables/cos2048_int8.h>

DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;
IotWebConf iotWebConf("AmpySynth", &dnsServer, &server, "amplience", CONFIG_VERSION);

TCA6424A tca;

struct CRGB leds[NUM_LEDS];

ESP32Encoder enc;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aVibrato(COS2048_DATA);
byte gain = 0;
byte vibrato = 0;


void handleRoot();
void audioLoop( void * pvParameters );
void keyboardLoop( void * pvParameters );

int r = 0;
int g = 0;
int b = 0;

int encPos = 0;

bool ledMode = false;

unsigned long lastEncoderButtonDebounceTime = 0;
unsigned long debounceDelay = 500; 

TaskHandle_t audioTask;
TaskHandle_t keyboardTask;

uint8_t keyBuffer[3] = {0,0,0};

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(KEY_B0, INPUT);
  pinMode(KEY_C3, INPUT);

  //Init encoder
  ESP32Encoder::useInternalWeakPullResistors=UP;
  enc.attachSingleEdge(ENC_A, ENC_B);

  //Set up web config / update service
  iotWebConf.setupUpdateServer(
    [](const char* updatePath) { httpUpdater.setup(&server, updatePath); },
    [](const char* userName, char* password) { httpUpdater.updateCredentials(userName, password); });
  iotWebConf.init();
  iotWebConf.setConfigPin(WEBCONF_RESET_PIN);
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });

  //Init addressable LEDs
  LEDS.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 250);

  //Init TCA6424 IO expander
  Serial.println("Initializing I2C devices...");
  Wire.begin(SDA, SCL);
  tca.initialize();
  Serial.println("Testing device connections...");
  if (!tca.testConnection()) {
    Serial.println("TCA6424A Connection Failed");
  }
  Serial.println("Initialised TCA6424A as input");

  //Init mozzi (audio processing library)
  startMozzi(64);
  aVibrato.setFreq(15.f);

  //Start a background task for audio on core 0
  xTaskCreatePinnedToCore(audioLoop, "AudioTask", 10000, NULL, 1, &audioTask, 0);
  //Start a background task for polling IO expander on core 0
  xTaskCreatePinnedToCore(keyboardLoop, "KeyboardTask", 10000, NULL, 1, &keyboardTask, 0);
}


void updateControl(){
  int maxGain = map(analogRead(POT_1), 0, 4096, 128, 0);
  vibrato = map(analogRead(POT_2), 0, 4096, 255, 0);
  float lfoFreq = map(analogRead(POT_3), 0, 4096, 255, 0) / 10.0;
  aVibrato.setFreq(lfoFreq);

  int newGain = 0;

  if(digitalRead(KEY_B0) == HIGH) {
    newGain = maxGain;
    aSin.setFreq(notes[0]);
  }
  for(int b = 0; b < 3; b++) {
    for(int i = 0; i < 8; i++) {
      if(keyBuffer[b] & (1 << i)) {
        newGain = maxGain;
        aSin.setFreq(notes[i + (b * 8) + 1]);
      }
    }
  }
  if(digitalRead(KEY_C3) == HIGH) {
    newGain = maxGain;
    aSin.setFreq(notes[25]);
  }
  
  gain = newGain;
}


AudioOutput_t updateAudio(){
  Q15n16 vib = (Q15n16) vibrato * aVibrato.next();
  return MonoOutput::from16Bit(aSin.phMod(vib) * gain); // 8 bits waveform * 8 bits gain makes 16 bits
}

void loop() {
  iotWebConf.doLoop();

  //Handle encoder direction
  int newEncPos = enc.getCount();
  if(newEncPos > encPos) {
    encPos--;
  } else if (newEncPos < encPos) {
    encPos++;
  }
  encPos = encPos % NUM_LEDS;
  if(encPos < 0) {
    encPos += NUM_LEDS;
  }
  enc.setCount(encPos);

  //Handle encoder button
  if(digitalRead(ENC_BTN) == LOW && (millis() - lastEncoderButtonDebounceTime) > debounceDelay) {
    lastEncoderButtonDebounceTime = millis();
    ledMode = !ledMode;
  }

  if(ledMode) {
    FastLED.clear();
    leds[encPos].r = map(analogRead(POT_1), 0, 4096, 255, 0);
    leds[encPos].g = map(analogRead(POT_2), 0, 4096, 255, 0);
    leds[encPos].b = map(analogRead(POT_3), 0, 4096, 255, 0);
  } else {
    int8_t thisHue = beat8(20,255);
    fill_rainbow(leds, NUM_LEDS, thisHue, 16);
  }

  FastLED.show();
}

void audioLoop( void * pvParameters ){
  for(;;){
    audioHook();
    //Feed the watchdog. Woof woof!
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed=1;
    TIMERG0.wdt_wprotect=0;
  }
}

void keyboardLoop( void * pvParameters ){
  for(;;){
    keyBuffer[0] = tca.readBank(0);
    keyBuffer[1] = tca.readBank(1);
    keyBuffer[2] = tca.readBank(2);
    //This loop doesn't need to run too fast
    delay(10);
  }
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