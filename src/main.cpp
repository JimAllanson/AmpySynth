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


#include <MozziGuts.h>
#include <Oscil.h> 
#include <ADSR.h>
#include <tables/sin2048_int8.h> 
#include <tables/cos2048_int8.h>
#include <mozzi_midi.h>

#include "SPI.h"
#include <TFT_eSPI.h> 

#include "AmpySynthNetwork.h"

TCA6424A tca;

struct CRGB leds[NUM_LEDS];

ESP32Encoder enc;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aVibrato(COS2048_DATA);

#define CONTROL_RATE 128
ADSR <CONTROL_RATE, AUDIO_RATE> envelope;

byte volume = 0;
byte vibrato = 0;
byte lfoFreq = 0;
byte attack = 50;
byte decay = 200;
int sustain = 10000;
byte release = 200;

void updateEncoderPosition();
void audioLoop( void * pvParameters );
void keyboardLoop( void * pvParameters );


int r = 0;
int g = 0;
int b = 0;

int encPos = 0;

bool ledMode = true;

unsigned long lastEncoderButtonDebounceTime = 0;
unsigned long debounceDelay = 500; 

TaskHandle_t audioTask;
TaskHandle_t keyboardTask;
TaskHandle_t websocketTask;

uint8_t keyBuffer[3] = {0,0,0};

TFT_eSPI tft = TFT_eSPI();

AmpySynthNetwork network(WEBCONF_RESET_PIN);


void setup() {
  Serial.begin(115200);

  network.init();


  pinMode(KEY_B0, INPUT);
  pinMode(KEY_C3, INPUT);

  //Init encoder
  ESP32Encoder::useInternalWeakPullResistors=UP;
  enc.attachSingleEdge(ENC_A, ENC_B);

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
  startMozzi(CONTROL_RATE);
  //aVibrato.setFreq(15.f);
  
  envelope.setADLevels(255, 64);



  //Set up LCD
  tft.rotation = 2;
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 1);
  tft.setTextColor(TFT_RED, TFT_BLACK);  
  tft.setTextSize(4);
  tft.print("Ampy");
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  
  tft.println("Synth!");
  tft.drawXBitmap(0, 40, AMPY_LOGO, LOGO_WIDTH, LOGO_HEIGHT, TFT_MAGENTA, TFT_BLACK);


  //Start a background task for audio on core 0
  xTaskCreatePinnedToCore(audioLoop, "AudioTask", 10000, NULL, 1, &audioTask, 0);
  //Start a background task for polling IO expander on core 1
  xTaskCreatePinnedToCore(keyboardLoop, "KeyboardTask", 10000, NULL, 1, &keyboardTask, 0);
  //Start a background task for polling websockets on core 1
 // xTaskCreatePinnedToCore(websocketLoop, "WebsocketTask", 10000, NULL, 1, &websocketTask, 1);



}


void updateControl(){
   //Handle encoder direction
  updateEncoderPosition();

  //Handle encoder button
  if(digitalRead(ENC_BTN) == LOW && (millis() - lastEncoderButtonDebounceTime) > debounceDelay) {
    lastEncoderButtonDebounceTime = millis();
    ledMode = !ledMode;
  }

  int pot1 = analogRead(POT_1);
  int pot2 = analogRead(POT_1);
  int pot3 = analogRead(POT_1);

  if(encPos == 0) {
    volume = map(pot1, 0, 4096, 128, 0);
    vibrato = map(pot2, 0, 4096, 255, 0);
    lfoFreq = map(pot3, 0, 4096, 255, 0) / 10.0;
  } else if(encPos == 1) {
    attack = map(pot1, 0, 4096, 255, 0);
    decay = map(pot2, 0, 4096, 255, 0);
    sustain = map(pot3, 0, 4096, 10000, 0);
  } else {
    release = map(pot1, 0, 4096, 255, 0);
  }
  envelope.setTimes(attack, decay, sustain, release);
  


  aVibrato.setFreq(lfoFreq);

  envelope.noteOff();
  if(digitalRead(KEY_B0) == HIGH) {
    envelope.noteOn();
    aSin.setFreq(mtof(59));
  }
  for(int b = 0; b < 3; b++) {
    for(int i = 0; i < 8; i++) {
      if(keyBuffer[b] & (1 << i)) {
        envelope.noteOn();
        aSin.setFreq(mtof(60 + i + (b * 8)));
      }
    }
  }
  if(digitalRead(KEY_C3) == HIGH) {
    envelope.noteOn();
    aSin.setFreq(mtof(84));
  }

  envelope.update();
}


AudioOutput_t updateAudio(){
  
  Q15n16 vib = (Q15n16) vibrato * aVibrato.next();

  int multByEnv = (int) (envelope.next() * aSin.phMod(vib))>>8;
  return MonoOutput::from16Bit(multByEnv * volume); // 8 bits waveform * 8 bits gain makes 16 bits
}


void loop() {
  network.loop();

 

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

void updateEncoderPosition() {
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

  char msg[255];
  snprintf_P(msg, sizeof(msg), PSTR("encoder:%i"), encPos);
  network.ws.broadcastTXT(msg);
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


