// graphicsCursor.ino

/*
* This sketch uses a USB mouse and USBhost_t36 to control the RA8876 graphics
* cursor. It initializes the graphis cursor memory with four different
* cursor shapes. These are defined in Ra8876_Lite and can be changed. 
* tft.Select_Graphic_Cursor_1();  Selects Pen Cursor.
* tft.Select_Graphic_Cursor_2();  Selects Arrow Cursor.
* tft.Select_Graphic_Cursor_3();  Selects Hour Glass (Busy) Cursor.
* tft.Select_Graphic_Cursor_4();  Selects Error (Stop) Cursor.
* These graphic cursors have two color setting, Foreground color and
* Outline color.
* I have included a few experimental routines of my own for detecting
* single click, double click and dragging of mouse buttons.
*
*/

#include "USBHost_t36.h"
#include "RA8876_Config_SPI.h"
#include <SPI.h>
#include <RA8876_t3.h>

RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);

//************************************************************
// Even though the keyboard is not used we still need
// to create a keyboardController and USBHIDParser instance
// just in case a wireless keyboard/mouse is used. Otherwise
// the mouse will not be claimed.
//************************************************************* 
KeyboardController keyboard1(myusb); // Not used.
KeyboardController keyboard2(myusb);
MouseController mouse1(myusb);
USBHIDParser hid1(myusb); // Needed for USB mouse.
USBHIDParser hid2(myusb); // Needed for use with wireless keyboard/mouse combo.

const char *button[] = {"IDLE        ",
	                    "DRAG STARTED",
	                    "DRAGGING    ",
	                    "DRAG_ENDED  ",
	                    "SINGLE_CLICK",
	                    "DOUBLE_CLICK"}; 

// Button states 
enum  { IDLE = 0, DRAG_INITIATED, DRAGGING, DRAG_RELEASED, SINGLE_CLICK, DOUBLE_CLICK };

// A structure to hold results of mouse operations.
// Some are not used in this sketch.
struct usbMouseMsg_struct {
  bool isButtonDown = false; 
  bool lastButtonState = false;
  bool isDragging = false;
  unsigned long lastClickTime = 0;
  bool expectingSecondClick = false;
  uint8_t buttons;
  uint8_t button_state = IDLE;
  uint8_t scCount = 0;
  uint8_t dcCount = 0;
  int accumulatedX = 0;
  int accumulatedY = 0;
  int16_t scaledX;
  int16_t scaledY;
  int8_t wheel;
  int8_t wheelH;
  bool mouseEvent;
};

usbMouseMsg_struct mouse_msg;

// Configuration Constants
const unsigned long DOUBLE_CLICK_WINDOW = 350; // Maximum time between clicks (ms) 
const int DRAG_THRESHOLD = 2; // Prevent micro-movements/shaking from falsely starting a drag

// Scale mouse XY to fit our screen (1023x599).
void scaleMouseXY(void) {
  if(!mouse1.available()) return; // No sense hanging around here!!
  mouse_msg.scaledX += (int16_t)mouse1.getMouseX();
  mouse_msg.scaledY += (int16_t)mouse1.getMouseY();
  if(mouse_msg.scaledX < 0)
    mouse_msg.scaledX = 0;
  if(mouse_msg.scaledX > (uint16_t)1023)
    mouse_msg.scaledX = (uint16_t)1023;
  if(mouse_msg.scaledY < 0)
    mouse_msg.scaledY = 0;
  if(mouse_msg.scaledY > (uint16_t)599)
    mouse_msg.scaledY = (uint16_t)599;
}

// Check for mouse button presses
uint8_t getMouseButtons(void) {
  mouse_msg.buttons = (uint8_t)mouse1.getButtons();
  return mouse_msg.buttons;
}

