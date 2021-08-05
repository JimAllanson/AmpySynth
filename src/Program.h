#ifndef PROGRAM_H
#define PROGRAM_H

#include <TFT_eSPI.h> 
#include "FastLED.h" 
#include <MozziGuts.h>

class Program {
    protected:
      CRGB *leds;    
      TFT_eSPI *tft;
      uint32_t *keys;
      void (* onExit)();
      void exit() {
          onExit();
      }
    public:
      virtual void setup() {
      }
      virtual void loop(){
      }
      virtual void update() {
      }
      void setEnv(CRGB *_leds, TFT_eSPI *_tft, uint32_t *_keys, void (* _onExit)()) {
        leds = _leds;
        tft = _tft;
        keys = _keys;
        onExit = _onExit;
        setup();
      };
      virtual AudioOutput_t audio() {
          return MonoOutput::from16Bit(0);
      };
      
};

#endif