#include <Wire.h>

void I2C_RxHandler(int numBytes)
{
  Serial.print("RX ");
  Serial.print(numBytes);
  int byteCtr = 0;
  byte cmdByte, RxByte;
  while(Wire.available()) {  // Read Any Received Data
    
    RxByte = Wire.read();
    
    if(byteCtr == 0){
      // expect RxByte == 'U' (update )or 'F' (finalize)
      cmdByte = RxByte;
    }
    else{
      // expect a score update
      if(cmdByte == 'U'){
        Serial.print("Score update recieved: +");
        Serial.println(RxByte); 
      }
      else if (cmdByte == 'F'){
        Serial.print("Finalize command recieved: Next value doesnt matter but was recieved -> ");
        Serial.println(RxByte); 
      }
    }
    byteCtr++;

  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(500);
  Serial.println("Hello");
  Serial.flush();
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  
  Wire.begin(0x33); // start as address 0x33
  Wire.onReceive(I2C_RxHandler);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(".");
  delay(5000);
}
