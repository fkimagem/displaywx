// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: JD9365 - DSI
    * Touch: GSL3680
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1280;
const int DISPLAY_H = 800;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

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
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    60000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    800 /* width default */, 1280 /* height default */, bus, rotationScreen, true,
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
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
    myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
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
    tft->fillCircle(54, 36, 15, CFK_COLOR01);
    tft->drawCircle(54, 36, 15, CFK_BLACK);
    tft->fillCircle(30, 49, 15, CFK_COLOR11);
    tft->drawCircle(30, 49, 15, CFK_BLACK);
    tft->fillCircle(29, 22, 15, CFK_COLOR21);
    tft->drawCircle(29, 22, 15, CFK_BLACK);
    tft->fillCircle(46, 49, 10, CFK_COLOR06);
    tft->drawCircle(46, 49, 10, CFK_BLACK);
    tft->fillCircle(22, 36, 10, CFK_COLOR17);
    tft->drawCircle(22, 36, 10, CFK_BLACK);
    tft->fillCircle(45, 22, 10, CFK_COLOR28);
    tft->drawCircle(45, 22, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 115, 32, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


