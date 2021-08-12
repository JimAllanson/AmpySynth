
#include "Program.h" 
#include "AmpySynth.h"
#include "FastLED.h" 

class RGBAmpy : public Program {
    public:
        void setup();
        void loop();
        void encoderPress();
};
