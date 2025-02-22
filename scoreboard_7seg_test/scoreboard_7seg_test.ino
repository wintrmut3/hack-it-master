#include <LedControl.h>
 // LED control syntax (DIN=3,CLK=6, CS/LOAD=2,) - ref https://learn.carobot.ca/guide/1519-8-digit-7-seg
LedControl lc1 = LedControl(3, 6, 2, 1);  // change second pin to 6. we need the 4/5 pins for i2c0 comms to master
LedControl lc2 = LedControl(7,9,8,1);
LedControl lc3 = LedControl(14, 19, 18, 1);
LedControl lc4 = LedControl(20, 22, 21, 1);


unsigned int alphabets[14] = {

  B01111011,  // G
  B00110111,  // H
  B00000110,  // I
  B00111000,  // J
  B00001110,  // L
  B01110110,  // N
  B01111110,  // O
  B01100111,  // P
  B00000101,  // R
  B01011011,  // S
  B00001111,  // T
  B00011100,  // U
  B00111110,  // V
  B00111011,  // Y

};

void displayValue(LedControl board, int index, char value, bool rawValue) {
  unsigned int valueToDisplay = -1;
  ////Serial.print("*********\n");
  if (rawValue) {
    board.setRow(0, index, value);
    return;
  }

  switch (value) {

    case 'g':
      valueToDisplay = alphabets[0];
      break;
    case 'h':
      valueToDisplay = alphabets[1];
      break;
    case 'i':
      valueToDisplay = alphabets[2];
      break;
    case 'j':
      valueToDisplay = alphabets[3];
      break;
    case 'l':
      valueToDisplay = alphabets[4];
      break;
    case 'n':
      valueToDisplay = alphabets[5];
      break;
    case 'o':
      valueToDisplay = alphabets[6];
      break;
    case 'p':
      valueToDisplay = alphabets[7];
      break;
    case 'r':
      valueToDisplay = alphabets[8];
      break;
    case 's':
      valueToDisplay = alphabets[9];
      break;
    case 't':
      valueToDisplay = alphabets[10];
      break;
    case 'u':
      valueToDisplay = alphabets[11];
      break;
    case 'v':
      valueToDisplay = alphabets[12];
      break;
    case 'y':
      valueToDisplay = alphabets[13];
      break;
      // default:
      //   valueToDisplay = int(value);
      //   break;
  }

  if (valueToDisplay != -1) {
    board.setRow(0, index, valueToDisplay);
  } else if ((int)value >= 48 && (int)value <= 57) {
    // //Serial.print("*********\n");
    // //Serial.print(value);
    board.setDigit(0, index, (int)value - 48, false);
  } else if (value >= 'a' && value <= 'f') {
    ////Serial.print(valueToDisplay);
    board.setChar(0, index, value, false);
  }

  return;
}

void init7seg(LedControl lc){
  int intensity = 8;
  lc.shutdown(0,false);
  lc.setIntensity(0, intensity); // low intensity
  lc.clearDisplay(0);
}

void setup() {
  // put your setup code here, to run once:
  delay(500); // delay for power


  init7seg(lc4);
  delay(100);
  init7seg(lc1);
  delay(100);
  init7seg(lc2);
  delay(100);
  init7seg(lc3);
  delay(100);




  // lc1.shutdown(0, false);
  // /* Set the brightness to a medium values */
  // lc1.setIntensity(0, 8);
  // /* and clear the display */
  // lc1.clearDisplay(0);

  // lc2.shutdown(0, false);
  // /* Set the brightness to a medium values */
  // lc2.setIntensity(0, 8);
  // /* and clear the display */
  // lc2.clearDisplay(0);


}
void drawRandom(){
    for (int j = 0; j < 0xF; j++) {
    for (int i = 0; i < 8; i++) {
      displayValue(lc1, i, '0'+rand()%10, false);
      displayValue(lc2, i, '0'+rand()%10, false);
      displayValue(lc3, i, '0'+rand()%10, false);
      displayValue(lc4, i, '0'+rand()%10, false);


        delay(10);
    }
  }
}

void drawIDs(){
    for (int i = 0; i < 8; i++) {
      displayValue(lc1, i, '0'+i, false);
      displayValue(lc2, i, '0'+i, false);
      displayValue(lc3, i, '0'+i, false);
      displayValue(lc4, i, '0'+i, false);
    }
}

void loop() {
  drawRandom();
  // drawIDs();
}
