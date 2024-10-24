#include <FastLED.h>

enum state{
  IDLE,
  START_GAME,
  WAIT_GAME,
  END_GAME
}

CRGB leftStrip[4];
CRGB rightStrip[4];

#define leftStripDataPin 5
#define rightStripdataPin 4

int l2r_mapping[4];
int completion[4];
int connected[4];

int analogPinMap[4] = { A0, A1, A2, A3 };

CRGB palette[4] = { CRGB::DeepSkyBlue, CRGB::Blue, CRGB::DeepPink, CRGB::BlueViolet };  // hardcoded color bindings

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, leftStripDataPin, GRB>(leftStrip, 4);
  FastLED.addLeds<WS2812B, rightStripdataPin, GRB>(rightStrip, 4);
  FastLED.setBrightness(32);
  FastLED.clear();
  // test_init
  l2r_mapping[0] = 3;
  l2r_mapping[1] = 2;
  l2r_mapping[2] = 1;
  l2r_mapping[3] = 0;

  for (int i = 0; i < 4; i++) completion[i] = false;
}

void DrawLEDPairMap() {
  for (int i = 0; i < 4; i++) {
    if (completion[i]) {
      leftStrip[i] = CRGB::Green;
      rightStrip[l2r_mapping[i]] = CRGB::Green;
    }
    else if (!connected[i]) {
      // disconnected - show hint
      leftStrip[i] = palette[i];
      rightStrip[l2r_mapping[i]] = palette[i];
    }
    else if(connected[i] && !completion[i]){
      // connected but wrong
      leftStrip[i] = CRGB::Red;
      rightStrip[l2r_mapping[i]] = CRGB::Red;
    }
  }
  FastLED.show();
}


void UpdateCompletionMap() {
  for (int i = 0; i < 4; i++) {
    int reading = round(analogRead(analogPinMap[i]) / 256.0);
    Serial.print("Pin A");
    Serial.print(i);

    Serial.print(" connected to ");
    Serial.println(reading);
    
    completion[i] = (reading == l2r_mapping[i]);
    connected[i] = (reading != 4);
  }
}

void loop() {

  DrawLEDPairMap();
  UpdateCompletionMap();

  delay(500);
}
