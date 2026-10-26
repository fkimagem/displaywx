// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32_EMBEDDED
    * Display: ST7789 - EMBED
    * Touch: CST820
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 320;
const int DISPLAY_H = 240;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 240;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_MOSI = 13;
const int DISP_MISO = 12;
const int DISP_SCLK = 14;
const int DISP_CS = 15;
const int DISP_DC = 2;
const int DISP_RST = -1;
const int DISP_LED = 27;
const int TOUCH_SCL = 32;
const int TOUCH_SDA = 33;
const int TOUCH_INT = 21;
const int TOUCH_RST = 25;
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
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchCST820(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
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
    tft->fillCircle(72, 85, 15, CFK_COLOR01);
    tft->drawCircle(72, 85, 15, CFK_BLACK);
    tft->fillCircle(48, 98, 15, CFK_COLOR11);
    tft->drawCircle(48, 98, 15, CFK_BLACK);
    tft->fillCircle(47, 71, 15, CFK_COLOR21);
    tft->drawCircle(47, 71, 15, CFK_BLACK);
    tft->fillCircle(64, 98, 10, CFK_COLOR06);
    tft->drawCircle(64, 98, 10, CFK_BLACK);
    tft->fillCircle(40, 85, 10, CFK_COLOR17);
    tft->drawCircle(40, 85, 10, CFK_BLACK);
    tft->fillCircle(63, 71, 10, CFK_COLOR28);
    tft->drawCircle(63, 71, 10, CFK_BLACK);
    myDisplay.printText("ard", 65, 172, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}
