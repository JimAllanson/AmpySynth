#include "Arduino.h"

#define LED_DATA_PIN 2
#define KEY_B0 23
#define KEY_C3 34
#ifdef AMPYSYNTH_V1_1
  #pragma message "Building for AmpySynth Revision 1.1"
  #define LED_DATA_PIN 23
  #define KEY_B0 2
#endif

#define CONFIG_VERSION "2"

#define SDA 17
#define SCL 5

#define POT_1 36
#define POT_2 39
#define POT_3 35

#define ENC_A 21
#define ENC_B 19
#define ENC_BTN 4

#define WEBCONF_RESET_PIN 32

#define COLOR_ORDER GRB
#define LED_TYPE WS2812
#define NUM_LEDS 16




const PROGMEM uint16_t notes[] = {
  247, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494, 523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988, 1047
};