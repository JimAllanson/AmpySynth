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

#include "SPI.h"
#include <TFT_eSPI.h> 



#include <menu.h>
#include <menuIO/TFT_eSPIOut.h>
#include <streamFlow.h>
#include <ClickEncoder.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/chainStream.h>



#include "AmpySynthNetwork.h"

#include "Program.h"
#include "programs/BasicSynth.h"
#include "programs/RGBAmpy.h"

TCA6424A tca;

struct CRGB leds[NUM_LEDS];



ESP32Encoder enc;




void updateEncoderPosition();
void audioLoop( void * pvParameters );
void keyboardLoop( void * pvParameters );
void encoderLoop( void * pvParameters );
void navLoop( void * pvParameters );
result setProgram0();
result setProgram1();
Program* getProgram();

int r = 0;
int g = 0;
int b = 0;

int encPos = 0;

bool ledMode = true;

unsigned long lastEncoderButtonDebounceTime = 0;
unsigned long debounceDelay = 500; 

TaskHandle_t audioTask;
TaskHandle_t keyboardTask;
TaskHandle_t navTask;
TaskHandle_t encoderTask;


uint32_t keys;

TFT_eSPI tft = TFT_eSPI();

AmpySynthNetwork network(WEBCONF_RESET_PIN);



#define GFX_WIDTH 240
#define GFX_HEIGHT 240
#define fontW 10
#define fontH 24

int programNum = -1;
Program *programs[] = {
  new BasicSynth(),
  new RGBAmpy()
};


MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,OP("RGBAmpy",setProgram0,enterEvent)
  ,OP("BasicSynth",setProgram1,enterEvent)
  ,EXIT("<Back")
);

#define Black RGB565(0,0,0)
#define Red	RGB565(255,0,0)
#define Green RGB565(0,255,0)
#define Blue RGB565(0,0,255)
#define Gray RGB565(128,128,128)
#define LighterRed RGB565(255,150,150)
#define LighterGreen RGB565(150,255,150)
#define LighterBlue RGB565(150,150,255)
#define DarkerRed RGB565(150,0,0)
#define DarkerGreen RGB565(0,150,0)
#define DarkerBlue RGB565(0,0,150)
#define Cyan RGB565(0,255,255)
#define Magenta RGB565(255,0,255)
#define Yellow RGB565(255,255,0)
#define White RGB565(255,255,255)
#define AmpDarkBlue RGB565(26,34,45)
#define AmpPink RGB565(AMPLIENCE_PINK_R, AMPLIENCE_PINK_G, AMPLIENCE_PINK_B)
const colorDef<uint16_t> colors[6] MEMMODE={
  {{(uint16_t)Black,(uint16_t)Black}, {(uint16_t)AmpDarkBlue, (uint16_t)AmpPink,  (uint16_t)AmpPink}},//bgColor
  {{(uint16_t)Gray, (uint16_t)Gray},  {(uint16_t)White, (uint16_t)White, (uint16_t)White}},//fgColor
  {{(uint16_t)White,(uint16_t)Black}, {(uint16_t)Yellow,(uint16_t)Yellow,(uint16_t)Red}},//valColor
  {{(uint16_t)White,(uint16_t)Black}, {(uint16_t)White, (uint16_t)Yellow,(uint16_t)Yellow}},//unitColor
  {{(uint16_t)White,(uint16_t)Gray},  {(uint16_t)AmpDarkBlue, (uint16_t)AmpPink,  (uint16_t)White}},//cursorColor
  {{(uint16_t)White,(uint16_t)Yellow},{(uint16_t)AmpDarkBlue,  (uint16_t)AmpPink,   (uint16_t)White}},//titleColor
};
#define MAX_DEPTH 4
const panel panels[] MEMMODE = {{0, 0, GFX_WIDTH / fontW, GFX_HEIGHT / fontH}};
navNode* nodes[sizeof(panels) / sizeof(panel)];
panelsList pList(panels, nodes, 1);
idx_t eSpiTops[MAX_DEPTH]={0};
TFT_eSPIOut eSpiOut(tft,colors,eSpiTops,pList,fontW,fontH+1);
menuOut* constMEM outputs[] MEMMODE={&eSpiOut};//list of output devices
outputsList out(outputs,sizeof(outputs)/sizeof(menuOut*));//outputs list controller

ClickEncoder clickEncoder = ClickEncoder(ENC_B, ENC_A, ENC_BTN, 4);
ClickEncoderStream encStream(clickEncoder, 1);
MENU_INPUTS(in, &encStream);

NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);


void setup() {
  Serial.begin(115200);

  network.init();

  clickEncoder.setAccelerationEnabled(true);
  clickEncoder.setDoubleClickEnabled(false);


  pinMode(KEY_B0, INPUT);
  pinMode(KEY_C3, INPUT);

  //Init encoder
  //ESP32Encoder::useInternalWeakPullResistors=UP;
  //enc.attachSingleEdge(ENC_A, ENC_B);

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
  



  //Set up LCD
  tft.rotation = 2;
  tft.begin();

  Serial.println("Starting async tasks");


  //Start a background task for audio on core 0
  xTaskCreatePinnedToCore(audioLoop, "AudioTask", 10000, NULL, 1, &audioTask, 0);
  //Start a background task for polling IO expander on core 1
  xTaskCreatePinnedToCore(keyboardLoop, "KeyboardTask", 10000, NULL, 1, &keyboardTask, 0);
  xTaskCreatePinnedToCore(encoderLoop, "EncoderTask", 10000, NULL, 1, &encoderTask,1);
  xTaskCreatePinnedToCore(navLoop, "NavTask", 10000, NULL, 1, &navTask,1);
  
  Serial.println("init prog");
  Program *program = programs[0];
  program->setEnv(leds, &tft, &keys, [](){
    programNum = -1;
  });

  Serial.println("Finish Setup");
}

result setProgram0() {
  programNum = 0;
  return proceed;
}

result setProgram1() {
  programNum = 1;
  return proceed;
}

void updateControl(){

    if(programNum >= 0) {
      getProgram()->update();
    } 
  
}


AudioOutput_t updateAudio(){
 if(programNum >= 0) {
    return getProgram()->audio();
  } else {
    return MonoOutput::from16Bit(0);
  }
}


Program* getProgram() {
  return programs[programNum];
}

void loop() {
  network.loop();

  if(programNum >= 0) {
    getProgram()->loop();
  }

  
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
    keys = digitalRead(KEY_C3);
    keys = keys << 8;
    keys = keys | tca.readBank(2);
    keys = keys << 8;
    keys = keys | tca.readBank(1);
    keys = keys << 8;
    keys = keys | tca.readBank(0);
    keys = keys << 1;
    keys = keys | digitalRead(KEY_B0);

    //This loop doesn't need to run too fast
    delay(10);
  }
}

void encoderLoop( void * pvParameters ){
  for(;;){
    clickEncoder.service();
    //This loop doesn't need to run too fast
    delay(1);
  }
}

void navLoop( void * pvParameters ){
  for(;;){
    nav.poll();
    //This loop doesn't need to run too fast
    delay(1);
  }
}

