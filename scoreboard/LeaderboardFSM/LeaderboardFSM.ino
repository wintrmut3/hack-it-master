#include "LedControl.h"
#include <LittleFS_Mbed_RP2040.h>
#include <string.h>
#include <Wire.h>

#define KEY_SELECT 10
#define KEY_ACCEPT 11
#define I2C_SLAVE_ADDRESS 0x10
//#define RESET_LEADERBOARD_SWITCH // uncomment and upload to run reset program

bool force_leaderboard_reset_at_startup = false; // hold key accept + key select at startup to reset

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 K M V W X cannot be displayed
 */

 // LED control syntax (DIN=3,CLK=6, CS/LOAD=2,) - ref https://learn.carobot.ca/guide/1519-8-digit-7-seg
LedControl lc = LedControl(7, 9, 8, 1);   //assuming LC is the top LED
LedControl lc1 = LedControl(3, 6, 2, 1);  // change second pin to 6. we need the 4/5 pins for i2c0 comms to master
LedControl lc2 = LedControl(14, 19, 18, 1);
LedControl lc3 = LedControl(20, 22, 21, 1);

int leaderboardScores[3] = { 0, 0, 0 };

int currentIndex[3] = { 0, 0, 0 };
int currentDisplay = 7;
// char displayName[3] = { 'a', 'a', 'a' };
char defaultName[] = "aaa";
// int playerScore = 3456;
int playerScore; 
char playerScoreString[4];
char fileName0[] = MBED_LITTLEFS_FILE_PREFIX "/leaderboard0.txt";
char fileName1[] = MBED_LITTLEFS_FILE_PREFIX "/leaderboard1.txt";
char fileName2[] = MBED_LITTLEFS_FILE_PREFIX "/leaderboard2.txt";
char fileName3[] = MBED_LITTLEFS_FILE_PREFIX "/leaderboard3.txt";

LedControl currentDisplayBoard = lc3;
char currentFileName[27];
LittleFS_MBED *myFS;
int globalReset = false;
unsigned long delaytime = 2000;
unsigned long scanDelay = 200;
bool cursorVisible = true;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500;

enum States {
  INITIALIZE,
  UPDATE_SCORE,
  PLAYER_INPUT,
  FINALIZE
};

const char *stateNames[] = { "INITIALIZE", "UPDATE_SCORE", "PLAYER_INPUT" };

enum States currentState = INITIALIZE;

//i2c mailbox to be polled
struct i2c_data {
  int scoreUpdate = 0;  // 0 to 255 if valid, -1 if we want to send a gameover signal
  bool isFreshData = false;
};

volatile struct i2c_data i2c_mailbox;

uint32_t playerInputTOStart = 0;
uint32_t playerInputTOMax = 30* 1000;  // 30 second timeout for player input


/*
Storing leaderboard as:
XXX[]YYYY  
*/

// States
// 0 - Initialized - (updated) leaderboard showing, input OFF
// 1 - Game Start - score being updated using receiving updates from Master, input OFF
// 2 - Game End - input ON, keys getting scanned

void blinkLED(int times = 1, int ms = 100) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(ms);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ms);
  }
}

void I2C_TxHandler() {
  // master only expects a synchronization response when the LB is being written/finalized at end of all games
  if(currentState == PLAYER_INPUT){
    Wire.write(0xFF); // Acknowledge - but not done
  }else if(currentState == FINALIZE){
    Wire.write(0x00); // Acknowledge - done
    currentState = INITIALIZE; // move to initialize after finalizing
  }
}


void I2C_RxHandler(int numBytes) {
  blinkLED();  // blink once on data recv

  //Serial.print("RX ");
  //Serial.print(numBytes);
  int byteCtr = 0;
  byte cmdByte, RxByte;
  while (Wire.available()) {  // Read Any Received Data
    RxByte = Wire.read();
    if (byteCtr == 0) {
      // expect RxByte == 'U' (update )or 'F' (finalize)
      cmdByte = RxByte;
    } else {
      // expect a score update
      if (cmdByte == 'U') {
        //Serial.print("Score update recieved: +");
        //Serial.println(RxByte);
        i2c_mailbox.scoreUpdate = RxByte;
        i2c_mailbox.isFreshData = true;
      } else if (cmdByte == 'F') {
        //Serial.print("Finalize command recieved: Next value doesnt matter but was recieved -> ");
        //Serial.println(RxByte);
        i2c_mailbox.scoreUpdate = -1;
        i2c_mailbox.isFreshData = true;
      }
    }
    byteCtr++;
  }
}

