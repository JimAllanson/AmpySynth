
#include "Program.h" 
#include "AmpySynth.h"

class PlayAudioFromSDCard : public Program {
    public:
        void setup();
        void update();
        AudioOutput_t audio();
};
