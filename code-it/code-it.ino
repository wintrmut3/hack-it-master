#include <LedControl.h>
#include <Keypad.h>
#include <Wire.h>

#define USE_SERIAL // comment this out to disable serial.prints

// #define AUTOSTART
/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 LedControl(a, b, c, x)
 pin a is connected to the DataIn 
 pin b is connected to the CLK 
 pin c is connected to LOAD 
 We have only a single MAX72XX. (x=1)
 */
// LedControl lc = LedControl(23, 12, 14, 1);
LedControl lc = LedControl(13, 12, 11, 1);

/* we always wait a bit between updates of the display */
unsigned long delaytime = 250;

/* Display number num on lc
 * leading 0 is blank
 */
void displayInt(int num) {
  lc.clearDisplay(0);  // clear display at addr 0
  // Specifically display 0
  if (num == 0) {
    lc.setDigit(0, 0, 0, false);
    return;
  }
  // Not-Zero
  for (int i = 0; i < 8; i++) {
    if (!num) {
      continue;
    }
    lc.setDigit(0, i, num % 10, false);
    num /= 10;
  }
}
/* Display a string
 * Input a string (usually a number) and display it
 */
void displayString(String num) {
  lc.clearDisplay(0);
  for (int i = 0; i < num.length(); i++) {
    lc.setChar(0, i, num.charAt(num.length() - i - 1), false);
  }
}

int FLASHING_DELAY = 1000;  // How long we delay flashing numbers

// Define keyboard layout using Keypad.h
const byte ROWS = 3;
const byte COLS = 3;
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' }
};
// byte rowPins[ROWS] = { 32, 33, 25 };  // Digital pins used by the keypads (R1, R2, R3)
// byte colPins[COLS] = { 17, 5, 18 };   // Digital pins used by the keypads (C1, C2, C3)
byte rowPins[ROWS] = { 8, 9, 10 };  // Digital pins used by the keypads (R1, R2, R3)
byte colPins[COLS] = { 2, 3, 4 };   // Digital pins used by the keypads (C1, C2, C3)
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Global variable remembering the target
String target = "";

// num is the current number entered
String num = "";

// Define a state machine
// TODO: can add a "transition to idle and game over" state to prevent constantly updating display
enum State {
  GAME_IDLE,
  START_GAME,
  START_ROUND,
  INPUT_PHASE,
  CORRECT,
  WRONG,
  GAME_OVER,
  NUM_STATES
};

State state = GAME_IDLE;

// Difficulty setting from 0 to 7, 7 being hardest. (0 displays only 1 number, while 7 displays all 8 numbers)
// Score still scales from 0 to 80 for correct, and 100 for all correct.
int difficulty = 7;

/* Score Logic:
 * 8 number max, All correct gets 100 score. not all correct get 10 score per correct to max of 70 score.
 */
uint8_t score = 0;

int I2C_ADDRESS = 0xC; 

void setup() {
  #ifdef USE_SERIAL
  Serial.begin(9600);
  #endif
  Wire.begin(I2C_ADDRESS);         // join i2c bus with address
  Wire.onRequest(onRequestEvent);  // register event
  Wire.onReceive(onReceiveEvent);  // register event

  // Define Display Pins
  pinMode(23, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);

  // Initialize state
  state = GAME_IDLE;
  #ifdef USE_SERIAL
  Serial.println("Starting setup");
  #endif

}

void loop() {
  random(1, 10);  // Flush out random numbers

  if (state == GAME_IDLE) {
    // Do nothing. I2C receive input will change this state
    displayString("-");
    // // Test: just move to start game state
    #ifdef AUTOSTART
      #ifdef USE_SERIAL
      Serial.println("Idle ");
      #endif
    if (state == GAME_IDLE) {
      // EXPERIMENT code - not real logic
      // difficulty = static_cast<int>(str[0]) - static_cast<int>('S');  // S is difficulty 0, Z is difficulty 7
      // difficulty = difficulty / 2 + 4;
                                      // remap S->Z range to show 4 to 4+4 = 8 chars.
      difficulty = 4;
      state = START_GAME;
    }
    #endif

  } else if (state == START_GAME) {
    // Reset the score
    score = 0;

    // Next state
    state = START_ROUND;
  } else if (state == START_ROUND) {
    #ifdef USE_SERIAL
      Serial.println("Starting game");
    #endif
    // Generate the random value and flash it
    target = "";
    num = "";
    for (int i = 0; i < difficulty + 1; i++) {
      target += (String)random(1, 10);
    }

    // Flashing target value twice
    displayString(target);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(target);
    delay(FLASHING_DELAY);
    displayString("");
    #ifdef USE_SERIAL
    Serial.println("Move to for input");
    #endif

    // Proceed to next state
    state = INPUT_PHASE;
  } else if (state == INPUT_PHASE) {
    // Read user input and display on screen, detect if 8 values are filled in
    char keyPressed = kp.getKey();
    if (keyPressed && keyPressed >= (char)'1' && keyPressed <= (char)'9') {
      num += keyPressed;
      #ifdef USE_SERIAL
      Serial.print("Actual input: ");
      Serial.println(num);
      #endif
      displayString(num);
    
    }

    // Proceed if round over
    if (num == target) state = CORRECT;
    else if (num.length() == difficulty + 1) state = WRONG;
  } else if (state == CORRECT) {
    // Flash entered result, display AC, add score, display current score
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(num);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(num);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString("AC");
    delay(FLASHING_DELAY);

    score += 100;
    displayInt((int)score);
    delay(2 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    // Currently we end game after 1 round, but this logic can change
    state = GAME_OVER;
  } else if (state == WRONG) {
    // Flash entered result, display no, show correct answer, show overlay (twice), add score, display current score
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(num);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(num);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString("no");
    delay(FLASHING_DELAY);
    displayString(target);
    delay(2 * FLASHING_DELAY);
    String temp = "";
    for (int i = 0; i < difficulty + 1; i++) {
      if (num.charAt(i) != target.charAt(i)) {
        temp += "-";
      } else {
        temp += num.charAt(i);
        score += 80 / (difficulty + 1);
      }
    }
    displayString(temp);
    delay(3 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    displayInt((int)score);
    delay(2 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    // Currently we end game after 1 round, but this logic can change
    state = GAME_OVER;
  } else if (state == GAME_OVER) {
    // Same as idle state, display an image and just wait for onRequest to change state
    displayString("--------");

    #ifdef AUTOSTART
    state = GAME_IDLE;
    #endif

  } else {
    // Nothing should be here
  }

  // Delay between cycles
  delay(10);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void onRequestEvent() {
  if (state == GAME_OVER) {
    Wire.write(score);
    score = 0;
    state = GAME_IDLE;
  } 
  else {
      Wire.write(0xff);
  }
}
// function that executes whenever data is received from master
// this function is registered as an event, see setup()

// The game starts whenever it receives ANYTHING, nothing will force game to stop
void onReceiveEvent(int howMany) {
  String str = "";
  while (Wire.available())  // loop through all
  {
    char c = Wire.read();  // receive byte as a character
    str += c;
  }

  if (state == GAME_IDLE) {
    difficulty = static_cast<int>(str[0]) - static_cast<int>('S');  // S is difficulty 0, Z is difficulty 7
    difficulty = difficulty / 2 + 4;                                // remap S->Z range to show 4 to 4+4 = 8 chars.
    state = START_GAME;
  }
}
