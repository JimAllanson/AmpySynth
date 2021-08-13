
#include "AmpySynth.h"
#include "FastLED.h" 
#include "utils/Peripherals.h" 
#include "utils/Program.h" 

class RGBAmpy : public Program {
    public:
        void setup();
        void loop();
        void encoderPress();
};
