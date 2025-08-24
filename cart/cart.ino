#include <Adafruit_NeoPixel.h>
#include <Wire.h>

// 7 LIGHTS
// 30 SECONDS
// 3 LIVES

#define RING_PIN      A0
#define NUMPIXELS     45
#define BUTTON_PIN    2
#define I2C_ADDRESS   0xF6

// Bypass I2C triggered start
#define AUTOSTART

// Disable loss condition after 30s timeout
//#define ENABLE_LOSE

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, RING_PIN, NEO_GRB + NEO_KHZ800);

// CONSTANTS/GLOBAL DECLARATIONS
uint32_t leds[NUMPIXELS];
uint32_t pastLed = 0;
uint16_t delaySpeed = 80; // ms between moving LEDs
int currLed = 0;
bool clockwise = true;
int randElement;
int numTargets;
int lives;
long startTime;
int tonePin = 6;
bool prevPressed = false;
int score= 0;
volatile int difficulty = 0;

typedef enum State {
    GAME_IDLE,
    PLAY_GAME,
    WIN,
    LOSE,
    GAME_OVER,
    SEND_SCORE,
    NUM_STATES
} state_t;

volatile state_t current_state = GAME_IDLE;

typedef struct mailbox{
  char message = '\0';
  int dirty = 0;
} mailbox_t;

volatile mailbox_t mailbox;



void setup() {
  Wire.begin(I2C_ADDRESS);      // join i2c bus with address
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent); // register event

  pixels.begin();
  pixels.setBrightness(10);
    // Turn off all pixels first
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = pixels.Color(0, 0, 0);
    pixels.setPixelColor(i, leds[i]);
  }
  pixels.show();  // Update the ring



  pinMode(tonePin, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.begin(9600);
  randomSeed(analogRead(1));
  randElement = random(0, NUMPIXELS);
  numTargets = 5;
  lives = 3; 
}

void blankLEDs(){
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i] = pixels.Color(0, 0, 0);
    pixels.setPixelColor(i, leds[i]);
  }
  pixels.show();  // Update the ring

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
  delaySpeed -= 15; // 80 -> 65- > 50 -> 35 -> 20 -> 5 -> -10 (how to deal with negatives?)
  delaySpeed = constrain(delaySpeed, 3,1000); // don't allow it to go negative or impossibly fast
}

// Function to run clockwise/left to right direction
// Blocks for a full loop
void clockwiseCycle() {

  // Local variables
  long checkTime = 0;
  unsigned long currTime = 0;
  bool repeat = true;

  for (int i = currLed; i < NUMPIXELS; i ++) {
    // bool invalPress = false;

    currTime = millis();
    // Indicate which led is currently selected
    // Vary indication based on if target led is selected
    if (i == randElement) {
      leds[i] = pixels.Color(0,255,0);
      checkTime = millis();
    } else {
      leds[i] = pixels.Color(0,0,255);
      // invalPress = true;
    }
    pixels.setPixelColor(i, leds[i]);

    // Return previous led to its non-selected colour
    int prev_index = (i - 1 < 0) ? 44 : i-1;
    if (prev_index == randElement) {
      pastLed = pixels.Color(255,255,0);
    } else {
      pastLed = pixels.Color(0,0,0);
    }
    pixels.setPixelColor(prev_index, pastLed);
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

    // bool invalPress = false;
    
    currTime = millis();
    // Indicate which led is currently selected
    // Vary indication based on if target led is selected

    if (i == randElement) {
      leds[i] = pixels.Color(0,255,0);
      checkTime = millis();
    } else {
      leds[i] = pixels.Color(0,0,255);
      // invalPress = true;
    }
    pixels.setPixelColor(i, leds[i]);

    // Return previous led to its non-selected colour
    int prev_index = (i + 1 > 44) ? 0 : i+1;
    if (prev_index == randElement) {
      pastLed = pixels.Color(255,255,0);
    } else {
      pastLed = pixels.Color(0,0,0);
    }
    pixels.setPixelColor(prev_index, pastLed);
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
        delay(10);
      }
    }

    for (int i = 0;i<NUMPIXELS;i++){
      if (i%2 == 0){
        leds[i] = pixels.Color(0,255,0);
        pixels.setPixelColor(i, leds[i]);
        pixels.show();
        delay(10);
      }
    }

    for (int i = 0; i<NUMPIXELS; i++){
      leds[i] = pixels.Color(0,0,0);
      pixels.setPixelColor(i, leds[i]);
    }
    pixels.show();
    delay(100);
    for (int loopNum = 0; loopNum<2;loopNum++){
      for (int i =0; i<NUMPIXELS;i++){
        leds[i] = pixels.Color(0,255,0);
        pixels.setPixelColor(i, leds[i]);
      }
      pixels.show();
      delay(100);
      for (int i = 0; i<NUMPIXELS; i++){
        leds[i] = pixels.Color(0,0,0);
        pixels.setPixelColor(i, leds[i]);
      }
      pixels.show();
      delay(100);
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
  noTone(500UL); 
  tone(tonePin,880);
  delay(200);
  noTone(500UL);
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
    current_state = GAME_IDLE; // Don't really like state mutation in callbacks; can expose to race condition but not in this case
  }
  else Wire.write(0xFF);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
