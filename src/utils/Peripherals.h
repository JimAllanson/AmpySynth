#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "AmpySynth.h"
#include "TFT_eSPI.h"

extern struct CRGB leds[NUM_LEDS];
extern uint32_t keys;
extern TFT_eSPI tft;

#endif