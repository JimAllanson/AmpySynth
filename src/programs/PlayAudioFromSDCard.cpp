#include "PlayAudioFromSDCard.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputMozzi.h"
#include <MozziGuts.h>

uint16_t sample;
AudioFileSourceSD *source = NULL;
AudioGeneratorMP3 *mp3;
AudioOutputMozzi mozziOut(&sample);

void PlayAudioFromSDCard::setup() {
}

void PlayAudioFromSDCard::update() {
}

AudioOutput_t PlayAudioFromSDCard::audio() {
    return MonoOutput::from16Bit(&sample);
    
}