// Process mouse buttons
uint8_t process_mouse(uint8_t button_num) {
  scaleMouseXY();
  mouse_msg.wheel += (int8_t)mouse1.getWheel(); // Check for wheel movement
  mouse_msg.wheelH += (int8_t)mouse1.getWheelH();
  mouse_msg.isButtonDown = button_num;
  // Extract relative movement deltas
  int16_t deltaX = mouse1.getMouseX();
  int16_t deltaY = mouse1.getMouseY();
  // 1. Edge Detection: BUTTON PRESSED ---
  if(mouse_msg.isButtonDown && !mouse_msg.lastButtonState) {
    unsigned long currentTime = millis();
    mouse_msg.accumulatedX = 0;
    mouse_msg.accumulatedY = 0;
    mouse_msg.isDragging = false;
    if(mouse_msg.expectingSecondClick) {
      if((currentTime - mouse_msg.lastClickTime) <= DOUBLE_CLICK_WINDOW) {
        mouse_msg.button_state = DOUBLE_CLICK;
        mouse_msg.dcCount++;
        mouse_msg.expectingSecondClick = false; // Reset sequence
      } else {
          mouse_msg.lastClickTime = currentTime;
      }
    } else {
      mouse_msg.expectingSecondClick = true;
      mouse_msg.lastClickTime = currentTime;
    }
  }
  // 2. State Processing: BUTTON HELD DOWN & MOUSE MOVING ---
  if(mouse_msg.isButtonDown) {
    mouse_msg.accumulatedX += deltaX;
    mouse_msg.accumulatedY += deltaY;
    // Check if movement exceeds the deadzone threshold to start or continue a drag
    if(abs(mouse_msg.accumulatedX) > DRAG_THRESHOLD || abs(mouse_msg.accumulatedY) > DRAG_THRESHOLD) {
      // If this is the exact moment the drag starts, suppress pending single clicks
      if(!mouse_msg.isDragging) {
        mouse_msg.isDragging = true;
        mouse_msg.expectingSecondClick = false; // Dragging invalidates an incoming click event
        mouse_msg.button_state = DRAG_INITIATED;
      }
      // Call the active drag step with current motion deltas
        mouse_msg.button_state = DRAGGING;
    }
  }
  // --- 3. Edge Detection: BUTTON RELEASED ---
  if(!mouse_msg.isButtonDown && mouse_msg.lastButtonState) {
    if(mouse_msg.isDragging) {
      mouse_msg.isDragging = false;
      mouse_msg.button_state = IDLE;
    }
  }
  // Save state for transition history tracking
  mouse_msg.lastButtonState = mouse_msg.isButtonDown;
  // 4. Asynchronous Click Expiration Window ---
  // If the button was pressed, released, and no dragging or second click happens:
  if(mouse_msg.expectingSecondClick && !mouse_msg.isButtonDown && (millis() - mouse_msg.lastClickTime > DOUBLE_CLICK_WINDOW)) {
    mouse_msg.button_state = SINGLE_CLICK;
    mouse_msg.scCount++;
    mouse_msg.expectingSecondClick = false; 
  }
  return mouse_msg.button_state;	
}

void setup() {
  //I'm guessing most copies of this display are using external PWM
  //backlight control instead of the internal RA8876 PWM.
  //Connect a Teensy pin to pin 14 on the display.
  //Can use analogWrite() but I suggest you increase the PWM frequency first so it doesn't sing.
#if defined(BACKLITE) // Defined in RA8876_Config.h.
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);
#endif
  Serial.begin(9600);
  while (!Serial && millis() < 1000) {} //wait for Serial Monitor
  myusb.begin();
  mouse_msg.scaledX = 512;
  mouse_msg.scaledY = 300;

  Serial.println("USB Mouse and Graphic Cursor Testing");

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif

  tft.fillScreen(DARKBLUE);
  tft.setFontSize(1, false);
  tft.setCursor(0,0);
  tft.setTextColor(YELLOW, DARKBLUE);
  tft.print("USB Mouse and Graphic Cursor Testing");

  tft.Graphic_cursor_initial(); // Initialize all 4 cursors.
  tft.Select_Graphic_Cursor_2(); // Select Arrow Cursor.
  tft.Enable_Graphic_Cursor(); // Turn on selected graphic cursor.
  // You can play with the cursor colors in the following two lines.
  tft.Set_Graphic_Cursor_Color_1(0xff); // White forground Color. (0 - 255)
  tft.Set_Graphic_Cursor_Color_2(0x00); // Black outline Color. (0 - 255)
  tft.Graphic_Cursor_XY(mouse_msg.scaledX, mouse_msg.scaledY); // Center cursor on screen.
  tft.drawSquareFill(300,100,800,500,WHITE);
  tft.setTextColor(GREEN, DARKBLUE);

}

void display_mouse_data(void) {
    tft.Graphic_Cursor_XY(mouse_msg.scaledX, mouse_msg.scaledY); // Position cursor on screen
    tft.textxy(0,5);
    tft.printf("      Mouse X: %4d\n", mouse_msg.scaledX);
    tft.printf("      Mouse Y: %4d\n", mouse_msg.scaledY);
    tft.printf("      Buttons: %4d\n", getMouseButtons());
    tft.printf("        Wheel: %4d\n", mouse_msg.wheel);
    tft.printf("       WheelH: %4d\n", mouse_msg.wheelH);
    tft.printf("Single Clicks: %4d\n", mouse_msg.scCount);
    tft.printf("Double Clicks: %4d\n", mouse_msg.dcCount);
    tft.printf("Button State: %s\n",button[mouse_msg.button_state]);
}

void loop() {
  myusb.Task();
  display_mouse_data();
  process_mouse(mouse_msg.buttons);
  mouse1.mouseDataClear(); 

}
