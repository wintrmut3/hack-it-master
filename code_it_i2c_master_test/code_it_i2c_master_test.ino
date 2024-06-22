// Wire Controller Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI peripheral device
// Refer to the "Wire Peripheral Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  delay(5000);
  Wire.beginTransmission(8); // transmit to device #4
  Wire.write("S");        // sends five bytes
  Wire.endTransmission();    // stop transmitting
  delay(100);
  
  while (true) {
    Wire.requestFrom(8,1);
    uint_8 result;
    while (Wire.available()) { // peripheral may send less than requested
      result = Wire.read(); // receive a byte as character
      Serial.print(c);         // print the character
    }
    Serial.println();  
    delay(1000);
    if (result != 0b11111111) break;
  }
}

void loop() {
//  Wire.requestFrom(8, 6);    // request 6 bytes from peripheral device #8

//  while (Wire.available()) { // peripheral may send less than requested
 //   char c = Wire.read(); // receive a byte as character
 ////   Serial.print(c);         // print the character
 // }
//
//  delay(500);
}