/*
void UpdateScoreTest() {
  int scoreReceived = 0;
  scoreReceived = 5;
  playerScore += scoreReceived;
  updatePlayerScoreString();
  displayInit(defaultName, playerScoreString, lc3);
  delay(3000);
  scoreReceived = 100;
  playerScore += scoreReceived;
  updatePlayerScoreString();
  displayInit(defaultName, playerScoreString, lc3);
  delay(3000);
  scoreReceived = 999;
  playerScore += scoreReceived;
  updatePlayerScoreString();
  displayInit(defaultName, playerScoreString, lc3);
  delay(3000);
}
*/
void executeCurrentState() {
  //Serial.print("Executing current state : ");
  //Serial.println(stateNames[currentState]);
  Serial.flush();
  switch (currentState) {
    case INITIALIZE:
      {
        //Serial.println("INITIALIZE State");
        initializeLeaderboard();
        playerScore = 0;
        delay(100);
        currentState = UPDATE_SCORE;
        for(int i =0; i < 3 ; i++){
          currentIndex[i] =0;// reset input character at each pos to 'A'
        }
        return;
      }

    case UPDATE_SCORE:
      {
        //Serial.println("UPDATE_SCORE State");
        // message format : U* where * is an 1 byte score (0-255) for a score increase
        //                  F* where * is a don't care byte signalling that the game is over and the player should now input their name.
        while (1) {
          // continuously read from / poll the i2c mailbox, if a fresh message is there then handle it.
          if (i2c_mailbox.isFreshData) {
            // process and set fresh flag to false so we don't re-read it
             i2c_mailbox.isFreshData = false;  // remember to unset the mailbox flag
            if (i2c_mailbox.scoreUpdate >= 0) {
              // a score was recieved but the game continues
              int scoreReceived = i2c_mailbox.scoreUpdate;
              playerScore += scoreReceived;
              updatePlayerScoreString();
              displayInit(defaultName, playerScoreString, lc3);
              delay(100);                   // probably unnecessary
              currentState = UPDATE_SCORE;  // stay here. to be expliit

            } else {
              //Serial.println("GAME OVER - Exiting UPDATE_SCORE state");
              // if isFreshData && scoreUpdate < 0 (set to -1 on complete command)
              break;
            }
          }
          delay(100);
        }

        // transition out 
        currentState = PLAYER_INPUT;
        return;
      }

    case PLAYER_INPUT:
      {
        playerInputTOStart = millis();

        //Serial.println("PLAYER_INPUT State");
        // 30s fallback
        while (millis()-playerInputTOStart < playerInputTOMax) {
          // blink logic
          unsigned long currentTime = millis();
          if (currentTime - lastBlinkTime >= blinkInterval) {
            cursorVisible = !cursorVisible;
            lastBlinkTime = currentTime;
            if (cursorVisible) {
              char currentChar = returnLetter(currentIndex[7 - currentDisplay]);
              displayValue(currentDisplayBoard, currentDisplay, currentChar, false);
            } else {
              currentDisplayBoard.setRow(0, currentDisplay, 0);
            }
          }

          Scan_Key();
          delay(scanDelay);

          if (globalReset) {
            globalReset = false;
            currentState = FINALIZE;
            return;
          }
        }
        break;
      }
    case FINALIZE:
      {
        // transition to INITIALIZE after returning control to master
        // T/O here
        // transition logic in I2C_TxHandler
        break;
      }
    default:
      return;
  }
}


void initializeLeaderboard() {
  char *initName;
  char *initScore;
  currentDisplay = 7;

  initName = readFileName(fileName0);
  initScore = readFileScoreString(fileName0);

  displayInit(initName, initScore, lc);

  initName = readFileName(fileName1);
  initScore = readFileScoreString(fileName1);

  displayInit(initName, initScore, lc1);

  initName = readFileName(fileName2);
  initScore = readFileScoreString(fileName2);

  displayInit(initName, initScore, lc2);

  // initName = "aaa";
  // initScore = "0000";

  displayInit("aaa", "0000", lc3);

  currentDisplayBoard = lc3;
  strcpy(currentFileName, fileName3);

  updatePlayerScoreString();

  lastBlinkTime = millis();

  leaderboardScores[0] = readFileScore(fileName0);
  leaderboardScores[1] = readFileScore(fileName1);
  leaderboardScores[2] = readFileScore(fileName2);
}

