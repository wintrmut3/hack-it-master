int latchPin = 5;  // Latch pin (SCLK) of sr is connected to Digital pin 5
int clockPin = 6;  // Clock pin (RCK) of sr is connected to Digital pin 6
int dataPin = 7;   // Data pin (SER) of sr is connected to Digital pin 4

int buttonPins[4] = { 9, 10, 11, 12 };

int ctr = 0;
// const int sevSegLookup[16] = {126,48,109,121,51,91,95,112,127,123,119,31,78,61,79,71};
const byte sevSegLookup[16] = { 252, 96, 218, 242, 102, 182, 190, 224, 254, 246, 238, 62, 156, 122, 158, 142 };
int leds;
int getSwitchReading();
/*
 * setup() - this function runs once when you turn your Arduino on
 * We initialize the serial connection with the computer
 */
void setup() {
  // Set all the pins of 74HC595 as OUTPUT
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);

  for (int i = 0; i < 4; i++) pinMode(buttonPins[i], INPUT);

  Serial.begin(9600);
  // testSevenSeg();
}

/*
 * loop() - this function runs over and over again
 */
void loop() {

  //test
  // for (int i = 0; i < 8; i++)        // Turn all the LEDs ON one by one.
  // {
  //   bitSet(leds, 8-i);                // Set the bit that controls that LED in the variable 'leds'
  //   updateShiftRegister();
  //   delay(500);
  // }

  int switchInput  = getSwitchReading();
  leds = sevSegLookup[switchInput];
  updateShiftRegister();
  Serial.println(switchInput);
  delay(100);

  // for (int i = 0; i < 16; i++) {
  //   leds = sevSegLookup[i];
  //   updateShiftRegister();
  //   delay(200);
  // }
}

/*
 * updateShiftRegister() - This function sets the latchPin to low, then calls the Arduino function 'shiftOut' to shift out contents of variable 'leds' in the shift register before putting the 'latchPin' high again.
 */
 
void updateShiftRegister() {
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, leds);
  digitalWrite(latchPin, HIGH);
}

int getSwitchReading(){
  int reading = 0;
  for (int i = 0; i < 4; i++) {
    // multiplication has precedence over shift! bruh
    reading+= (1 << i) * digitalRead(buttonPins[i]);
    Serial.print("Switch ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(digitalRead(buttonPins[i]));
    Serial.print("\t");
  }
  Serial.println(reading);
  return reading;
}

void testSevenSeg(){
  for(int i = 0; i < 255 ;i++){
  Serial.println(i);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, 1);
  digitalWrite(latchPin, HIGH);
  delay(1000);
  }
}