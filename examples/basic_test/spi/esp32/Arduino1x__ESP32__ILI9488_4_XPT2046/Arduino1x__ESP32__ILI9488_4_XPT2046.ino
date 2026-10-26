// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32
    * Display: ILI9488
    * Touch: XPT2046
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_FREQUENCY = 2500000;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = false;
const int DISP_MOSI = 23;
const int DISP_MISO = 19;
const int DISP_SCLK = 18;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const int TOUCH_CS = 21;
const int TOUCH_SCL = 18;
const int TOUCH_MOSI = 23;
const int TOUCH_MISO = 19;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
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
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, isIPS);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
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
    tft->fillCircle(114, 94, 15, CFK_COLOR01);
    tft->drawCircle(114, 94, 15, CFK_BLACK);
    tft->fillCircle(90, 107, 15, CFK_COLOR11);
    tft->drawCircle(90, 107, 15, CFK_BLACK);
    tft->fillCircle(90, 80, 15, CFK_COLOR21);
    tft->drawCircle(90, 80, 15, CFK_BLACK);
    tft->fillCircle(106, 107, 10, CFK_COLOR06);
    tft->drawCircle(106, 107, 10, CFK_BLACK);
    tft->fillCircle(82, 94, 10, CFK_COLOR17);
    tft->drawCircle(82, 94, 10, CFK_BLACK);
    tft->fillCircle(105, 80, 10, CFK_COLOR28);
    tft->drawCircle(105, 80, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 50, 157, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