void updatePlayerScoreString() {
  char playerScoreStringCopy[4];

  sprintf(playerScoreString, "%d", playerScore);
  strcpy(playerScoreStringCopy, playerScoreString);

  if (playerScore > 9999) {
    playerScore = 9999;
  }

  if (playerScore <= 9) {
    playerScoreString[3] = playerScoreStringCopy[0];
    playerScoreString[0] = '0';
    playerScoreString[1] = '0';
    playerScoreString[2] = '0';
  } else if (playerScore <= 99) {
    playerScoreString[2] = playerScoreStringCopy[0];
    playerScoreString[3] = playerScoreStringCopy[1];
    playerScoreString[0] = '0';
    playerScoreString[1] = '0';
  } else if (playerScore <= 999) {
    playerScoreString[1] = playerScoreStringCopy[0];
    playerScoreString[2] = playerScoreStringCopy[1];
    playerScoreString[3] = playerScoreStringCopy[2];
    playerScoreString[0] = '0';
  }
}



//din 7 ld 8 clk 9
/* we always wait a bit between updates of the display */

unsigned int alphabets[14] = {

  B01111011,  // G
  B00110111,  // H
  B00000110,  // I
  B00111000,  // J
  B00001110,  // L
  B01110110,  // N
  B01111110,  // O
  B01100111,  // P
  B00000101,  // R
  B01011011,  // S
  B00001111,  // T
  B00011100,  // U
  B00111110,  // V
  B00111011,  // Y

};

char currentLetter = 'a';


void displayValue(LedControl board, int index, char value, bool rawValue) {
  unsigned int valueToDisplay = UINT_MAX;
  ////Serial.print("*********\n");
  if (rawValue) {
    board.setRow(0, index, value);
    return;
  }

  switch (value) {

    case 'g':
      valueToDisplay = alphabets[0];
      break;
    case 'h':
      valueToDisplay = alphabets[1];
      break;
    case 'i':
      valueToDisplay = alphabets[2];
      break;
    case 'j':
      valueToDisplay = alphabets[3];
      break;
    case 'l':
      valueToDisplay = alphabets[4];
      break;
    case 'n':
      valueToDisplay = alphabets[5];
      break;
    case 'o':
      valueToDisplay = alphabets[6];
      break;
    case 'p':
      valueToDisplay = alphabets[7];
      break;
    case 'r':
      valueToDisplay = alphabets[8];
      break;
    case 's':
      valueToDisplay = alphabets[9];
      break;
    case 't':
      valueToDisplay = alphabets[10];
      break;
    case 'u':
      valueToDisplay = alphabets[11];
      break;
    case 'v':
      valueToDisplay = alphabets[12];
      break;
    case 'y':
      valueToDisplay = alphabets[13];
      break;
      // default:
      //   valueToDisplay = int(value);
      //   break;
  }

  if (valueToDisplay != UINT_MAX) {
    board.setRow(0, index, valueToDisplay);
  } else if ((int)value >= 48 && (int)value <= 57) {
    // //Serial.print("*********\n");
    // //Serial.print(value);
    board.setDigit(0, index, (int)value - 48, false);
  } else if (value >= 'a' && value <= 'f') {
    ////Serial.print(valueToDisplay);
    board.setChar(0, index, value, false);
  }

  return;
}

