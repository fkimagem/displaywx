// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32
    * Display: GC9A01
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 240;
const int DISPLAY_H = 240;
const int DISP_FREQUENCY = 27000000;
const int DISP_MOSI = 23;
const int DISP_MISO = -1;
const int DISP_SCLK = 18;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
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
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, GFX_NOT_DEFINED /* MISO não utilizado */, &spi_shared);
    tft = new Arduino_GC9A01(bus, DISP_RST, rotationScreen, isIPS); // bus,reset pin, rotation, ips
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
    tft->fillCircle(77, 69, 15, CFK_COLOR01);
    tft->drawCircle(77, 69, 15, CFK_BLACK);
    tft->fillCircle(53, 83, 15, CFK_COLOR11);
    tft->drawCircle(53, 83, 15, CFK_BLACK);
    tft->fillCircle(53, 55, 15, CFK_COLOR21);
    tft->drawCircle(53, 55, 15, CFK_BLACK);
    tft->fillCircle(69, 83, 10, CFK_COLOR06);
    tft->drawCircle(69, 83, 10, CFK_BLACK);
    tft->fillCircle(45, 69, 10, CFK_COLOR17);
    tft->drawCircle(45, 69, 10, CFK_BLACK);
    tft->fillCircle(69, 55, 10, CFK_COLOR28);
    tft->drawCircle(69, 55, 10, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}
