#include "Arduino.h"
// Signaller LEDs are arrayed as follows:
/*
 G  R   Game
53  52  CART
51  50  LBD
49  48  Catchit
47  46  Codeit
45  44  WireIt
43  42  HexIt

The red index is always green index -1
*/


#pragma once
#include "names.h"

void InitializeLEDPins(){
  for(uint8_t pin = 42; pin <=53; pin++){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}


void TestAllLEDPins(){
  for(uint8_t ppin = 42; ppin <=53; ppin++){
    digitalWrite(ppin, LOW);
    delay(200);
    digitalWrite(ppin, HIGH);
  }
}

void SetLEDForLeaderboard(bool showRed, bool showGreen){
  uint8_t basePin = 50; // red
    digitalWrite(basePin, !showRed);
  if(showGreen){
    digitalWrite(basePin+1, !showGreen);
  }


}
void SetLEDForGame(game g, bool showRed, bool showGreen){
  uint8_t basePin = 0;
  switch (g) {
    case CART: basePin = 52; break;
    case CODEIT: basePin=46; break;
    case WIREIT: basePin = 44; break;
    case HEXIT: basePin=42; break;
    case CATCHIT: basePin=48; break;
    default:
      return 255;
  }

    digitalWrite(basePin, !showRed);
    digitalWrite(basePin+1, !showGreen);
}
