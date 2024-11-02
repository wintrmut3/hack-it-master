int device_addr = 0xA; // DON'T FORGET TO SET THIS


#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  delay(5000);
}

void loop() {
  Wire.beginTransmission(device_addr);  // transmit to device
  Wire.write("S");            // sends five bytes
  Wire.endTransmission();     // stop transmitting
  Serial.print("Sent start command to device with address ");
  Serial.println(device_addr);
  delay(100);

  while (true) {
    Wire.requestFrom(device_addr, 1);
    uint8_t result;
    Serial.println("Requesting status update from device...");
  
    while (Wire.available()) {  // peripheral may send less than requested
      result = Wire.read();     // receive a byte as character
      Serial.print("Got response: ");
      Serial.println(result);     // print the character
    }
    if (result != 0b11111111) break;
    delay(1000);
  }
}
