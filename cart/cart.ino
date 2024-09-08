#include <Adafruit_NeoPixel.h>
#include <Wire.h>

// 7 LIGHTS
// 30 SECONDS
// 3 LIVES

#define RING_PIN      9
#define NUMPIXELS     45
#define BUTTON_PIN    2
#define I2C_ADDRESS   0xF6

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RING_PIN, NEO_GRB + NEO_KHZ800);

// CONSTANTS/GLOBAL DECLARATIONS
uint32_t leds[NUMPIXELS];
uint32_t *pastLed = NULL;
int delaySpeed = 80;
int currLed = 0;
bool clockwise = true;
int randElement;
int numTargets;
int lives;
long startTime;
int tonePin = 6;
bool prevPressed = false;
int score= 0;

typedef enum State {
    GAME_IDLE,
    PLAY_GAME,
    WIN,
    LOSE,
    GAME_OVER,
    SEND_SCORE,
    NUM_STATES
} state_t;

state_t current_state = GAME_IDLE;

void setup() {
  Wire.begin(I2C_ADDRESS);      // join i2c bus with address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event

  pixels.begin();
  pixels.setBrightness(10);

  pinMode(tonePin, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  randomSeed(analogRead(0));
  randElement = random(0, NUMPIXELS);
  numTargets = 5;
  lives = 3; 
  startTime = millis();
}

// Function to select a new random target led and increase speed
void reset() {
  int oldRand = randElement;
  leds[randElement] = pixels.Color(0,0,0);
  pixels.setPixelColor(randElement, leds[randElement]);
  pixels.show();
  randElement = random(0, NUMPIXELS);
  // Re-select the randElement if it picks the same or an LED beside the old LED
  while (randElement == oldRand || randElement == (oldRand + 1) || randElement == (oldRand - 1)) {
    randElement = random(0, NUMPIXELS);
  }
  Serial.println(randElement);
  clockwise = !clockwise;
  delaySpeed -= 15;
}

// Function to run clockwise/left to right direction
void clockwiseCycle() {

  // Local variables
  long checkTime = 0;
  long currTime = 0;
  bool repeat = true;

  for (int i = currLed; i < NUMPIXELS; i ++) {
    bool invalPress = false;

    currTime = millis();
    // Indicate which led is currently selected
    // Vary indication based on if target led is selected
    if (i == randElement) {
      leds[i] = pixels.Color(0,255,0);
      checkTime = millis();
    } else {
      leds[i] = pixels.Color(0,0,255);
      invalPress = true;
    }
    pixels.setPixelColor(i, leds[i]);

    // Return previous led to its non-selected colour
    int prev_index = (i - 1 < 0) ? 44 : i-1;
    if (prev_index == randElement) {
      *pastLed = pixels.Color(255,255,0);
    } else {
      *pastLed = pixels.Color(0,0,0);
    }
    pixels.setPixelColor(prev_index, *pastLed);
    pixels.show();

    while (millis() < currTime + delaySpeed) {
      // Check if user input was instated during correct time frame
      if ((millis() - checkTime <= delaySpeed) && (digitalRead(BUTTON_PIN) == LOW)) {
        hitSound();

        // Check win (targets)
        if (numTargets == 1) {
          winJingle();
          win();
          current_state = GAME_OVER;
          break;
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
      }
    }
    if ((!repeat) || (current_state == GAME_OVER)) {
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
      leds[i] = pixels.Color(0,255,0);
      checkTime = millis();
    } else {
      leds[i] = pixels.Color(0,0,255);
      invalPress = true;
    }
    pixels.setPixelColor(i, leds[i]);

    // Return previous led to its non-selected colour
    int prev_index = (i + 1 > 44) ? 0 : i+1;
    if (prev_index == randElement) {
      *pastLed = pixels.Color(255,255,0);
    } else {
      *pastLed = pixels.Color(0,0,0);
    }
    pixels.setPixelColor(prev_index, *pastLed);
    pixels.show();

    while (millis() < currTime + delaySpeed) {
      // Check if user input was instated during correct time frame
      if ((millis() - checkTime <= delaySpeed) && (digitalRead(BUTTON_PIN) == LOW)) {
        hitSound();

        // Check win (targets)
        if (numTargets == 1) {
          winJingle();
          win();
          current_state = GAME_OVER;
          break;
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
      }
    }
    if ((!repeat) || (current_state == GAME_OVER)) {
      break;
    }
  }

  // Reset index if target was not correctly pressed
  if (repeat) {
    currLed = 44;
  }
}

// Function to run win animation
void win(){
    for (int i = 0;i<NUMPIXELS;i++){
      if (i%2 != 0){
        leds[i] = pixels.Color(0,255,0);
        pixels.setPixelColor(i, leds[i]);
        pixels.show();
        delay(100);
      }
    }

    for (int i = 0;i<NUMPIXELS;i++){
      if (i%2 == 0){
        leds[i] = pixels.Color(0,255,0);
        pixels.setPixelColor(i, leds[i]);
        pixels.show();
        delay(100);
      }
    }

    for (int i = 0; i<NUMPIXELS; i++){
      leds[i] = pixels.Color(0,0,0);
      pixels.setPixelColor(i, leds[i]);
    }
    pixels.show();
    delay(500);
    for (int loopNum = 0; loopNum<2;loopNum++){
      for (int i =0; i<NUMPIXELS;i++){
        leds[i] = pixels.Color(0,255,0);
        pixels.setPixelColor(i, leds[i]);
      }
      pixels.show();
      delay(500);
      for (int i = 0; i<NUMPIXELS; i++){
        leds[i] = pixels.Color(0,0,0);
        pixels.setPixelColor(i, leds[i]);
      }
      pixels.show();
      delay(500);
    }
}

// Function to run lose animation
void lose() {
    bool flip = true;
    for (int i = 0; i < 3; i++) {
      if (flip) {
        for (int i = 0; i < NUMPIXELS; i++) {
          if (i % 2 == 0) {
            leds[i] = pixels.Color(255,0,0);
          }
          else {
            leds[i] = pixels.Color(0,0,0);
          }
          pixels.setPixelColor(i, leds[i]);
        }
      } else {
        for (int i = 0; i < NUMPIXELS; i++) {
          if (i % 2 == 0) {
            leds[i] = pixels.Color(0,0,0);
          }
          else {
            leds[i] = pixels.Color(255,0,0);
          }
          pixels.setPixelColor(i, leds[i]);
        }
      }
      flip = !flip;
      pixels.show();
      delay(175);
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
  if (current_state == SEND_SCORE) {
    Wire.write(score); 
    current_state = GAME_IDLE;
  }
  else Wire.write(0xFF);
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
  if (str == "S") {
    current_state = PLAY_GAME;
    startTime = millis();
    }
  else current_state = GAME_IDLE;
}

void game_loop(){
  // check loss (time)
  if (millis() - startTime > 30000) {
      loseJingle();
      lose();  
      current_state = GAME_OVER;
  }

  leds[randElement] = pixels.Color(255,255,0);
  pixels.setPixelColor(randElement, leds[randElement]);
  pixels.show();

  if (clockwise) {
    clockwiseCycle();
  } else {
    counterclockwiseCycle();
  }
}


void loop() {

  switch(current_state){
    case GAME_IDLE:
      //turn off all the leds 
      randomSeed(analogRead(0));
      randElement = random(0, NUMPIXELS);
      numTargets = 5;
      lives = 3; 
      delaySpeed = 80;

      //don't need the button read when 
      if (digitalRead(BUTTON_PIN) == LOW){
        current_state = PLAY_GAME;
        startTime = millis();
      }
      break;
    case PLAY_GAME:
      game_loop();
      break;
    case GAME_OVER:
      for (int i = 0; i < NUMPIXELS; i++){
        leds[i] = pixels.Color(0,0,0);
      }
      current_state = SEND_SCORE;
      break;
    case SEND_SCORE:
      current_state = GAME_IDLE; //replace this with nothing once connected to master
      break;
    default:
      break; 
  }
}
