#include "names.h"
#include "logger.h"

//initialize these. game test_games[] = { CART, CATCHIT};
// returns number of connected games found
// if shuffleSubgames = true (defined in main.ino) then shuffle order before return.
int getConnectedGames(game* connectedGames, bool shuffleSubgames) {

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

  // early exit if not shuffling
  if(!shuffleSubgames) return wptr; 

  // subgame durstenfeld shuffler
  // at this point wptr = num_connected_games
  // connectedGames is filled up to num_connected_games elements [0, num_connected_games)
  randomSeed(analogRead(0));
  long randidx;
  for(int i = wptr-1; i >=1; i--){
    randidx = random(i+1); // [0, i]
    int curridx = i;
    // swap
    game tmp = connectedGames[curridx]; // copy value?
    connectedGames[curridx] = connectedGames[randidx];
    connectedGames[randidx] = tmp;
  }

  return wptr;
}