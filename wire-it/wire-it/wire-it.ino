#include <FastLED.h>
#include<Wire.h>
enum state {
  IDLE,
  START_GAME,
  WAIT_GAME,
  END_GAME,
  CLEANUP
};

CRGB leftStrip[4];
CRGB rightStrip[4];

#define leftStripDataPin 5
#define rightStripdataPin 4

int l2r_mapping[4];
int completion[4];
int connected[4];
int connection_reading[4];


int analogPinMap[4] = { A0, A1, A2, A3 };

CRGB pal[11] = { CRGB::DeepSkyBlue, CRGB::DeepPink, CRGB::Coral, CRGB::Gold, CRGB::Indigo, CRGB::LightSeaGreen, CRGB::MintCream, CRGB::SteelBlue, CRGB::SlateGray, CRGB::Plaid, CRGB::Orchid };  // hardcoded color bindings

int game_difficulty = 1;

struct mailbox {
  int score;
  bool ready;
};
struct mailbox mailbox;
state current_state, next_state;
int should_start_game = 0;
void OnWireRequest();
void OnWireReceive();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  FastLED.addLeds<WS2812B, leftStripDataPin, GRB>(leftStrip, 4);
  FastLED.addLeds<WS2812B, rightStripdataPin, GRB>(rightStrip, 4);
  FastLED.setBrightness(32);

  //init i2c
  Wire.begin(0xA);
  Wire.onReceive(OnWireReceive);
  Wire.onRequest(OnWireRequest);
  randomSeed(analogRead(A0));
  current_state = IDLE;
}

void DrawLEDPairMap() {
  // update LED strip based on pair matrix.
  for (int i = 0; i < 4; i++) {
    if (completion[i]) {
      leftStrip[i] = CRGB::Green;
      rightStrip[l2r_mapping[i]] = CRGB::Green;
    } else if (!connected[i]) {
      // disconnected - show color pair
      leftStrip[i] = pal[i];
      rightStrip[l2r_mapping[i]] = pal[i];
    } else if (connected[i] && !completion[i]) {
      // connected but wrong --> shows the LHS connector RED?
      // also the RHS which it should be connected to as RED?
      // need to consider the logic more closely 
      // we don't preserve what the connection actually is. -- now in connection_reading
      
      /*
      If we're here, that means leftSide[i] is connected to something, that is incorrect
      We then illuminate the correct location with the rightstrip of what the left-based wire should be at
      But we could alternatively just red-highlight the incorrect connection as well. 
      */

      leftStrip[i] = CRGB::Red;
      // rightStrip[l2r_mapping[i]] = CRGB::Red; 
      rightStrip[connection_reading[i]] = CRGB::Red;
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
    connection_reading[i] = reading;
  }
}

bool IsGameComplete(){
  int correct = 0;
  for(int i= 0; i < 4 ;i ++){
    if (completion[i]) correct++;
  }
  Serial.print("Got #Correct = ");
  Serial.println(correct);
  return correct == 4;
}

void loop() {

  switch (current_state) {
    case IDLE:
      next_state = should_start_game ? START_GAME : IDLE;
      break;
    case START_GAME:
      FastLED.clear();
      should_start_game = false; // unset!
      

      // fisher_yates shuffle -> initialize
      for(int i = 0; i < 4; i++){
        l2r_mapping[i] = i;
      }
      
      for(int i =0 ;i < 4; i++){
        int swp_idx = 4-1-i;
        int rand_idx = random(4-i); 
        //swap elements @ rand_idx & swp_idx
        int tmp = l2r_mapping[swp_idx];
        l2r_mapping[swp_idx] = l2r_mapping[rand_idx];
        l2r_mapping[rand_idx] = tmp;
      }

      // shuffle palette - only first 4 els will be used.
      for(int i =0 ; i < 11 ; i++){
        int swp_idx = 11-1-i;
        int rand_idx = random(11-i); 
        //swap elements @ rand_idx & swp_idx
        CRGB tmp = pal[swp_idx];
        pal[swp_idx] = pal[rand_idx];
        pal[rand_idx] = tmp;
      }

      for (int i = 0; i < 4; i++) completion[i] = false;
      next_state = WAIT_GAME;
      break;
    case WAIT_GAME:
      DrawLEDPairMap();
      UpdateCompletionMap();

      next_state = IsGameComplete() ? END_GAME : WAIT_GAME;
      break;
    case END_GAME:
      mailbox.score = 10;
      mailbox.ready = 1;
      next_state = CLEANUP;
      break;
    case CLEANUP:
      // holding state for ack 
      next_state = mailbox.ready? CLEANUP:IDLE; 
      break;
  }
  current_state = next_state;
}


void OnWireRequest() {
  if (mailbox.ready) {
    Wire.write(mailbox.score);
    mailbox.ready = false;
  } else {
    // ignore this signal.
    Wire.write(0xFF);
  }
}

void OnWireReceive() {
  char c = Wire.read();
  if (c == 'S' || c == 'T' || c== 'U') {
    // start game
    should_start_game = true;
    // how does difficulty transmission work?
    game_difficulty = c -'S';
  }
}
