
#include "Program.h" 
#include "AmpySynth.h"
#include "FastLED.h" 

class BasicSynth : public Program {
    public:
        void setup();
        void update();
        void loop();
        void encoderUp();
        void encoderDown();
        void encoderPress();
        AudioOutput_t audio();
};
