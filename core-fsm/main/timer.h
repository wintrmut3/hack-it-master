#include <FastLED.h>
#define NUM_LEDS 21
 
CRGBArray<NUM_LEDS> leds;
 
void InitTimer() {
   FastLED.addLeds<NEOPIXEL,12>(leds, NUM_LEDS); 
   FastLED.setBrightness(32);
}

void drawTimingStrip(int remainingTime, int maxTime){
  float frac = remainingTime / float(maxTime);     // 1.0 → 0.0
  float ledFloat = frac * NUM_LEDS;                // number to fill

  for (int i=0; i < NUM_LEDS; i++) {

    // Compute smooth color from green → red
    uint8_t red   = (1.0f - frac) * 255;
    uint8_t green = frac * 255;
    uint8_t blue  = 0;

    if (i < ledFloat) {
      leds[i] = CRGB(red, green, blue);  // active LEDs colored
    } else {
      leds[i] = CRGB::Black;             // rest off
    }
  }
  FastLED.show();
}
