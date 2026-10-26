// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 128
#define DISPLAY_H 64
#define DISP_SCLK	12
#define DISP_DT	11
#define DISP_CS	18
#define DISP_DC	14
#define DISP_RST	8

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

// Include for plugins of chip 1
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <U8g2lib.h>
#include <displaywx.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
U8G2_ST7565_NHD_C12864_F_4W_SW_SPI *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
void u8g2_prepare(void) {
    tft->setFont(u8g2_font_6x10_tf);
    tft->setFontRefHeightExtendedText();
    tft->setDrawColor(1);
    tft->setFontPosTop();
    tft->setFontDirection(0);
}
DisplayWX myDisplay;

void setup(){
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    tft = new U8G2_ST7565_NHD_C12864_F_4W_SW_SPI(U8G2_R0, DISP_SCLK, DISP_DT, DISP_CS, DISP_DC, DISP_RST);
    tft->begin();
    tft->enableUTF8Print();
    tft->clearBuffer();
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    // Touch model not defined. No need startTouch() function.
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){
    delay(2000);
}

void screen0(){
    tft->clearBuffer();
    u8g2_prepare();
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->drawStr(30, 51, "Custom text");
    tft->drawDisc(11, 40, 10);
    tft->drawCircle(11, 40, 10);
    tft->drawCircle(11, 11, 10);
    tft->drawFrame(25, 5, 18, 17);
    tft->drawBox(25, 30, 18, 17);
    tft->drawFrame(25, 30, 18, 17);
    tft->drawLine(47,6,66,47);
    tft->drawTriangle(80, 9, 112, 11, 87, 27);
    tft->sendBuffer();
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
}
