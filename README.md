# TeensyRA8876-SPI
## A RA8876 SPI DRIVER  library for Teensy boards and variants.
## CHANGES:
09-07-26:
- Reworked "graphicCursor.ino" sketch. Installed new process_mouse() function for processing mouse delta X and delta Y scaling as well as processing mouse button single, double clicks and object dragging with mouse pointer.


This library is designed to be used with the TeensyRa8876-GFX-Common library and can be found here:
- https://github.com/wwatson4506/TeensyRA8876-GFX-Common

Communication with the Teensy is accomplished using SPI.

![https://github.com/wwatson4506/TeensyRA8876-8080/blob/main/extras/MEM_Transfer.jpg](https://github.com/wwatson4506/TeensyRA8876-8080/blob/main/extras/MEM_Transfer.jpg)

## TOUCH SCREEN
The capacitive touch controller now used on the ER-TFTM101-1 is the Goodix GT9271. I2C communication is used with the GT9371 controller.
A minimal driver adapted from the arduino-goodix library can be found here: 
- https://github.com/wwatson4506/Arduino-goodix-GT9271
***

## LIBRARY INSTALLATION
1. Unzip this library to the **Arduino/libraries** folder.
2. Unzip **TeensyRA8876-GFX-Common**  library to the **Arduino/libraries** folder.
3. Download **ILI9341_fonts** [https://github.com/wwatson4506/ILI9341_fonts](https://github.com/wwatson4506/ILI9341_fonts) and install into the **Arduino/libraries** folder.
***

# PINOUTS

### CONNECTING THE TEENSY TO THE ER-TFTM101-1 in SPI 4 wire  mode:
40 pin dual inline connector pinouts for SPI buss mode can be found here:

https://www.buydisplay.com/download/interfacing/ER-TFTM101-1_CTP_Interfacing.pdf

### Teensy 4.1 <--------------> RA8876
### Dev BRD 5 (Same)
```
SPI usage.
PIN                      PIN
- CS   10 -------------------> Pin 05 SCS
- SCK  13 -------------------> Pin 08 SCLK
- MOSI 11 -------------------> Pin 07 SDI
- MISO 12 -------------------> Pin 06 SDO
- RST  09 -------------------> Pin 11 RESET
- BL  3.3V (BACKLITE) -------> 14 or I/O pin.
Power and Grounds
- TFT 5V --------------------> 3,4,37,38
- GND -----------------------> 1,2,13,31,39,40
NOTE: All power and ground pins should be connected.

Touch Screen (ER_TFTM101-1 40 pin dual inline connector)
  Teensy 4.1            RA8876
- 3.3V ---------------> CPT_/RST 36
- 28   ---------------> CPT_INT  33
- 25   ---------------> CPT_SDA  34
- 24   ---------------> CPT_SCL  35

```
***
### CONFIG FILE
Both SPI Parallel and 8080 libraries have config file.
Config file for SPI:
```
/* RA8876_Config_SPI.h
 A file to place user defines and configs.
*/

#ifndef RA8876_CONFIG_SPI_H
#define RA8876_CONFIG_SPI_H

// SPI hardware settings
#define USE_SPI  // Needed for writeRect() in RA8876_GFX
#define RA8876_CS 10
#define RA8876_RESET 9

// Example usage in sketch: RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins

// Uncomment define below to use 47MHz SPI clock. Default is 30MHz.
// We start off slow in case of long connecting wires. If having problems,
// the SPI clock speed can be reduced further.
//
// #define USE_SPI_47000000
//

// External backlight control connected to this Arduino pin. Can be controlled with PWM.
// Otherwise 3.3v
// Un-comment one of these defines for pin control of backlite.
//#define BACKLITE 5 //External backlight control connected to this Arduino pin
//#define BACKLITE 29 // Kurt's DB5 shield

// Uncomment to use FT5206 touch. (Not used on the ER-TFTM1010-1)
#define USE_FT5206_TOUCH

#endif // RA8876_CONFIG_H
```

### MINIMAL SKETCH EXAMPLE
```
// sketch.ino

#include "Arduino.h"
#include "RA8876_Config_SPI.h"
#include <RA8876_t3.h>
#include <SPI.h>

// RA8876_CS and RA8876_RESET are defined in
// src/RA8876_Config_SPI.h.
RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {} //wait for Serial Monitor (3 seconds).

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif
  ... // Rest of user setup code.
}

void loop() {
 ... // Users loop code.
}

```

***

## Examples Folder Listings:

Example sketches can be found in the TeensyRA8876-8080 and TeensyRA8876-SPI examples folders.

- gauges  ---------------------->  A Sumotoy example originally created for the RA8875.
- graphicCursor ------------->  A demonstration of defining and using a graphical mouse pointer. A USB mouse is required. Also demonstrates single and double mouse button clicks.
- graphics --------------------->  Demonstrates common graphics usage, lines, circles, rectangles and more.
- ILI_ADA_FontTest4 -------> Demonstrates usage of ILI9341 and Adafruit fonts on the RA8876.
- MemoryTransfer -----------> Shows usage of many BTE (Block Transfer Engine) functions.
- pipTest ------------------------> Example of PIP (Picture In Picture) usage.
- RA8876_pictureEmbed --> Displays 16Bit color images. Also demonstrates rotation.
- RA8876Rotate --------------> Also demontrates use of rotation.
- scroll ---------------------------> Simple demonstration of scrolliing screen up and down.
- treedee ------------------------> Demonstrates a spinning 3D wire cube.
- UserDefinedFonts ----------> Demonstrates loading  user define fonts into pattern ram.   Fonts can also be loaded from an SD card or USB drive.


***
# CREDITS
Major work on this library was done by the following PJRC forum members:

@mjs513, @KurtE, @MorganS, @rezo and @K7MDL (Keith).

They added functions to be compatible with other display libraries, performed a major rework and helped in debugging the code. The 8080 parallel FlexIO driver is based on work done by @rezo.

***

# REFERENCES
ER-TFTM-101-1 10.1" TFT from BuyDisplay:
- https://www.buydisplay.com/serial-spi-i2c-10-1-inch-tft-lcd-module-dislay-w-ra8876-optl-touch-panel
- https://www.buydisplay.com/download/ic/RA8876.pdf

PJRC Forum Threads:
- https://forum.pjrc.com/threads/58565-RA8876LiteTeensy-For-Teensy-T36-and-T40
- https://forum.pjrc.com/index.php?threads/ra8876-parallel-display-library-testing.75345/
- https://forum.pjrc.com/index.php?threads/recommendations-for-10-tft-display-with-touchscreen-for-teensy-4-1.75666/
***

# This is WIP.   USE AT YOUR OWN RISK.  There are no guarantees when using this library. More to come.
