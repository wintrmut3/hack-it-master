/*
How to use:
1. import "logger.h" // make sure this is in the same directory as your source code
2. inside setup (after running serial.begin):  run initLogger();
3. Call log(char* s).

You can pass a string literal for s, like "testing"
or a format string like the following example with sprintf;

int i = 1;
char buff [32];
sprintf(buff, "test: 2^%d = %lu", i, long(2<<i));
log(buff); // prints "test 2^1 = 2" ...etc
*/


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define NLINES 4
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int cursor_pos;
int line_no; // print linenumbers. loops @0xff

void initLogger() {
  Serial.println("Logger initialized.");
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  cursor_pos = 0;
  line_no = 0;
  display.clearDisplay();
}

void log(char* s) {
  display.setTextSize(1);                           // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);              // Draw white text
  display.setCursor(0, (cursor_pos % NLINES) * 8);  // Start at top-left corner

  // clear backing area
  for (int y = 0; y < 8; y++) {
    display.drawFastHLine(0, (cursor_pos % NLINES) * 8 + y, 128, 0);
  }
  // display text
  display.cp437(true);  // Use full 256 char 'Code Page 437' font

  // write linenumber
  char buff_lineno[4];
  if (line_no < 0x10) {
    sprintf(buff_lineno, "0%x|", line_no);
  } else {
    sprintf(buff_lineno, "%x|", line_no);
  }

  for (int i = 0; i < 3; i++) {
    display.write(buff_lineno[i]);
  }

  // write actual text
  for (int i = 0; i < strlen(s); i++) {
    display.write(s[i]);
  }

  display.display();
  cursor_pos++;
  line_no++;
  if(line_no > 0xff) line_no = 0;
}