void Scan_Key() {
  if (digitalRead(KEY_ACCEPT) == 1) {
    //Serial.print("Accept Key Pressed\n");
    if (currentDisplay > 5) {
      int oldDisplay = currentDisplay;
      currentDisplay--;
      // show final char at old position
      char finalChar = returnLetter(currentIndex[7 - oldDisplay]);
      displayValue(currentDisplayBoard, oldDisplay, finalChar, false);
      // reset blink for new position
      cursorVisible = true;
      lastBlinkTime = millis();
    } else if (currentDisplay == 5) {
      char nameToStore[] = "aaa";
      int i = 0;
      for (i = 0; i < 3; i++) {
        nameToStore[i] = returnLetter(currentIndex[i]);
      }
      char *firstNameReturn;
      char *secondNameReturn;
      char firstName[4];
      char firstScore[5];
      char *firstScoreReturn;
      char *secondScoreReturn;
      char secondName[4];
      char secondScore[5];

      firstNameReturn = readFileName(fileName0);
      firstScoreReturn = readFileScoreString(fileName0);
      strcpy(firstName, firstNameReturn);
      strcpy(firstScore, firstScoreReturn);
      secondNameReturn = readFileName(fileName1);
      secondScoreReturn = readFileScoreString(fileName1);
      strcpy(secondName, secondNameReturn);
      strcpy(secondScore, secondScoreReturn);

      writeFile(currentFileName, nameToStore, sizeof(nameToStore));
      appendFile(currentFileName, playerScoreString, sizeof(playerScoreString));

      if (playerScore > leaderboardScores[0]) {

        writeFile(fileName0, nameToStore, sizeof(nameToStore));
        appendFile(fileName0, playerScoreString, sizeof(playerScoreString));

        writeFile(fileName1, firstName, sizeof(firstName));
        appendFile(fileName1, firstScore, sizeof(firstScore));

        writeFile(fileName2, secondName, sizeof(secondName));
        appendFile(fileName2, secondScore, sizeof(secondScore));

      }

      else if (playerScore > leaderboardScores[1]) {

        writeFile(fileName1, nameToStore, sizeof(nameToStore));
        appendFile(fileName1, playerScoreString, sizeof(playerScoreString));

        writeFile(fileName2, secondName, sizeof(secondName));
        appendFile(fileName2, secondScore, sizeof(secondScore));

      }

      else if (playerScore > leaderboardScores[2]) {
        writeFile(fileName2, nameToStore, sizeof(nameToStore));
        appendFile(fileName2, playerScoreString, sizeof(playerScoreString));
      }

      printLine();
      writeFile(currentFileName, nameToStore, sizeof(nameToStore));
      appendFile(currentFileName, playerScoreString, sizeof(playerScoreString));
      printLine();

      readFile(fileName0);
      printLine();
      readFile(fileName1);
      printLine();
      readFile(fileName2);
      printLine();
      readFile(fileName3);
      printLine();

      char *initName;
      char *initScore;

      initName = readFileName(fileName0);
      initScore = readFileScoreString(fileName0);

      displayInit(initName, initScore, lc);

      // free(initName);
      // free(initScore);

      initName = readFileName(fileName1);
      initScore = readFileScoreString(fileName1);

      displayInit(initName, initScore, lc1);

      // free(initName);
      // free(initScore);

      initName = readFileName(fileName2);
      initScore = readFileScoreString(fileName2);

      displayInit(initName, initScore, lc2);

      // free(initName);
      // free(initScore);

      // initName = "aaa";
      // initScore = "0000";
      
      // displayInit(initName, initScore, lc3);
      displayInit("aaa", "0000", lc3);
      currentDisplay = -1;
      //Serial.print("Name Entered\n");


      globalReset = true;
      return;
    }
  }

  if (digitalRead(KEY_SELECT) == 1 && currentDisplay != -1) {
    //Serial.println("Select key Pressed\n");
    char toDisplay = ++currentIndex[7 - currentDisplay];

    // Letter is between A and F
    if (currentIndex[7 - currentDisplay] >= 0 && currentIndex[7 - currentDisplay] <= 5) {
      toDisplay += 'a';

      displayValue(currentDisplayBoard, currentDisplay, toDisplay, false);
    }
    // If greater than F and smaller than Z
    else if (currentIndex[7 - currentDisplay] >= 6 && currentIndex[7 - currentDisplay] <= 19) {
      displayValue(currentDisplayBoard, currentDisplay, alphabets[currentIndex[7 - currentDisplay] - 6], true);
    }
    // Loop back to start
    else if (currentIndex[7 - currentDisplay] > 19) {
      currentIndex[7 - currentDisplay] = 0;
      displayValue(currentDisplayBoard, currentDisplay, 'a', false);
    }
    cursorVisible = true;
    lastBlinkTime = millis();
  }
}

