#include "RGBAmpy.h" 

int color;

void RGBAmpy::loop() {
    if(digitalRead(ENC_BTN) == LOW) {
        exit();
    }
    int8_t thisHue = beat8(20,255);
    fill_rainbow(leds, NUM_LEDS, thisHue, 16);
    FastLED.show();

    color = tft->color565(leds[0].r, leds[0].g, leds[0].b);

    tft->fillScreen(TFT_BLACK);
    tft->setCursor(0, 0, 1);
    tft->setTextColor(color, TFT_BLACK);  
    tft->setTextSize(2);
    tft->print("Ampy");
    tft->setTextColor(TFT_WHITE,TFT_BLACK);  
    tft->println("Synth!");
    tft->drawXBitmap(0, 40, AMPY_LOGO, LOGO_WIDTH, LOGO_HEIGHT, color, TFT_BLACK);
}