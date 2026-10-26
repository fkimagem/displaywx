// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>

    /* Project setup:
    * MCU: ESP32S3_EMBEDDED
    * Display: ST7789 - 8bit
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 170;
const int DISPLAY_H = 320;
const int DISP_BL = 38;
const int DISP_DC = 7;
const int DISP_CS = 6;
const int DISP_WR = 8;
const int DISP_RD = 9;
const int DISP_D0 = 39;
const int DISP_D1 = 40;
const int DISP_D2 = 41;
const int DISP_D3 = 42;
const int DISP_D4 = 45;
const int DISP_D5 = 46;
const int DISP_D6 = 47;
const int DISP_D7 = 48;
const int DISP_PWD = 15;
const int DISP_RST = 5;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag
// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
// Create global SPI object
#if defined(CONFIG_IDF_TARGET_ESP32S3)
//SPIClass spi_shared(FSPI);
#else
//SPIClass spi_shared(HSPI);
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
    // spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    pinMode(DISP_PWD, OUTPUT);
    digitalWrite(DISP_PWD, HIGH);
    
    pinMode(DISP_BL, OUTPUT);
    digitalWrite(DISP_BL, HIGH);
    
    bus = new Arduino_ESP32PAR8Q(DISP_DC, DISP_CS, DISP_WR, DISP_RD, DISP_D0, DISP_D1, DISP_D2, DISP_D3, DISP_D4, DISP_D5, DISP_D6, DISP_D7);
    tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS, 170, 320, 35, 0, 35, 0);
    tft->begin();
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}
void loop(){
  Serial.println(millis());
delay(500);

}
void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillCircle(70, 232, 15, CFK_COLOR01);
    tft->drawCircle(70, 232, 15, CFK_BLACK);
    tft->fillCircle(46, 245, 15, CFK_COLOR11);
    tft->drawCircle(46, 245, 15, CFK_BLACK);
    tft->fillCircle(45, 218, 15, CFK_COLOR21);
    tft->drawCircle(45, 218, 15, CFK_BLACK);
    tft->fillCircle(62, 245, 10, CFK_COLOR06);
    tft->drawCircle(62, 245, 10, CFK_BLACK);
    tft->fillCircle(38, 232, 10, CFK_COLOR17);
    tft->drawCircle(38, 232, 10, CFK_BLACK);
    tft->fillCircle(61, 218, 10, CFK_COLOR28);
    tft->drawCircle(61, 218, 10, CFK_BLACK);
    tft->fillRect(0, 0, 24, 19, CFK_COLOR18);
    tft->drawRect(0, 0, 24, 19, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets(){

}
