//We always have to include the library
// circuits4you.com
// #include "LedControl.h"
#include <LedControl.h>
#include <Keypad.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 LedControl(a, b, c, x)
 pin a is connected to the DataIn 
 pin b is connected to the CLK 
 pin c is connected to LOAD 
 We have only a single MAX72XX. (x=1)
 */
LedControl lc=LedControl(23,21,22,1);

/* we always wait a bit between updates of the display */
unsigned long delaytime=250;

/* Display number num on lc
 * leading 0 is blank
 */
void displayInt(int num){
  lc.clearDisplay(0);
  for(int i = 0; i < 8; i++){
    if(!num){
      continue;
    }
    lc.setDigit(0,i,num%10,false);
    num /= 10;
  }
}
/* Display a string
 * Input a string (usually a number) and display it
 */
void displayString(String num){
  lc.clearDisplay(0);
  for(int i = 0; i < num.length(); i++){
    lc.setChar(0,i,num.charAt(num.length()-i-1),false);
  }
}

int FLASHING_DELAY = 1000; // How long we delay flashing numbers

// Define keyboard layout using Keypad.h
const byte ROWS = 3; 
const byte COLS = 3; 
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};
byte rowPins[ROWS] = {32, 33, 25}; // Digital pins used by the keypads (R1, R2, R3)
byte colPins[COLS] = {17, 5, 18}; // Digital pins used by the keypads (C1, C2, C3)
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Global variable remembering the target
String target = "";

// num is the current number entered
String num = "";

// Define a state machine
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

int score = 0;

void setup() {
  // Generate a random string with 8 digits, no zero
  for(int i = 0; i < 8; i++){
    target += (String)random(1,10);
  }

  // Define Display Pins
  pinMode(23, OUTPUT);
  pinMode(22, OUTPUT);
  pinMode(21, OUTPUT);

  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  // Initialize state
  state = GAME_IDLE;
}

void loop() {
  random(1,10); // Flush out random numbers

  if (state == GAME_IDLE) 
  {
    // Do nothing. I2C receive input will change this state
    // Test: just move to start game state
    state = START_GAME;
  } 
  else if (state == START_GAME) 
  {
    // Reset the score
    score = 0;

    // Next state
    state = START_ROUND;
  } 
  else if (state == START_ROUND) 
  {
    // Generate the random value and flash it
    target = "";
    num = "";
    for(int i = 0; i < 8; i++){
      target += (String)random(1,10);
    }

    // Flashing target value twice
    displayString(target);
    delay(FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);
    displayString(target);
    delay(FLASHING_DELAY);
    displayString("");
    
    // Proceed to next state
    state = INPUT_PHASE;
  } 
  else if (state == INPUT_PHASE) 
  {
    // Read user input and display on screen, detect if 8 values are filled in
    char keyPressed = kp.getKey();
    if(keyPressed && keyPressed >= (char)'1' && keyPressed <= (char)'9'){
        num += keyPressed;
    }
    displayString(num);

    // Proceed if round over
    if (num == target) state = CORRECT;
    else if (num.length() == 8) state = WRONG;
  } 
  else if (state == CORRECT) 
  {
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
    displayInt(score);
    delay(2 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    // Currently we end game after 1 round, but this logic can change
    state = GAME_OVER;
  } 
  else if (state == WRONG) 
  {
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
    for(int i = 0; i < 8; i++){
      if(num.charAt(i) != target.charAt(i)){
        temp += "-";
      }else{
        temp += num.charAt(i);
        score += 10;
      }
    }
    displayString(temp);
    delay(3 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    displayInt(score);
    delay(2 * FLASHING_DELAY);
    displayString("");
    delay(FLASHING_DELAY);

    // Currently we end game after 1 round, but this logic can change
    state = GAME_OVER;
  } 
  else if (state == GAME_OVER) 
  {
    // Same as idle state, display an image and just wait for onRequest to change state
    displayString("--------");
    // Test: just go back to idle
    delay(FLASHING_DELAY);
    state = GAME_IDLE;
  } 
  else 
  {
    // Nothing should be here
  }
  
  // Delay between cycles
  delay(10);
}