char returnLetter(int index) {
  if (index >= 0 && index <= 5) {
    return ('a' + index);
  }
  // If greater than F and smaller than Z
  else if (index >= 6 && index <= 19) {
    switch (index) {
      case 6:
        return 'g';
      case 7:
        return 'h';
      case 8:
        return 'i';
      case 9:
        return 'j';
      case 10:
        return 'l';
      case 11:
        return 'n';
      case 12:
        return 'o';
      case 13:
        return 'p';
      case 14:
        return 'r';
      case 15:
        return 's';
      case 16:
        return 't';
      case 17:
        return 'u';
      case 18:
        return 'u';
      case 19:
        return 'y';
      default:
        return 'a';
    }
  }

  // default case
  return 'a';
}

void readFile(const char *path) {
  //Serial.print("Reading file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "r");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return;
  }

  char c;
  uint32_t numRead = 1;

  while (numRead) {
    numRead = fread((uint8_t *)&c, sizeof(c), 1, file);

    if (numRead) {
      //Serial.print(c);
    }
  }

  fclose(file);
}

int readFileScore(const char *path) {
  //Serial.print("Reading file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "r");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return 0;
  }

  char c;
  uint32_t numRead = 1;
  int digitsRead = -1;
  char scoreInFile[5];
  int scoreInFileInt;
  //Serial.print("\n");
  while (numRead) {
    numRead = fread((uint8_t *)&c, sizeof(c), 1, file);

    if (numRead) {

      if (digitsRead >= 0 && digitsRead <= 3) {
        digitsRead++;
        ////Serial.print(c);
        scoreInFile[digitsRead - 1] = c;
      }

      if (c == '\0') {
        digitsRead++;
      }
    }
  }
  scoreInFile[4] = '\0';
  scoreInFileInt = (int)strtol(scoreInFile, (char **)NULL, 10);

  ////Serial.print("************PRINTING SCORE\n");
  //Serial.print(scoreInFile);
  //Serial.print("\n");
  fclose(file);

  return scoreInFileInt;
}

char *readFileScoreString(const char *path) {
  //Serial.print("Reading file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "r");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return 0;
  }

  char c;
  uint32_t numRead = 1;
  int digitsRead = -1;
  static char scoreInFile[5];
  //Serial.print("\n");
  while (numRead) {
    numRead = fread((uint8_t *)&c, sizeof(c), 1, file);

    if (numRead) {

      if (digitsRead >= 0 && digitsRead <= 3) {
        digitsRead++;
        ////Serial.print(c);
        scoreInFile[digitsRead - 1] = c;
      }

      if (c == '\0') {
        digitsRead++;
      }
    }
  }
  scoreInFile[4] = '\0';
  fclose(file);

  return scoreInFile;
}

char *readFileName(const char *path) {
  //Serial.print("Reading file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "r");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return 0;
  }
  char c;
  uint32_t numRead = 1;
  int digitsRead = 0;
  static char nameInFile[4];

  while (numRead) {
    numRead = fread((uint8_t *)&c, sizeof(c), 1, file);
    if (numRead) {
      if (digitsRead >= 0 && digitsRead <= 2) {
        // //Serial.print("digits ");
        // //Serial.print(c);
        // //Serial.print("\n");
        nameInFile[digitsRead] = c;
        digitsRead++;
      } else {
        break;
      }
    }
  }
  nameInFile[3] = '\0';
  // //Serial.print("************PRINTING NAME\n");
  // //Serial.print("\n");
  ////Serial.print(nameInFile);
  //char *nameInFileReturn = (char*)malloc(4);
  // if(nameInFileReturn == NULL) {
  //   //Serial.print("NULLName\n");
  // }
  //strcpy(nameInFileReturn, nameInFile);
  fclose(file);

  return nameInFile;
}

void writeFile(const char *path, const char *message, size_t messageSize) {
  //Serial.print("Writing file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "w");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return;
  }

  if (fwrite((uint8_t *)message, 1, messageSize, file)) {
    //Serial.println("* Writing OK");
  } else {
    //Serial.println("* Writing failed");
  }

  fclose(file);
}

