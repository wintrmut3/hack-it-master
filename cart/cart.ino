#include <FastLED.h>
#include <Stepper.h>
#include <Wire.h>

// 7 LIGHTS
// 30 SECONDS
// 3 LIVES

// CONSTANTS/GLOBAL DECLARATIONS
struct CRGB leds[24];
struct CRGB timer[7];
CRGB *pastLed = NULL;
int delaySpeed = 225;
int currLed = 0;
bool clockwise = true;
int randElement;
int numTargets;
int lives;
long startTime;
int tonePin = 6;
bool prevPressed = false;
int score = 0;

enum State {
    GAME_IDLE,
    PLAY,
    CORRECT,
    GAME_OVER,
    NUM_STATES
};

State state = GAME_IDLE;
int I2C_ADDRESS = 8;

void setup() {
  Wire.begin(I2C_ADDRESS);      // join i2c bus with address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event

  LEDS.addLeds<WS2812B, 11, GRB>(leds, 24);
  LEDS.addLeds<WS2812B, 13, GRB>(timer, 5);

  pinMode(8, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(tonePin, OUTPUT);
  pinMode(tonePin, OUTPUT);

  Serial.begin(9600);
  randomSeed(analogRead(0));
  randElement = random(0, 24);
  numTargets = 7;
  lives = 3;
  startTime = millis();
}

// Function to select a new random target led and increase speed
void reset() {
  int oldRand = randElement;
  leds[randElement] = CRGB::Black;
  FastLED.show();
  randElement = random(0, 24);
  // Re-select the randElement if it picks the same or an LED beside the old LED
  while (randElement == oldRand || randElement == (oldRand + 1) || randElement == (oldRand - 1)) {
    randElement = random(0, 24);
  }
  Serial.println(randElement);
  clockwise = !clockwise;
  delaySpeed -= 25;
}

// Function to run clockwise/left to right direction
void clockwiseCycle() {

  // Local variables
  long checkTime = 0;
  long currTime = 0;
  bool repeat = true;

  for (int i = currLed; i < 24; i ++) {
    bool invalPress = false;

    currTime = millis();
    // Indicate which led is currently selected
    // Vary indication based on if target led is selected
    if (i == randElement) {
      leds[i] = CRGB::LawnGreen;
      checkTime = millis();
    } else {
      leds[i] = CRGB::Blue;
      invalPress = true;
    }

    // Return previous led to its non-selected colour
    if (pastLed != NULL) {
      if (*pastLed == leds[randElement]) {
        *pastLed = CRGB::Yellow;
      } else {
        *pastLed = CRGB::Black;
      }
    }
    pastLed = &leds[i];
    FastLED.show();

    while (millis() < currTime + delaySpeed) {
      if (digitalRead(12) == LOW) {
        prevPressed = true;
      } else {
        prevPressed = false;
      }
      // Check if user input was instated during correct time frame
      if ((millis() - checkTime <= delaySpeed) && (prevPressed && digitalRead(12) == HIGH)) {
        hitSound();

        // Check win (targets)
        if (numTargets == 1) {
          winJingle();
          win();
        } else {
          numTargets -= 1;
        }
        // Check loss (lives)

        reset();
        // Adjust trackers
        currLed = i;
        checkTime = 0;
        repeat = false;
        break;
      // } else if (invalPress && digitalRead(12) == LOW) {
      //   if (lives == 0) {
      //     lose();            
      //   } else {
      //     invalPress = false;
      //     lives -= 1;
      //   }  
      }
    }
    if (!repeat) {
      break;
    }
  }

  // Reset index if target was not correctly pressed
  if (repeat) {
    currLed = 0;
  }
}

// Function to run counterclockwise/right to left direction
void counterclockwiseCycle() {

  // Local variables
  long checkTime = 0;
  long currTime = 0;
  bool repeat = true;

  for (int i = currLed; i >= 0; i --) {

    bool invalPress = false;
    
    currTime = millis();
    // Indicate which led is currently selected
    // Vary indication based on if target led is selected
    if (i == randElement) {
      leds[i] = CRGB::LawnGreen;
      checkTime = millis();
    } else {
      invalPress = true;
      leds[i] = CRGB::Blue;
    }

    // Return previous led to its non-selected colour
    if (pastLed != NULL) {
      if (*pastLed == leds[randElement]) {
        *pastLed = CRGB::Yellow;
      } else {
        *pastLed = CRGB::Black;
      }
    }
    pastLed = &leds[i];
    FastLED.show();

    while (millis() < currTime + delaySpeed) {
      if (digitalRead(12) == LOW) {
        prevPressed = true;
      } else {
        prevPressed = false;
      }
      // Check if user input was instated during correct time frame
      if ((millis() - checkTime <= delaySpeed) && (prevPressed && digitalRead(12) == HIGH)) {
        hitSound();
        // Check win (targets)
        if (numTargets == 1) { //looks like we win after hitting one target, this can change
          state = WIN;
        } else {
          numTargets -= 1;
        }
        // Check loss (lives)
        
        reset();
        // Adjust trackers
        currLed = i;
        checkTime = 0;
        repeat = false;
        break;
      // } else if (invalPress && digitalRead(12) == LOW) {
      //   if (lives == 0) {
      //     lose();
      //   } else {
      //     invalPress = false;
      //     lives -= 1;
      //   }  
      }
    }
    if (!repeat) {
      break;
    }
  }

  // Reset index if target was not correctly pressed
  if (repeat) {
    currLed = 23;
  }
}

// Function to run win animation
void win(){
  while(1){
    for (int i = 0;i<24;i++){
      if (i%2 != 0){
        leds[i] = CRGB::CRGB::LawnGreen;
        FastLED.show();
        delay(75);
      }
    }

    for (int i = 0;i<24;i++){
      if (i%2 == 0){
        leds[i] = CRGB::CRGB::LawnGreen;
        FastLED.show();
        delay(75);
      }
    }

    for (int i = 0; i<24; i++){
      leds[i] = CRGB::Black;
    }
    FastLED.show();
    delay(500);
    for (int loopNum = 0; loopNum<2;loopNum++){
      for (int i =0; i<24;i++){
        leds[i] = CRGB::CRGB::LawnGreen;
      }
      FastLED.show();
      delay(500);
      for (int i = 0; i<24; i++){
        leds[i] = CRGB::Black;
      }
      FastLED.show();
      delay(500);
    }
  }
}

// Function to run lose animation
void lose() {
  while(1){
    bool flip = true;
    for (int i = 0; i < 3; i++) {
      if (flip) {
        for (int i = 0; i <= 2; i++) {
          leds[i] = CRGB::Red;
          leds[3 + i] = CRGB::Black;
          leds[6 + i] = CRGB::Red;
          leds[9 + i] = CRGB::Black;
          leds[12 + i] = CRGB::Red;
          leds[15 + i] = CRGB::Black;
          leds[18 + i] = CRGB::Red;
          leds[21 + i] = CRGB::Black;
        }
      } else {
        for (int i = 0; i <= 2; i++) {
          leds[i] = CRGB::Black;
          leds[3 + i] = CRGB::Red;
          leds[6 + i] = CRGB::Black;
          leds[9 + i] = CRGB::Red;
          leds[12 + i] = CRGB::Black;
          leds[15 + i] = CRGB::Red;
          leds[18 + i] = CRGB::Black;
          leds[21 + i] = CRGB::Red;
        }
      }
      flip = !flip;
      FastLED.show();
      delay(175);
    }
  }
}

void loseJingle(){
  tone(tonePin,349);
  delay(400);
  tone(tonePin,256);
  delay(400);
  tone(tonePin,172);
  delay(700);
  noTone(tonePin);
  delay(50);
}

void winJingle(){
  tone(tonePin,440);
  delay(400);
  noTone(100); 
  tone(tonePin,660);
  delay(200);
  noTone(500); 
  tone(tonePin,880);
  delay(200);
  noTone(500);
  tone(tonePin,880);
  delay(200);
  noTone(200);  
  tone(tonePin,880);
  delay(200);
  noTone(200);
  tone(tonePin,880);
  delay(400);
  noTone(tonePin);
  delay(50);
}

void missSound(){
  tone(tonePin,200);
  delay(50);
  tone(tonePin,100);
  delay(200);
  noTone(tonePin);
  delay(50);
}

void hitSound(){
  tone(tonePin,300);
  delay(100);
  tone(tonePin,400);
  delay(50);
  noTone(tonePin);
  delay(50);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  if (state == GAME_OVER) {
    Wire.write(score); 
    state = GAME_IDLE;
  }
  else Wire.write(0b11111111);
}

// function that executes whenever data is received from master
    // this function is registered as an event, see setup()

void receiveEvent(int howMany){
  String str = "";
  while(Wire.available()) // loop through all 
  {
    char c = Wire.read(); // receive byte as a character
    str += c;
  }
  if (str == "S") state = START_GAME;
  else state = GAME_IDLE;
}

void loop() {
  
  if (state == GAME_IDLE) 
  {
    // Do nothing. I2C receive input will change this state
    displayString("-");
    // // Test: just move to start game state
    // state = START_GAME;
  } 
  else if (state == START_GAME){
    //initialization stuff goes here

    //generate a new randome element
  }
  else if (state == PLAY){
        // check loss (time)
    if (millis() - startTime > 30000) {
      state == LOSE;
    }

    leds[randElement] = CRGB::Yellow;
    FastLED.show();

    if (clockwise) {
      clockwiseCycle();
    } else {
      counterclockwiseCycle();
    }
  }
  else if (state == LOSE){
      loseJingle();
      lose();

      score = -1; //when indicating a lost game, is it negative?
  }
  else if (state == WIN){
      winJingle();
      win();
  }

  else if (state == GAME_OVER){
    //set all the LEDs to 0?
  }
  else{
    ;
  }
  delay(10);
}
