// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: JD9165 - DSI
    * Touch: GT911
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1024;
const int DISPLAY_H = 600;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 1024;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 600;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = 22;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    Serial.begin(115200);
    bus = new Arduino_ESP32DSIPanel(
    40 /* hsync_pulse_width */, 160 /* hsync_back_porch */, 160 /* hsync_front_porch */,
    10 /* vsync_pulse_width */, 23 /*vsync_back_porch  */, 12 /* vsync_front_porch */,
    48000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    1024 /* width default */, 600 /* height default */, bus, rotationScreen, true,
    DISP_RST, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillCircle(59, 58, 15, CFK_COLOR01);
    tft->drawCircle(59, 58, 15, CFK_BLACK);
    tft->fillCircle(35, 71, 15, CFK_COLOR11);
    tft->drawCircle(35, 71, 15, CFK_BLACK);
    tft->fillCircle(34, 44, 15, CFK_COLOR21);
    tft->drawCircle(34, 44, 15, CFK_BLACK);
    tft->fillCircle(51, 71, 10, CFK_COLOR06);
    tft->drawCircle(51, 71, 10, CFK_BLACK);
    tft->fillCircle(27, 58, 10, CFK_COLOR17);
    tft->drawCircle(27, 58, 10, CFK_BLACK);
    tft->fillCircle(50, 44, 10, CFK_COLOR28);
    tft->drawCircle(50, 44, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 100, 47, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


