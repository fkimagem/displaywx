// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32
    * Display: SD1306
    * Touch: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 128;
const int DISPLAY_H = 32;
const uint8_t DISP_ADDRESS = 0x3C;
const int DISP_SDA = 21;
const int DISP_SCL = 22;
const int DISP_RST = -1;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

Adafruit_SSD1306 *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    Wire.begin(DISP_SDA, DISP_SCL);
    tft = new Adafruit_SSD1306(128, 32, &Wire, DISP_RST);
    if(!tft->begin(SSD1306_SWITCHCAPVCC, DISP_ADDRESS, false, false)) {
        Serial.println("Failed to initialize SSD1306");
        for(;;);
    }
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
    tft->drawCircle(16, 13, 11, CFK_COLOR01);
    myDisplay.printText("Custom text", 35, 6, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular5pt7b);
    tft->display();
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


