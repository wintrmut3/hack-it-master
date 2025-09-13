#include "names.h"
#include "logger.h"

//initialize these. game test_games[] = { CART, CATCHIT};
void getConnectedGames(game* connectedGames) {

  int wptr = 0;
  Serial.println("Scanning...");
  for (byte address = 1; address < 127; ++address) {
    // The i2c_scanner uses the return value of
    // the Wire.endTransmission to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");

      game found_game = i2cAddressToGame(address);
      char buff[16];

      sprintf(buff, "i2c got %s@%x",game2str[found_game],address );
      log(buff);
      if (found_game!=NONE){
        connectedGames[wptr++] = found_game;
      }
      

    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  return connectedGames;
}