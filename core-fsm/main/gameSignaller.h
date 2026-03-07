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

void SetLEDForGame(game g, bool showRed, bool showGreen);

void InitializeLEDPins(){
  for(uint8_t pin = 42; pin <=53; pin++){
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }
}


void TestAllLEDPins(){
  for(uint8_t ppin = 42; ppin <=53; ppin++){
    digitalWrite(ppin, LOW);
    delay(125);
    digitalWrite(ppin, HIGH);
  }
}

void BlinkAllLEDPins(bool showRed, bool showGreen, int nLoops){
  // return to false afterwards
  for(int loop = 0; loop < nLoops; loop++){
    for(int i = 0 ; i < NUM_GAMES; i++){
      SetLEDForGame((game)i, showRed, showGreen);
    }
    delay(100);
    for(int i = 0 ; i < NUM_GAMES; i++){
      SetLEDForGame((game)i, false, false);
    }
    delay(100);
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

// Do one loop starting at g_start_at -> g_start_at, then stop at g_stop_at
void RotateAllGameLEDs(game g_start_at, game g_stop_at){
  uint16_t delay_t = 150;

  int distance = ((int)g_stop_at - (int)g_start_at + NUM_GAMES) % NUM_GAMES;
  int n_steps = NUM_GAMES + distance;

  for(int i = 0; i < n_steps; i++){
    int index = ((int)g_start_at + i) % NUM_GAMES;
    game lit_game = (game)index;

    SetLEDForGame(lit_game, true, false);
    delay(delay_t);
    SetLEDForGame(lit_game, false, false);
    delay(delay_t);
  }
}
