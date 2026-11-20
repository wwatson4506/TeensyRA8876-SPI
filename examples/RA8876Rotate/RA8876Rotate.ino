#include "RA8876_Config_SPI.h"
#include <SPI.h>
#include <RA8876_t3.h>
#include "font_Arial.h"

RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins

#define PINK        0xFC18
#define REG_DUMP_CNT  0 //0x70
#define USE_STATUS_LINE

uint8_t reg_values[REG_DUMP_CNT];

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

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif

  Serial.println("Rotate Sreen Test");

  for (uint8_t reg = 0; reg < REG_DUMP_CNT; reg++) {
    reg_values[reg] = tft.lcdRegDataRead(reg);
  }

#if defined(BACKLITE)
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);
  tft.backlight(true);
#endif

  tft.graphicMode(true);
  Serial.printf("Before W: %d H: %d\n", tft.width(), tft.height());
  tft.fillScreen(COLOR65K_LIGHTYELLOW);
  drawTestScreen();
}

void drawTestScreen() {
#ifdef USE_STATUS_LINE
  int image_height = tft.height() - STATUS_LINE_HEIGHT; 
#else
  int image_height = tft.height()
#endif

  int centerx = tft.width() / 2;
  int centery = image_height / 2;
  tft.fillRect(tft.width() - 50, 0, 50, 50, COLOR65K_GREEN);
  tft.fillRect(tft.width() - 50, image_height - 50 , 50, 50, COLOR65K_BLUE);
  tft.fillRect(0, image_height - 50 , 50, 50, COLOR65K_ORCHID);
  tft.drawLine(0, 0, tft.width(), image_height, COLOR65K_WHITE);
  tft.drawLine(0, image_height, tft.width(), 0, COLOR65K_BLACK);
  tft.drawRect(centerx - 50, centery / 2, 100, 50, COLOR65K_BLUE);
  tft.fillRect(centerx - 40, centery / 2 + 10, 80, 30, COLOR65K_GREEN);
  tft.drawLine(centerx - 50, centery / 2, centerx, 0, COLOR65K_RED);
  tft.drawLine(centerx + 50, centery / 2, centerx, 0, COLOR65K_CRIMSON);
  tft.drawLine(centerx, centery / 2, centerx, 0, COLOR65K_GREEN);
  tft.drawTriangleFill(centerx, centery, centerx - 100, centery + 100, centerx + 100, centery + 100, COLOR65K_CRIMSON);
  tft.fillRect(0, 0, 50, 50, COLOR65K_RED);
  tft.fillCircle(centerx, centery, 50, COLOR65K_ORCHID);
  tft.drawCircle(centerx, centery, 25, COLOR65K_BLACK);
  tft.drawTriangle(50, 50, 50, image_height - 50, centerx - 50, centery, COLOR65K_BLUE);
  tft.fillRoundRect(100, centery - 50, 150, 100, 25, 25, COLOR65K_GREEN);
  tft.drawRoundRect(110, centery - 40, 130, 80, 25, 25, COLOR65K_WHITE);
  tft.drawEllipse(tft.width() - 100, centery, 50, 35, COLOR65K_BLUE);
  tft.drawEllipseFill(tft.width() - 100, centery, 40, 25, COLOR65K_RED);

  tft.setFont(Arial_14);
  tft.setTextColor(COLOR65K_BLACK);
  tft.setCursor(25, centery);
  tft.printf("Rotation: %d", tft.getRotation());
  tft.setCursor(50, 50);
  tft.write('0');
  tft.setCursor(tft.width() - 50, 50);
  tft.write('1');
  tft.setCursor(50, image_height - 50);
  tft.write('2');
  tft.setCursor(tft.width() - 50, image_height - 50);
  tft.write('3');

  tft.printStatusLine(0, COLOR65K_RED, COLOR65K_GREEN, "Status Text");
#if REG_DUMP_CNT > 0
  Serial.print("LCD Register dump:");
  for (uint8_t reg = 0; reg < REG_DUMP_CNT; reg++) {
    uint8_t r_value = tft.lcdRegDataRead(reg);
    if ((reg & 0xf) == 0) Serial.printf("\n%02x -", reg);
    Serial.printf(" %c%02x",
                  (r_value == reg_values[reg]) ? ' ' : '*',
                  r_value);
    reg_values[reg] = r_value;
  }
  Serial.print("\n");
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
  wait_for_keyboard();  // see if all yellow
  uint8_t rotation = (tft.getRotation() + 1) & 0x3;
  tft.setRotation(rotation);

  Serial.printf("Rotation: %d After W: %d H: %d\n", tft.getRotation(), tft.width(), tft.height());
  tft.fillScreen(COLOR65K_YELLOW);
  //wait_for_keyboard();  // see if all yellow
  drawTestScreen();
  //tft.textRotate(false);
  Serial.printf("MACR and End: %x\n", tft.lcdRegDataRead(RA8876_MACR));

}
void wait_for_keyboard() {
  Serial.println("Wait for any key to continue");
  while (Serial.read() == -1);
  while (Serial.read() != -1);
}
