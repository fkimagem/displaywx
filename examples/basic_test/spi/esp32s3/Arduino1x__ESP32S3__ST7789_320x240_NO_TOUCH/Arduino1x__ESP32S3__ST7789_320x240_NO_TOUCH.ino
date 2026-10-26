// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3
    * Display: ST7789 - SPI
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 240;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
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

    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Custom text", 90, 55, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    tft->fillCircle(68, 67, 15, CFK_COLOR01);
    tft->drawCircle(68, 67, 15, CFK_BLACK);
    tft->fillCircle(44, 80, 15, CFK_COLOR11);
    tft->drawCircle(44, 80, 15, CFK_BLACK);
    tft->fillCircle(43, 53, 15, CFK_COLOR21);
    tft->drawCircle(43, 53, 15, CFK_BLACK);
    tft->fillCircle(60, 80, 10, CFK_COLOR06);
    tft->drawCircle(60, 80, 10, CFK_BLACK);
    tft->fillCircle(36, 67, 10, CFK_COLOR17);
    tft->drawCircle(36, 67, 10, CFK_BLACK);
    tft->fillCircle(59, 53, 10, CFK_COLOR28);
    tft->drawCircle(59, 53, 10, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


