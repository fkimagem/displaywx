// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7262 - Waveshare
    * Touch: GT911
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 800;
const int DISPLAY_H = 480;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = 22;
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
constexpr int LCD_RGB_TIMING_FREQ_HZ = 16 * 1000 * 1000; // RGB timing frequency
constexpr int LCD_RGB_BOUNCE_BUFFER_SIZE = DISPLAY_W * 10;
#define DISP_LED 2
constexpr int LCD_RGB_TIMING_HPW = 4;   // Horizontal pulse width
constexpr int LCD_RGB_TIMING_HBP = 8;   // Horizontal back porch
constexpr int LCD_RGB_TIMING_HFP = 8;   // Horizontal front porch
constexpr int LCD_RGB_TIMING_VPW = 4;   // Vertical pulse width
constexpr int LCD_RGB_TIMING_VBP = 8;   // Vertical back porch
constexpr int LCD_RGB_TIMING_VFP = 8;   // Vertical front porch
Arduino_ESP32RGBPanel *bus = nullptr;
Arduino_RGB_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_ESP32RGBPanel(
    DISP_IO_DE, DISP_IO_VSYNC, DISP_IO_HSYNC, DISP_IO_PCLK,
    DISP_IO_DATA0, DISP_IO_DATA1, DISP_IO_DATA2, DISP_IO_DATA3, DISP_IO_DATA4,
    DISP_IO_DATA5, DISP_IO_DATA6, DISP_IO_DATA7, DISP_IO_DATA8, DISP_IO_DATA9, DISP_IO_DATA10,
    DISP_IO_DATA11, DISP_IO_DATA12, DISP_IO_DATA13, DISP_IO_DATA14, DISP_IO_DATA15,
    0, LCD_RGB_TIMING_HFP, LCD_RGB_TIMING_HPW, LCD_RGB_TIMING_HBP,
    0, LCD_RGB_TIMING_VFP, LCD_RGB_TIMING_VPW, LCD_RGB_TIMING_VBP,
    1, LCD_RGB_TIMING_FREQ_HZ, false,
    0, 0, LCD_RGB_BOUNCE_BUFFER_SIZE);
    tft = new Arduino_RGB_Display(800, 480, bus, rotationScreen, true);
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
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
    tft->fillCircle(62, 49, 15, CFK_COLOR01);
    tft->drawCircle(62, 49, 15, CFK_BLACK);
    tft->fillCircle(38, 63, 15, CFK_COLOR11);
    tft->drawCircle(38, 63, 15, CFK_BLACK);
    tft->fillCircle(38, 35, 15, CFK_COLOR21);
    tft->drawCircle(38, 35, 15, CFK_BLACK);
    tft->fillCircle(54, 63, 10, CFK_COLOR06);
    tft->drawCircle(54, 63, 10, CFK_BLACK);
    tft->fillCircle(30, 49, 10, CFK_COLOR17);
    tft->drawCircle(30, 49, 10, CFK_BLACK);
    tft->fillCircle(54, 35, 10, CFK_COLOR28);
    tft->drawCircle(54, 35, 10, CFK_BLACK);
    myDisplay.printText("Custom text", 105, 42, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

}


