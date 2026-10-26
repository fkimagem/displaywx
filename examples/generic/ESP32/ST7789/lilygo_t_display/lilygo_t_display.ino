// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 170
#define DISPLAY_H 320
#define DISP_CS	5
#define DISP_MISO	19
#define DISP_MOSI	23
#define DISP_SCLK	18
#define DISP_RST	4
#define DISP_DC	2
#define DISP_FREQUENCY	27000000
#define TC_CS	21
#define TC_SCL	18
#define TC_MOSI	23
#define TC_MISO	19
#define TOUCH_FREQUENCY	2500000
#define TOUCH_INVERT_X	false
#define TOUCH_INVERT_Y	false
#define TOUCH_MAP_X0	0
#define TOUCH_MAP_X1	170
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	320
#define TOUCH_SWAP_XY	false

#define GFX_BL 38

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

// Include for plugins of chip 1
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
#if defined(CONFIG_IDF_TARGET_ESP32S3)
//SPIClass spi_shared(FSPI);
#else
//SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;
// Texto de exemplo.

void setup(){
    Serial.begin(115200);
    pinMode(15 /* PWD */, OUTPUT);
    digitalWrite(15 /* PWD */, HIGH); 

    #ifdef GFX_BL
        pinMode(GFX_BL, OUTPUT);
        digitalWrite(GFX_BL, HIGH); 
    #endif

    //spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_ESP32PAR8Q(7 /* DC */, 6 /* CS */, 8 /* WR */, 9 /* RD */, 39 /* D0 */, 40 /* D1 */, 41 /* D2 */, 42 /* D3 */, 45 /* D4 */, 46 /* D5 */, 47 /* D6 */, 48 /* D7 */);
    tft = new Arduino_ST7789(bus, 5 /* RST */, 0 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);
    tft->begin();


    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillCircle(75, 260, 22, CFK_COLOR17);
    tft->drawCircle(75, 260, 22, CFK_COLOR01);
    myDisplay.printText("Custom text", 25, 97, TL_DATUM, CFK_COLOR05, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("99.9", 40, 154, TL_DATUM, CFK_COLOR13, CFK_GREY3, &G7_Segment7_S520pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
}
