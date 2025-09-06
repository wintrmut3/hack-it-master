// HACK IT -- Master Controller Code

#include "names.h"
#include <Wire.h>
#include <assert.h>


#define LOGGING // undefine this to disable logging
#ifdef LOGGING
#include "logger.h"
#endif

#define MAXTIME 1200          // Max game runtime in seconds [testing - set to 1000. ]
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

char buff[17];

uint8_t cart_address = 0x6F;
uint8_t codeit_address = 0xC;  // currently not set - temp - C . before was 0x8
uint8_t leaderbrd_address = 0x10;
uint8_t wireit_address = 0xA;
uint8_t hexit_address = 0x9;
uint8_t catchit_address = 0x13;


// bool first_161_filter;

uint8_t gameToi2cAddress(game g) {
  switch (g) {
    case CART: return cart_address;
    case CODEIT: return codeit_address;
    case WIREIT: return wireit_address;
    case HEXIT: return hexit_address;
    case CATCHIT: return catchit_address;
    default:
      return 0;
  }
}

void blinkLED(int times = 1, int ms = 100) {
  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < times + 1; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(ms);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ms);
  }
}

// Separate state transition logic and
// state execution logic into two funcs
void calculateNextState() {
  switch (currentState) {
    case INIT:
      {
        nextState = START_SUBGAME;
        break;
      }
    case START_SUBGAME:
      {
        nextState = WAIT_SUBGAME;
        break;
      }
    case WAIT_SUBGAME:
      {
        nextState = shouldEND_SUBGAME ? END_SUBGAME : WAIT_SUBGAME;
        break;
      }
    case END_SUBGAME:
      {
        if (millis() - startTime >= MAXTIME * 1000UL) nextState = END_ALL_GAMES;
        else nextState = START_SUBGAME;
        break;
      }
    case END_ALL_GAMES:
      {
        nextState = WAIT_LDRBRD;
        break;
      }
    case WAIT_LDRBRD:
      {
        nextState = shouldCleanup ? CLEANUP : WAIT_LDRBRD;  // since we lock in a while loop, this is unnecessary
        break;
      }
    case CLEANUP:
      {
        nextState = INIT;
        break;
      }
  }
}

uint16_t game_ctr = 0;

game test_games[] = { CART };
// game test_games[] = { HEXIT };

uint8_t n_test_games = 1;

