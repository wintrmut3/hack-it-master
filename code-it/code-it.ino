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
 *
*/
void showNumber(int num){
  lc.clearDisplay(0);
  for(int i = 0; i < 8; i++){
    if(!num){
      continue;
    }
    lc.setDigit(0,i,num%10,false);
    num /= 10;
  }
}
void displayString(String num){
  lc.clearDisplay(0);
  for(int i = 0; i < num.length(); i++){
    lc.setChar(0,i,num.charAt(num.length()-i-1),false);
  }
}


/*
 This portion is for the keypad input... 
 we give constant HIGH on 4 cols, and read output on 4 rows
 use an array to track
 Ignore the ABCD*# (we can add it easily)
*/

const byte ROWS = 3; 
const byte COLS = 3; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};
byte rowPins[ROWS] = {32, 33, 25};
byte colPins[COLS] = {17, 5, 18};
Keypad kp = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// int readInput(){
//   for(int rowIndex = 0; rowIndex < rowNums; rowIndex++){
//     pinMode(rowPins[rowIndex], INPUT);
//   }
//   for(int colIndex = 0; colIndex < colNums; colIndex++){
//     pinMode(colPins[colIndex], INPUT);
//   }
//   for(int rowIndex = 0; rowIndex < rowNums; rowIndex++){
//     pinMode(rowPins[rowIndex], OUTPUT);
//     digitalWrite(rowPins[rowIndex], HIGH);
//     for(int colIndex = 0; colIndex < colNums; colIndex++){
//       if(digitalRead(colPins[colIndex]) == HIGH){
//         // digitalWrite(rowPins[rowIndex], LOW);
//         pinMode(rowPins[rowIndex], INPUT);
//         return keyPadLayout[rowIndex][colIndex];
//       }
//     }
//   }
//   return -1;
// }
String target = "";
void setup() {
  for(int i = 0; i < 8; i++){
    target += (String)random(1,10);
  }
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

  displayString(target);
  delay(1000);
  displayString("");
  delay(1000);
  displayString(target);
  delay(1000);
  displayString("");
  delay(1000);
  displayString(target);
  delay(1000);
  displayString("");
}

// INT VERSION OF DISPLAY
// int n = -1;
// int MAX_INT = 99999999;
// void loop(){
//   char keyPressed = kp.getKey();
//   if(keyPressed && keyPressed >= (char)'0' && keyPressed <= (char)'9'){
//     if(n==-1){
//       n = keyPressed-48;
//     }else if (n*10 + keyPressed-48 <= MAX_INT){
//       n *= 10;
//       n += keyPressed-48;
//     }
//   }
  
//   if(n != -1){
//     showNumber(n);
//   }
//   delay(100);
// }

String num = "";
void loop() {
  char keyPressed = kp.getKey();
  // if it's a number
  if(keyPressed && keyPressed >= (char)'1' && keyPressed <= (char)'9'){
    // if(num.length() < 8){
      num += keyPressed;
    // }
  }
  displayString(num);
  if(num == target){
    delay(1000);
    displayString("");
    delay(1000);
    displayString(num);
    delay(1000);
    displayString("");
    delay(1000);
    displayString(num);
    delay(1000);
    displayString("");
    delay(1000);
    displayString("AC");
    delay(1000);
    displayString(target);
    delay(2000);
    String temp = "";
    int scoreCount = 0;
    for(int i = 0; i < 8; i++){
      if(num.charAt(i) != target.charAt(i)){
        temp += " ";
      }else{
        temp += num.charAt(i);
        scoreCount++;
      }
    }
    displayString(temp);
    delay(3000);
    displayString("");
    delay(1000);
    displayString((String)scoreCount);
    delay(2000);
    displayString("");
    delay(1000);
    target = "";
    num = "";
    for(int i = 0; i < 8; i++){
      target += (String)random(1,10);
    }
    displayString(target);
    delay(1000);
    displayString("");
    delay(1000);
    displayString(target);
    delay(1000);
    displayString("");
  }
  if(num.length() == 8){
    delay(1000);
    displayString("");
    delay(1000);
    displayString(num);
    delay(1000);
    displayString("");
    delay(1000);
    displayString(num);
    delay(1000);
    displayString("");
    delay(1000);
    displayString("no");
    delay(1000);
    displayString(target);
    delay(2000);
    String temp = "";
    int scoreCount = 0;
    for(int i = 0; i < 8; i++){
      if(num.charAt(i) != target.charAt(i)){
        temp += " ";
      }else{
        temp += num.charAt(i);
        scoreCount++;
      }
    }
    displayString(temp);
    delay(3000);
    displayString("");
    delay(1000);
    displayString((String)scoreCount);
    delay(2000);
    displayString("");
    delay(1000);
    target = "";
    num = "";
    for(int i = 0; i < 8; i++){
      target += (String)random(1,10);
    }
    displayString(target);
    delay(1000);
    displayString("");
    delay(1000);
    displayString(target);
    delay(1000);
    displayString("");
    delay(1000);
    displayString(target);
    delay(1000);
    displayString("");
  }
  delay(100);
}