void appendFile(const char *path, const char *message, size_t messageSize) {
  //Serial.print("Appending file: ");
  //Serial.print(path);

  FILE *file = fopen(path, "a");

  if (file) {
    //Serial.println(" => Open OK");
  } else {
    //Serial.println(" => Open Failed");
    return;
  }

  if (fwrite((uint8_t *)message, 1, messageSize, file)) {
    //Serial.println("* Appending OK");
  } else {
    //Serial.println("* Appending failed");
  }

  fclose(file);
}

void deleteFile(const char *path) {
  //Serial.print("Deleting file: ");
  //Serial.print(path);

  if (remove(path) == 0) {
    //Serial.println(" => OK");
  } else {
    //Serial.println(" => Failed");
    return;
  }
}

void renameFile(const char *path1, const char *path2) {
  //Serial.print("Renaming file: ");
  //Serial.print(path1);
  //Serial.print(" to: ");
  //Serial.print(path2);

  if (rename(path1, path2) == 0) {
    //Serial.println(" => OK");
  } else {
    //Serial.println(" => Failed");
    return;
  }
}

void printLine() {
  //Serial.println("====================================================");
}

void displayInit(const char *name, const char *score, LedControl bd) {

  for (int i = 0; i < 3; i++) {
    displayValue(bd, 7 - i, name[i], false);
  }

  for (int j = 0; j < 4; j++) {
    displayValue(bd, 3 - j, score[j], false);
  }
}



void setup() {

  pinMode(KEY_ACCEPT, INPUT_PULLDOWN);
  pinMode(KEY_SELECT, INPUT_PULLDOWN);

  force_leaderboard_reset_at_startup = digitalRead(KEY_ACCEPT) && digitalRead(KEY_SELECT);

  pinMode(LED_BUILTIN, OUTPUT);

  blinkLED(5, 50);  // rapid blink on startup
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);


  lc1.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc1.setIntensity(0, 8);
  /* and clear the display */
  lc1.clearDisplay(0);

  lc2.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc2.setIntensity(0, 8);
  /* and clear the display */
  lc2.clearDisplay(0);

  lc3.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc3.setIntensity(0, 8);
  /* and clear the display */
  lc3.clearDisplay(0);

  
      // test
    // UpdateScoreTest();

  // Serial.begin(9600);
  // while (!Serial) //bruh
  //   delay(100);
  //Serial.println("Serial acquired");
  // Serial.flush();

  //Serial.print("\nStart LittleFS_Test on ");
  //Serial.println(BOARD_NAME);
  //Serial.println(LFS_MBED_RP2040_VERSION);

#if defined(LFS_MBED_RP2040_VERSION_MIN)

  if (LFS_MBED_RP2040_VERSION_INT < LFS_MBED_RP2040_VERSION_MIN) {
    //Serial.print("Warning. Must use this example on Version equal or later than : ");
    //Serial.println(LFS_MBED_RP2040_VERSION_MIN_TARGET);
  }

#endif

  myFS = new LittleFS_MBED();

  if (!myFS->init()) {
    //Serial.println("LITTLEFS Mount Failed");

    return;
  }

  //Serial.println("\nTest complete");

  // Reset Leaderboard Code - Uncomment below, Comment initializeLeaderboard() and loop code
if(force_leaderboard_reset_at_startup){
  //Serial.println("Resetting leaderboard!");
  char nameToStore[] = "aaa";
  char playerScoreString[] = "0000";

  writeFile(fileName0, nameToStore, sizeof(nameToStore));
  appendFile(fileName0, playerScoreString, sizeof(playerScoreString));

  writeFile(fileName1, nameToStore, sizeof(nameToStore));
  appendFile(fileName1, playerScoreString, sizeof(playerScoreString));

  writeFile(fileName2, nameToStore, sizeof(nameToStore));
  appendFile(fileName2, playerScoreString, sizeof(playerScoreString));
}

  Wire.begin(I2C_SLAVE_ADDRESS);  // start as address 0x33
  Wire.onReceive(I2C_RxHandler);
  Wire.onRequest(I2C_TxHandler);

  // initializeLeaderboard();
  currentState = INITIALIZE;
}

void loop() {
  
  // if we reset 
  if(force_leaderboard_reset_at_startup) return;

  executeCurrentState();
  // if (globalReset) {
  //   globalReset = false;
  //   initializeLeaderboard();
  //   return;
  // }

  // IDLE PHASE
  // Scan_Key();
  // delay(scanDelay);
}