// Don't mutate current state
void receiveEvent(int howMany){
  char recv='\0';
  while(Wire.available()) // loop through all 
  {
    recv = Wire.read(); // receive byte as a character (last byte)
  }

  if (recv >= 'S' && recv <= 'Z') {
    mailbox.dirty = 1;
    mailbox.message = recv;
    }
}

void game_loop(){
  leds[randElement] = pixels.Color(255,255,0);
  pixels.setPixelColor(randElement, leds[randElement]);
  pixels.show();

  // Serial.println("Tick game loop step");

  if (clockwise) {
    clockwiseCycle();
  } else {
    counterclockwiseCycle();
  }

  #ifdef ENABLE_LOSE
  // check loss (time), check at the end of the loop
  if (millis() - startTime > 30000) {
      loseJingle();
      lose();  
      current_state = GAME_OVER;
  }
  #endif
}

int currentIndex = 0;
void showIdlePattern(){
 // Clear the ring first
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Turn off all pixels
  }
  pixels.show();  // Update the LEDs

  // Loop through all pixels and light them in a rotating pattern with varying brightness
  for (int i = 0; i < NUMPIXELS; i++) {
    int brightness = map(i, 0, NUMPIXELS - 1, 50, 255); // Vary brightness from low to high
    int hue = (i * 255 / NUMPIXELS + currentIndex * 5) % 255; // Smooth hue rotation effect
    pixels.setPixelColor(i, 0, hue*brightness*1.0/255, 0);  // Use custom Wheel function to generate the color
  }

  pixels.show();  // Update the LEDs

  // Increment the index to create the "rotation" effect
  currentIndex++;
  if (currentIndex >= 255) {
    currentIndex = 0;  // Reset the rotation after a full cycle
  }

  delay(10); // Adjust speed of rotation
}

void loop() {

  switch(current_state){
    case GAME_IDLE:
      randomSeed(analogRead(0));
      randElement = random(0, NUMPIXELS);
      // lives = 3; 

      showIdlePattern();
      //don't need the button read when connected to master
      #ifdef AUTOSTART
      if (digitalRead(BUTTON_PIN) == LOW){
        current_state = PLAY_GAME;
        startTime = millis();
      }
      // default difficulty setting
      numTargets = 5;
      delaySpeed = 80; // minval = 80-5*numTargets (5ms)
      blankLEDs();
      #endif
      #ifndef AUTOSTART
      if (mailbox.dirty){
        // Initialize game configuration globals
        startTime = millis();
        current_state = PLAY_GAME;
        difficulty = (int) (mailbox.message - 'S');  // 0-7
        char buf[64];
        snprintf(buf, sizeof(buf), "Starting CART with difficulty %d from mailbox message %c", difficulty, mailbox.message);
        Serial.println(buf);
        // reset mailbox

        numTargets = 1+difficulty;
        delaySpeed = 80-2*difficulty; // 80 to 64
        mailbox.message = '\0';
        mailbox.dirty = 0;

        blankLEDs();
      }
      #endif 
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
      #ifdef AUTOSTART
      current_state = GAME_IDLE; //bypass score 
      #endif
      break;
    default:
      break; 
  }
}
