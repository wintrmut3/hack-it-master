#include <Wire.h>

enum state {
  IDLE,
  START_GAME,
  WAIT_INPUT,
  SHOW_SOLN,
  CLEANUP
};

struct mailbox{
  int score;
  bool ready;
};

int latchPin = 5;  // Latch pin (SCLK) of sr is connected to Digital pin 5
int clockPin = 6;  // Clock pin (RCK) of sr is connected to Digital pin 6
int dataPin = 7;   // Data pin (SER) of sr is connected to Digital pin 4

int buttonPins[4] = { 9, 10, 11, 12 };
int confirmPin = 13;
int ctr = 0;
int seg_loading_ctr;
int current_rand_hex;
struct mailbox mailbox;
// const int sevSegLookup[16] = {126,48,109,121,51,91,95,112,127,123,119,31,78,61,79,71};
const byte sevSegLookup[16] = { 252, 96, 218, 242, 102, 182, 190, 224, 254, 246, 238, 62, 156, 122, 158, 142 };

int getSwitchReading();
void showSwitchReadingOnHex();
void showHexDigit(int digit);
void OnWireRequest();
void OnWireReceive();

state current_state, next_state;
bool should_start_game;


void setup() {
  // Set all the pins of 74HC595 as OUTPUT
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(confirmPin, INPUT_PULLUP);
  for (int i = 0; i < 4; i++) pinMode(buttonPins[i], INPUT);

  Serial.begin(9600);
  randomSeed(analogRead(0));  // Set random based on noise from Analog0

  //init i2c 
  Wire.begin(0x9);
  Wire.onReceive(OnWireReceive);
  Wire.onRequest(OnWireRequest);

  current_state = IDLE;
}

void loop() {
  switch (current_state) {
    case IDLE:
      next_state = should_start_game ? START_GAME :IDLE;
      break;

    case START_GAME:
      // select a random hex digit and flash 3 times
      should_start_game = 0;
      current_rand_hex = random(16);
      for (int i = 0; i < 5; i++) {
        showHexDigit(current_rand_hex);
        delay(500);
        updateShiftRegister(0);  //clear display
        delay(200);
      }
      mailbox.score = 0;
      mailbox.ready = 0;

      // autotransition
      next_state = WAIT_INPUT;
      break;

    case WAIT_INPUT:
      if (digitalRead(confirmPin) == HIGH) {
        next_state = SHOW_SOLN;
      } else {
        seg_loading_ctr += 1;
        updateShiftRegister(1 << (seg_loading_ctr % 7));
        delay(500);
      }
      break;

    case SHOW_SOLN:
      // compute score
      int input_hex = getSwitchReading();
      int score = current_rand_hex == input_hex ? 50 : 0;  // 50 if correct. 0 if wrong.

      // flash what they put
      for (int i = 0; i < 3; i++) {
        showHexDigit(input_hex);
        delay(500);
        updateShiftRegister(0);  //clear display
        delay(200);
      }

      // flash the correct number
      for (int i = 0; i < 5; i++) {
        showHexDigit(current_rand_hex);
        delay(200);
        updateShiftRegister(0);  //clear display
        delay(100);
      }
      current_rand_hex = -1; // safety -- should throw some errors if encountered

      // enqueue for send to mailbox
      mailbox.score = score;
      mailbox.ready = 1;
      
      updateShiftRegister(0); // finally clear the display.
 
      next_state = CLEANUP;
      

      break;

    case CLEANUP: // wait here until the mailbox is cleaned.
      // the ready bit will be cleared by the onRequest callback once it is sent out.
      next_state = mailbox.ready ? CLEANUP : IDLE;
      break;
  }

  current_state = next_state;
}

void showSwitchReadingOnHex() {
  int switchInput = getSwitchReading();
  int leds = sevSegLookup[switchInput];
  updateShiftRegister(leds);
  Serial.println(switchInput);
  delay(100);
}

void showHexDigit(int digit) {
  int leds = sevSegLookup[digit];
  updateShiftRegister(leds);
}


/*
 * updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.
 */

void updateShiftRegister(int leds) {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
}

int getSwitchReading() {
  int reading = 0;
  for (int i = 0; i < 4; i++) {
    // multiplication has precedence over shift! bruh
    reading += (1 << i) * digitalRead(buttonPins[i]);
    Serial.print("Switch ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(digitalRead(buttonPins[i]));
    Serial.print("\t");
  }
  Serial.println(reading);
  return reading;
}

void testSevenSeg() {
  for (int i = 0; i < 255; i++) {
    Serial.println(i);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, 1);
    digitalWrite(latchPin, HIGH);
    delay(1000);
  }
}

void OnWireRequest(){
  if(mailbox.ready){
    Wire.write(mailbox.score);
    mailbox.ready = false;
  }
  else{
    // ignore this signal.
      Wire.write(0xFF);

  }
}
void OnWireReceive(){
  char c = Wire.read();
  if(c == 'S'){
    // start game
    should_start_game = true;
  }
}