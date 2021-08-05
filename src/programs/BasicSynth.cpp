#include "BasicSynth.h" 

#include <MozziGuts.h>
#include <Oscil.h> 
#include <ADSR.h>
#include <tables/sin2048_int8.h> 
#include <tables/cos2048_int8.h>
#include <mozzi_midi.h>

int encPos2 = 0;

byte volume = 10;
byte vibrato = 0;
byte lfoFreq = 0;
byte attack = 50;
byte decay = 200;
int sustain = 10000;
byte release = 200;

Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin(SIN2048_DATA);
Oscil<COS2048_NUM_CELLS, AUDIO_RATE> aVibrato(COS2048_DATA);

ADSR <CONTROL_RATE, AUDIO_RATE> envelope;


void BasicSynth::setup() {
  envelope.setADLevels(255, 64);
}

void BasicSynth::update() {
    if(digitalRead(ENC_BTN) == LOW) {
        exit();
    }


  int pot1 = analogRead(POT_1);
  int pot2 = analogRead(POT_2);
  int pot3 = analogRead(POT_3);

  if(encPos2 == 0) {
    volume = map(pot1, 0, 4096, 128, 0);
    vibrato = map(pot2, 0, 4096, 255, 0);
    lfoFreq = map(pot3, 0, 4096, 255, 0) / 10.0;
  } else if(encPos2 == 1) {
    attack = map(pot1, 0, 4096, 255, 0);
    decay = map(pot2, 0, 4096, 255, 0);
    sustain = map(pot3, 0, 4096, 10000, 0);
  } else {
    release = map(pot1, 0, 4096, 255, 0);
  }
  envelope.setTimes(attack, decay, sustain, release);
  
  aVibrato.setFreq(lfoFreq);

  envelope.noteOff();
  for(int i = 0; i < 32; i++) {
    if(*keys & (1 << i)) {
      envelope.noteOn();
      aSin.setFreq(mtof(59 + i));
    }
  }

  envelope.update();
}

AudioOutput_t BasicSynth::audio() {
  Q15n16 vib = (Q15n16) vibrato * aVibrato.next();

  int multByEnv = (int) (envelope.next() * aSin.phMod(vib))>>8;
  return MonoOutput::from16Bit(multByEnv * volume); // 8 bits waveform * 8 bits gain makes 16 bits
}