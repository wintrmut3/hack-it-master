int device_addr = 0xF6;  // DON'T FORGET TO SET THIS
int device_poll_delay = 5000;

#include <Wire.h>
int difficulty = 0;

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  Serial.println("Initialize master controller.");
  
  delay(5000);
}

void loop() {
  char ch_difficulty  = (char)('S'+difficulty);
  Wire.beginTransmission(device_addr);  // transmit to device
  Wire.write(ch_difficulty);                      // sends start command
  Wire.endTransmission();               // stop transmitting
  Serial.print("Sent start command to device with address ");
  Serial.print(device_addr);
  Serial.print(" and difficulty ");
  Serial.println(ch_difficulty);
  delay(100);

  while (true) {
    Wire.requestFrom(device_addr, 1);
    uint8_t result = 0xff;
    Serial.println("Requesting status up date from device...");

    while (Wire.available()) {  // peripheral may send less than requested
      result = Wire.read();     // receive a byte as character
      Serial.print("Got response: ");
      Serial.println(result);  // print the character
    }
    if (result != 0xff) break;
    delay(device_poll_delay);
  }
  Serial.print("Waiting 5 seconds before new game; incremeting difficulty");
  difficulty++;
  delay(5000);
}
