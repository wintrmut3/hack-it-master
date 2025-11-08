#include <FastLED.h>
#define NUM_LEDS 21
 
CRGBArray<NUM_LEDS> leds;
 
void InitTimer() {
   FastLED.addLeds<NEOPIXEL,12>(leds, NUM_LEDS); 
   FastLED.setBrightness(32);
}

void drawTimingStrip(int remainingTime, int maxTime){
  float normTime = (remainingTime * 1.0f/maxTime)*NUM_LEDS; // [0,NUM_LEDs)
  // float del = normTime - (int) normTime;
  // normTime = del < 0.5 ? (int) normTime : ((int)normTime +1);
  // int lastDrawnLED =  normtime * NUM_LEDS;

  for (int i= 0; i < NUM_LEDS; i++){
    if (i < normTime){
    leds[i] = CRGB::White; FastLED.show();
    }
    else{
      leds[i] = CRGB::Black; FastLED.show();
    }
  }
}