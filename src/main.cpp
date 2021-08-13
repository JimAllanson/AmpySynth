#include "Arduino.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

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
#include <TFT_eSPI.h> 

#include "AmpySynth.h"
#include "utils/Peripherals.h"
#include "utils/AmpySynthNetwork.h"
#include "utils/Program.h"
#include "programs/MainMenu.h"
#include "programs/RGBAmpy.h"
#include "programs/BasicSynth.h"

struct CRGB leds[NUM_LEDS];
uint32_t keys;
TFT_eSPI tft = TFT_eSPI();

TCA6424A tca;
ESP32Encoder encoder;
AmpySynthNetwork network(WEBCONF_RESET_PIN);


void audioLoop( void * pvParameters );
void keyboardLoop( void * pvParameters );
void encoderLoop( void * pvParameters );
TaskHandle_t audioTask;
TaskHandle_t keyboardTask;
TaskHandle_t encoderTask;

void setProgram(int programNumber);
Program *program = MainMenu::getInstance();
ProgramEntry programs[] {
    {"Main Menu", MainMenu::getInstance},
    {"RGBAmpy", Program::create<RGBAmpy>},
    {"Basic Synth", Program::create<BasicSynth>},
};

void setup() {
  Serial.begin(115200);

  network.init();

  ESP32Encoder::useInternalWeakPullResistors=UP;
  encoder.attachSingleEdge(ENC_A, ENC_B);

  pinMode(KEY_B0, INPUT);
  pinMode(KEY_C3, INPUT);

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

  //Set up LCD
  tft.rotation = 2;
  tft.begin();
  tft.setTextSize(2);

  setProgram(0);

  Serial.println("Starting async tasks");
  //Start a background task for audio on core 0
  xTaskCreatePinnedToCore(audioLoop, "AudioTask", 10000, NULL, 1, &audioTask, 0);
  //Start a background task for polling IO expander on core 1
  xTaskCreatePinnedToCore(keyboardLoop, "KeyboardTask", 10000, NULL, 1, &keyboardTask, 0);
  //Start a background task for updating encoder state on core 1
  xTaskCreatePinnedToCore(encoderLoop, "EncoderTask", 10000, NULL, 1, &encoderTask,1);
  
  Serial.println("Finish Setup");
}

void setProgram(int next) {
  if (program != MainMenu::getInstance()) {
      delete program;
  }
  if(next > 0) {
      program = programs[next].programFactory();
  } else {
    program = MainMenu::getInstance();
  }
  program->setEnv(setProgram);
}

//Runs 64 times a second, put IO tasks etc here
void updateControl(){
  program->update();
}

//Called very frequently, feed the audio bytes
AudioOutput_t updateAudio(){
  return program->audio();
}

//Runs as fast as possible, but beware of eating CPU cycles
void loop() {
  network.loop();
  program->loop();
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
    uint32_t newKeys;
    newKeys = digitalRead(KEY_C3);
    newKeys = newKeys << 8;
    newKeys = newKeys | tca.readBank(2);
    newKeys = newKeys << 8;
    newKeys = newKeys | tca.readBank(1);
    newKeys = newKeys << 8;
    newKeys = newKeys | tca.readBank(0);
    newKeys = newKeys << 1;
    newKeys = newKeys | digitalRead(KEY_B0);
    keys = newKeys;

    //This loop doesn't need to run too fast
    delay(10);
  }
}

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;
int lastEncoderValue = 0;
int encoderButtonState = 1;
void encoderLoop( void * pvParameters ){
  for(;;){
    int newEncPos = encoder.getCount();
    if(newEncPos > lastEncoderValue) {
      program->encoderDown();
    } else if (newEncPos < lastEncoderValue) {
      program->encoderUp();
    }
    lastEncoderValue = newEncPos;

    
    if ((millis() - lastDebounceTime) > debounceDelay) {
      int buttonState = digitalRead(ENC_BTN);
      if(encoderButtonState == LOW && buttonState == HIGH) {
        program->encoderRelease();
      }
      if(encoderButtonState == HIGH && buttonState == LOW) {
        program->encoderPress();
      }
      encoderButtonState = buttonState;
    }


    //This loop doesn't need to run too fast
    delay(10);
  }
}