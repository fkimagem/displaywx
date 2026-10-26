// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3
    * Display: PDC8544
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 84;
const int DISPLAY_H = 48;
const int DISP_DIN = 11;
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

Adafruit_PCD8544 *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    tft = new Adafruit_PCD8544(DISP_SCLK, DISP_DIN, DISP_DC, DISP_CS, DISP_RST);
    tft->begin();
    tft->setContrast(75);
    tft->clearDisplay();
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

}

void screen0(){

    tft->clearDisplay();
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(30, 5, 19, 15, CFK_COLOR12);
    tft->drawRect(35, 10, 19, 15, CFK_COLOR01);
    myDisplay.printText("Custom", 5, 22, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    tft->display();
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}
