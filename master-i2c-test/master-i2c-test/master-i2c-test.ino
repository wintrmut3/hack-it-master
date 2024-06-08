#include <Wire.h>

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(9600);
}

void submitMessage(byte cmd, byte ctr){
    Wire.beginTransmission(0x33);
    Wire.write(cmd); // sends ascii 0 not literal 0
    Wire.write(ctr);
    Wire.endTransmission();
    Serial.print("Submitting message ");
    Serial.print(char(cmd));
    Serial.println(ctr);
}

int ctr = 0;
void loop() {
  // put your main code here, to run repeatedly:
  
  submitMessage(ctr%10==0?'F':'U',ctr);
  delay(1000);
  ctr++;
}