void executeCurrentState() {
  switch (currentState) {
    case INIT:  // the start of one series of subgames
      {
        blinkLED(2, 50);
        timeLeft = MAXTIME;
        startTime = millis() / 1000;
        globalScore = 0;
        Serial.println("init state");
        break;
      }
    case START_SUBGAME:
      {
        // go through games one at a time
        // currentGame = (enum game)(game_ctr % NUM_GAMES);
        // game_ctr++;

        // go through games under test only
        currentGame = test_games[game_ctr % n_test_games];
        game_ctr++;


        //rand()%NUM_GAMES;                                // rand()%NUM_GAMES;         // get a random number % NUM_GAMES
        currentGameAddress = gameToi2cAddress(currentGame);  // get address for game
        shouldEND_SUBGAME = false;
        blinkLED(3, 50);
        Serial.print("Starting subgame");
        Serial.println(currentGameAddress);
#ifdef LOGGING
        snprintf(buff, 16, "start sg@%x", currentGameAddress);
        log(buff);
#endif

        Wire.beginTransmission(currentGameAddress);
        Wire.write('S');  // START GAME CMD. TODO: Add difficulty (char)('S' + difficulty)
        Wire.endTransmission();

        // EXPERIMENT - Dump i2c bus constantly
        // for (;;) {
        //   Serial.println("Checking i2c bus for signal");
        //   while (Wire.available()) {
        //     uint8_t read_val = Wire.read();
        //     Serial.print("\t[DEBUG] i2c bus dump: ");
        //     Serial.println(read_val);
        //     Serial.flush();
        //   }
        // }

        delay(1000);  // don't tx start to subgame then immediately request. wait some minimum prewarm time.
        break;
      }
    case WAIT_SUBGAME:
      {
        // Note - what happens if the game is still running but the global timer has timed out.

        // Assume if any transmission is achieved then
        // Wire.beginTransmission(currentGameAddress);
        // Wire.write('?');  // ARE YOU DONE CMD
        // Wire.endTransmission();

        // startWAIT_SUBGAMETime = millis();
        // while (millis() - startWAIT_SUBGAMETime < MAX_WAIT_GAME_TIME * 1000 /*ms to s*/) {
        Serial.println("Awaiting subgame");  //this is getting spammed.

// char buff2 [32];
// sprintf(buff2, "wait sg %x", currentGameAddress);
// log(buff2);
#ifdef LOGGING
        log("wait sg");
#endif
        // Serial.println(millis());
        // Serial.println(startWAIT_SUBGAMETime);

        blinkLED(4);

        // flush the bus - invariant here is that nothing should be on the bus
        // while (Wire.available()) {
        //   Wire.read();
        // }

        // EXPERIMENT - keep requesting off a bus with nothing on it but i2c dev connected.
        // for (;;) {
        //   Serial.println("Checking i2c bus for signal");

        //   Wire.requestFrom(currentGameAddress, (uint8_t)8, (uint8_t)1); 
        //   delay(1000);
          
        //   while (Wire.available()) {
        //     uint8_t read_val = Wire.read();
        //     Serial.print("\t[DEBUG] i2c bus dump: ");
        //     Serial.println(read_val);
        //     Serial.flush();
        //   }
        // }

        Wire.requestFrom(currentGameAddress, (size_t)1);  // will trigger interrupt to callback onRequest on slave for 1 byte.
                                                          // slave should respond with 0XFF if it's not done.

        // if it doesn't get data on the bus, is it 161?
        // No - it looks like if there's nothing; requestFrom doesn't populate the buffer with 161 and instead
        // wire.available = false

        while (Wire.available()) {
          // no commands sent from slave - > master.
          // if anything's on the bus, it's a score from [0,255-1 ] (0xff is reserved for a "not done ack")

          lastGameScore = (uint8_t)Wire.read();  // if read 0xFF then it's not done. otherwise, transition out.

          // if (lastGameScore == 161 && first_161_filter) {
          //   first_161_filter = false;
          //   lastGameScore = 0;
          //   while (Wire.available()) {
          //     Wire.read();
          //   }
          //   break;
          // }

          if (lastGameScore != 0xff) {
            shouldEND_SUBGAME = true;
            // blinkLED(10, 50);

#ifdef LOGGING
            snprintf(buff, 16, "sg@%x: +%d", currentGameAddress, lastGameScore);
            log(buff);  // prints "test 2^1 = 2" ...etc
#endif
          }
        }

        if (!shouldEND_SUBGAME) {
          delay(MAX_WAIT_GAME_TIME * 1000);  // only delay if we got NOT READY response to let the slave continue
        }

        // }
        break;
      }
    case END_SUBGAME:
      {
        assert(lastState == WAIT_SUBGAME);
        //      last state must be from WAIT_SUBGAME assuming last game score was just written
        globalScore += lastGameScore;  // assuming this was just overwritten.
        // writeback to lastGameScore ? as extra verification.
        Serial.print("Last game score");
        Serial.println(lastGameScore);

        // write through to leaderboard and update quickly.
        Wire.beginTransmission(leaderbrd_address);  // LEADERBOARD ADDRESS
        Wire.write('U');                            // Send "update score command" to leaderboard
        Wire.write(lastGameScore);                  // Update delta
        Wire.endTransmission();

// char buff4[32];
#ifdef LOGGING
        snprintf(buff, 16, "p %d -> lb", lastGameScore);  // "print <score> to leaderboard"
        log(buff);
#endif


        //will autotransition to startSubgame if time is not up.
        break;
      }
    case END_ALL_GAMES:
      {
        Wire.beginTransmission(leaderbrd_address);  // LEADERBOARD ADDRESS
        Wire.write('F');                            // Send "Game Is Over" to leaderboard
        Wire.write(0);                              // Any random number. To keep consistent 2 bytes.
        Wire.endTransmission();

        shouldCleanup = false;
        // At this point, leaderboard is waiting for user to type name.
        break;
      }
    case WAIT_LDRBRD:
      {
        // wait until the user has put their name in. timeout if they abandon?
        while (1) {  // assume finalize confimration is sent eventually
          while (Wire.available()) {
            char leaderboardResponse = Wire.read();
            Serial.print(leaderboardResponse);
            shouldCleanup = true;
          }
        }
        break;
      }
    case CLEANUP:
      // prepare hackit for the next round of minigames.
      // assume correctly init'd by INIT
      break;
  }
}

void setup() {
  // first time setup only, PWRON state
  Wire.begin();

  Serial.begin(9600);
  
  currentState = INIT;
#ifdef LOGGING
  initLogger();
  log("Setup complete!");
#endif

  // first_161_filter = true;
  // Allow other subgames to warm up and boot
  delay(1000);
}

void loop() {
  executeCurrentState();
  calculateNextState();
  lastState = currentState;
  currentState = nextState;
}
