#ifndef PROGRAM_H
#define PROGRAM_H

#include <TFT_eSPI.h> 
#include "FastLED.h" 
#include "Peripherals.h" 
#include <MozziGuts.h>

class Program {
    protected:
      void (* onExit)(int nextProgram);
    public:
      template<typename T>
      static Program* create() {
          return new T();
      }
      virtual void setup() {
      }
      virtual void loop(){
      }
      virtual void update() {
      }
      virtual void encoderUp() {
      }
      virtual void encoderDown() {
      }
      virtual void encoderPress() {
      }
      virtual void encoderRelease() {
      }
      void setEnv(void (* _onExit)(int nextProgram)) {
        onExit = _onExit;
        tft.fillScreen(TFT_BLACK);
        FastLED.clear();
        FastLED.show();
        setup();
      };
      void exit(int nextProgram = 0) {
          onExit(nextProgram);
      }
      virtual AudioOutput_t audio() {
          return MonoOutput::from16Bit(0);
      };
};

struct ProgramEntry { 
  char* name;
  Program* (* programFactory)();
};

extern ProgramEntry programs[20];

#endif