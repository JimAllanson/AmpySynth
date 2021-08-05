
#include "Program.h" 
#include "AmpySynth.h"
#include "FastLED.h" 

class BasicSynth : public Program {
    public:
        void setup();
        void update();
        AudioOutput_t audio();
};
