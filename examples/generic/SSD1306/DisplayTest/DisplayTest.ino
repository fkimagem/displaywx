// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 128
#define DISPLAY_H 32
#define DISP_ADDRESS	0x3C
#define DISP_SDA 14
#define DISP_SCLK 13
#define DISP_RST	-1

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
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <displaywx.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
Adafruit_SSD1306 *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;

void setup(){
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    Wire.begin(DISP_SDA, DISP_SCLK);
    tft = new Adafruit_SSD1306(DISPLAY_W, DISPLAY_H, &Wire, DISP_RST);
    if(!tft->begin(SSD1306_SWITCHCAPVCC, DISP_ADDRESS, false, false)) {
        Serial.println("Failed to initialize SSD1306");
        for(;;);
    }
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
    tft->clearDisplay();
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(3, 6, 21, 17, CFK_COLOR01);
    tft->drawRect(3, 6, 21, 17, CFK_COLOR01);
    tft->fillRect(62, 24, 36, 20, CFK_COLOR01);
    tft->drawRect(62, 24, 36, 20, CFK_COLOR01);
    myDisplay.printText("Custom text", 5, 37, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    tft->display();
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
}
