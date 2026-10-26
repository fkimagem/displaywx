// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7701 - DSI
    * Touch: GT911
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 800;
const int DISPLAY_H = 480;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 5;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
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
    12  /* hsync_pulse_width */,  42  /* hsync_back_porch */,  42  /* hsync_front_porch */,
    2   /* vsync_pulse_width */, 8   /* vsync_back_porch */,  166 /* vsync_front_porch */,
    34000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    480 /* width default */, 800 /* height default */, bus, rotationScreen, true,
    DISP_RST, st7701_init_operations, sizeof(st7701_init_operations) / sizeof(lcd_init_cmd_t));
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
    tft->fillCircle(70, 51, 15, CFK_COLOR01);
    tft->drawCircle(70, 51, 15, CFK_BLACK);
    tft->fillCircle(46, 64, 15, CFK_COLOR11);
    tft->drawCircle(46, 64, 15, CFK_BLACK);
    tft->fillCircle(45, 37, 15, CFK_COLOR21);
    tft->drawCircle(45, 37, 15, CFK_BLACK);
    tft->fillCircle(62, 64, 10, CFK_COLOR06);
    tft->drawCircle(62, 64, 10, CFK_BLACK);
    tft->fillCircle(38, 51, 10, CFK_COLOR17);
    tft->drawCircle(38, 51, 10, CFK_BLACK);
    tft->fillCircle(61, 37, 10, CFK_COLOR28);
    tft->drawCircle(61, 37, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 115, 42, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


