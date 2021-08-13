#include "RGBAmpy.h" 

int color;

void RGBAmpy::setup() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0, 1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  
    tft.print("Ampy");
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  
    tft.println("Synth!");
}

void RGBAmpy::loop() {
    int8_t thisHue = beat8(20,255);
    fill_rainbow(leds, NUM_LEDS, thisHue, 16);
    FastLED.show();

    color = tft.color565(leds[5].r, leds[5].g, leds[5].b);

    tft.setCursor(0, 0, 1);
    tft.setTextColor(color, TFT_BLACK);  
    tft.print("Ampy");
    tft.drawXBitmap(0, 40, AMPY_LOGO, LOGO_WIDTH, LOGO_HEIGHT, color, TFT_BLACK);
}

void RGBAmpy::encoderPress() {
    exit();
}