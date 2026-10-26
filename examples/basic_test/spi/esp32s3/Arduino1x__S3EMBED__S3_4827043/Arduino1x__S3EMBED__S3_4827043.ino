// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3_EMBEDDED
    * Display: ESP32-4827S043
    * Touch: XPT2046
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 272;
const int TOUCH_FREQUENCY = 2500000;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 480;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 272;
const bool TOUCH_SWAP_XY = false;
const int TOUCH_CS = 38;
const int TOUCH_SCL = 12;
const int TOUCH_MOSI = 11;
const int TOUCH_MISO = 13;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

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
#define DISP_LED 2
Arduino_ESP32RGBPanel *bus = nullptr;
Arduino_RGB_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    Serial.begin(115200);
    // Start SPI object for touch
    spi_shared.begin(TOUCH_SCL, TOUCH_MISO, TOUCH_MOSI);
    bus = new Arduino_ESP32RGBPanel(
    40 /* DE */, 41 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    45 /* R0 */, 48 /* R1 */, 47 /* R2 */, 21 /* R3 */, 14 /* R4 */,
    5 /* G0 */, 6 /* G1 */, 7 /* G2 */, 15 /* G3 */, 16 /* G4 */, 4 /* G5 */,
    8 /* B0 */, 3 /* B1 */, 46 /* B2 */, 9 /* B3 */, 1 /* B4 */,
    0 /* hsync_polarity */, 1 /* hsync_front_porch */, 1 /* hsync_pulse_width */, 43 /* hsync_back_porch */,
    0 /* vsync_polarity */, 3 /* vsync_front_porch */, 1 /* vsync_pulse_width */, 12 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 10000000 /* prefer_speed */, isIPS /* useBigEndian */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, 0 /* bounce_buffer_size_px */);
    tft = new Arduino_RGB_Display(
    480, 272, bus,
    rotationScreen, true);
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
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_CS, &spi_shared, tft, TOUCH_FREQUENCY);
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
    myDisplay.printText("Custom text", 100, 35, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    tft->fillCircle(46, 36, 15, CFK_COLOR01);
    tft->drawCircle(46, 36, 15, CFK_BLACK);
    tft->fillCircle(32, 44, 15, CFK_COLOR11);
    tft->drawCircle(32, 44, 15, CFK_BLACK);
    tft->fillCircle(32, 28, 15, CFK_COLOR21);
    tft->drawCircle(32, 28, 15, CFK_BLACK);
    tft->fillCircle(41, 44, 10, CFK_COLOR06);
    tft->drawCircle(41, 44, 10, CFK_BLACK);
    tft->fillCircle(27, 36, 10, CFK_COLOR17);
    tft->drawCircle(27, 36, 10, CFK_BLACK);
    tft->fillCircle(41, 28, 10, CFK_COLOR28);
    tft->drawCircle(41, 28, 10, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}
