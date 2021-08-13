
#include "AmpySynth.h"
#include "utils/Program.h" 
#include "utils/Peripherals.h" 
#include <MozziGuts.h>
#include <Oscil.h> 
#include <ADSR.h>
#include <tables/sin2048_int8.h> 
#include <tables/cos2048_int8.h>
#include <mozzi_midi.h>

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
