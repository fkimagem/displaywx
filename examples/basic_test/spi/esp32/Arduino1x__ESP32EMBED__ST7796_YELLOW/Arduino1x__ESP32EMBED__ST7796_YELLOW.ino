// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32_EMBEDDED
    * Display: None
    * Touch: XPT2046
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int TOUCH_FREQUENCY = 2500000;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const int CS = -1;
const int CLK = -1;
const int MOSI = -1;
const int MISO = -1;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

// Create global SPI object
#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif

void setup(){

    Serial.begin(115200);
    // Start SPI object for touch
    spi_shared.begin(TOUCH_SCL, TOUCH_MISO, TOUCH_MOSI);
    // No display selected or not supported yet
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SCL, TOUCH_MOSI, TOUCH_MISO, TOUCH_CS, tft, TOUCH_FREQUENCY);
    //myDisplay.enableTouchLog();
    //myDisplay.recalibrate();
    myDisplay.checkCalibration();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillCircle(62, 39, 15, CFK_COLOR01);
    tft->drawCircle(62, 39, 15, CFK_BLACK);
    tft->fillCircle(38, 53, 15, CFK_COLOR11);
    tft->drawCircle(38, 53, 15, CFK_BLACK);
    tft->fillCircle(38, 25, 15, CFK_COLOR21);
    tft->drawCircle(38, 25, 15, CFK_BLACK);
    tft->fillCircle(54, 53, 10, CFK_COLOR06);
    tft->drawCircle(54, 53, 10, CFK_BLACK);
    tft->fillCircle(30, 39, 10, CFK_COLOR17);
    tft->drawCircle(30, 39, 10, CFK_BLACK);
    tft->fillCircle(54, 25, 10, CFK_COLOR28);
    tft->drawCircle(54, 25, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 100, 32, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


