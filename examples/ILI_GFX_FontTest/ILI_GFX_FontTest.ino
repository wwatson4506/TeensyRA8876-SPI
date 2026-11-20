#include <Adafruit_GFX.h>
#include "RA8876_Config_SPI.h"
#include <SPI.h>
#include <RA8876_t3.h>
#include <_font_ComicSansMS.h>
#include "Fonts/FreeSansOblique12pt7b.h"

RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins

void setup() {
  //I'm guessing most copies of this display are using external PWM
  //backlight control instead of the internal RA8876 PWM.
  //Connect a Teensy pin to pin 14 on the display.
  //Can use analogWrite() but I suggest you increase the PWM frequency first so it doesn't sing.
#if defined(BACKLITE) // Defined in RA8876_Config.h.
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);
#endif

  Serial.begin(115200);
  while (!Serial && millis() < 1000) {} //wait for Serial Monitor

  Serial.println("Simple Fonts");

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif

  tft.fillScreen(COLOR65K_BLACK);

  tft.setTextColor(COLOR65K_WHITE);
  tft.println("Test of the default font");
  tft.println();

  tft.setTextColor(COLOR65K_WHITE, COLOR65K_BLUE);
  tft.setFont(ComicSansMS_18);
  tft.println("Opaque ILI font ComicSansMS_18");
  tft.println();

  int cursor_x = tft.getCursorX();
  int cursor_y = tft.getCursorY();
  tft.fillRect(cursor_x, cursor_y, tft.width(), 20, COLOR65K_RED);
  tft.setTextColor(COLOR65K_GREEN);
  tft.println("Transparent ILI ComicSansMS_18");
  tft.println();

  tft.setTextColor(COLOR65K_WHITE, COLOR65K_RED);
  tft.setFont(&FreeSansOblique12pt7b);
  tft.println("Opaque GFX FreeSansOblique12pt");

  cursor_x = tft.getCursorX();
  cursor_y = tft.getCursorY();
  tft.fillRect(cursor_x, cursor_y, tft.width(), 20, COLOR65K_YELLOW);

  tft.setTextColor(COLOR65K_BLUE);
  tft.println("Transparent GFX");

  tft.setFontDef();
  tft.setTextColor(COLOR65K_GREEN);
  tft.setFontSize(1, true);
  tft.println("This is default font:");
  tft.println("ABCDEF 1 2 3 4 5 6 7");

}

void loop()
{  }
