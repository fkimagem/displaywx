// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <U8g2lib.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32P4
    * Display: ST7565 - U8G2
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 128;
const int DISPLAY_H = 64;
const int DISP_SCLK = 31;
const int DISP_DT = 51;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
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
U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *tft = nullptr;
void u8g2_prepare(void) {
    tft->setFont(u8g2_font_6x10_tf);
    tft->setFontRefHeightExtendedText();
    tft->setDrawColor(1);
    tft->setFontPosTop();
    tft->setFontDirection(0);
    //tft->setContrast(50);
}
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    tft = new U8G2_ST7565_NHD_C12864_F_4W_SW_SPI(U8G2_R0, DISP_SCLK, DISP_DT, DISP_CS, DISP_DC, DISP_RST);
    tft->begin();
    tft->enableUTF8Print();
    tft->clearBuffer();
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

}

void screen0(){

    tft->clearBuffer();
    u8g2_prepare();
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->drawBox(17, 23, 27, 16);
    tft->drawFrame(17, 23, 27, 16);
    tft->drawStr(20, 37, "p4");
    tft->sendBuffer();
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}
