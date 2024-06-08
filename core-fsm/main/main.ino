// HACK IT -- Master Controller Code
#include "names.h"
#include <Wire.h>
#include <assert.h>
#define MAXTIME 120
#define MAX_WAIT_GAME_TIME 5  // max seconds to wait until reading again.

state lastState, currentState, nextState;
int timeLeft;
int startTime;
int globalScore;
int startWAIT_SUBGAMETime;
uint8_t lastGameScore;
uint8_t currentGameAddress;
bool shouldEND_SUBGAME;
bool shouldCleanup;
game currentGame;


// Separate state transition logic and
// state execution logic into two funcs
void calculateNextState() {
  switch (currentState) {
    case INIT:
      nextState = START_SUBGAME;
      break;
    case START_SUBGAME:
      nextState = WAIT_SUBGAME;
      break;
    case WAIT_SUBGAME:
      nextState = shouldEND_SUBGAME ? END_SUBGAME : WAIT_SUBGAME;
      break;
    case END_SUBGAME:
      if (millis() - startTime >= MAXTIME * 1000) nextState = END_ALL_GAMES;
      else nextState = START_SUBGAME;
      break;
    case END_ALL_GAMES:
      nextState = WAIT_LDRBRD;
      break;
    case WAIT_LDRBRD:
      nextState = shouldCleanup ? CLEANUP : WAIT_LDRBRD;  // since we lock in a while loop, this is unnecessary
      break;
    case CLEANUP:
      nextState = INIT;
      break;
  }
}

void executeCurrentState() {
  switch (currentState) {
    case INIT:  // the start of one series of subgames
      timeLeft = MAXTIME;
      startTime = millis() / 1000;
      globalScore = 0;
      break;
    case START_SUBGAME:
      currentGame = 0;         // get a random number % NUM_GAMES
      currentGameAddress = 0;  // get address for game
      shouldEND_SUBGAME = false;
      Wire.beginTransmission(currentGameAddress);
      Wire.write('S');  // START GAME CMD
      Wire.endTransmission();
      break;
    case WAIT_SUBGAME:
      // Note - what happens if the game is still running but the global timer has timed out.
      Wire.beginTransmission(currentGameAddress);
      Wire.write('?');  // ARE YOU DONE CMD
      Wire.endTransmission();
      startWAIT_SUBGAMETime = millis();
      while (millis() - startWAIT_SUBGAMETime < MAX_WAIT_GAME_TIME * 1000) {
        while (Wire.available()) {
          // no commands sent from slave - > master.
          // if anything's on the bus, it's a score from [0,255]
          lastGameScore = Wire.read();
          shouldEND_SUBGAME = true;
        }
      }
      break;
    case END_SUBGAME:
      globalScore += lastGameScore;  // assuming this was just overwritten. last state must be from WAIT_SUBGAME
      lastGameScore = 11;
      assert(lastState == WAIT_SUBGAME);
      Wire.beginTransmission(-1);  // LEADERBOARD ADDRESS
      Wire.write('U');             // Send "update score command" to leaderboard
      Wire.write(lastGameScore);   // Update delta
      Wire.endTransmission();
      break;
    case END_ALL_GAMES:
      Wire.beginTransmission(-1);  // LEADERBOARD ADDRESS
      Wire.write('F');  // Send "Game Is Over" to leaderboard
      Wire.write(0);    // Any random number. To keep consistent 2 bytes.
      Wire.endTransmission();

      shouldCleanup = false;
      // At this point, leaderboard is waiting for user to type name.
      break;
    case WAIT_LDRBRD:
      // wait until the user has put their name in. timeout if they abandon?
      while (1) {  // assume finalize confimration is sent eventually
        while (Wire.available()) {
          char leaderboardResponse = Wire.read();
          Serial.print(leaderboardResponse);
          shouldCleanup = true;
        }
      }
      break;
    case CLEANUP:
      // prepare hackit for the next round of minigames.
      // assume correctly init'd by INIT
      break;
  }
}

void setup() {
  // first time setup only, PWRON state
  Wire.begin();
  currentState = INIT;
}

void loop() {
  executeCurrentState();
  calculateNextState();
  lastState = currentState;
  currentState = nextState;
}
