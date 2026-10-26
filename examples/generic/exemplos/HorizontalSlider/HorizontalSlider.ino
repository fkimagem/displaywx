// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 480
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
#define TOUCH_MAP_X1	480
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	320
#define TOUCH_SWAP_XY	false

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void slider_cb();

// Include for plugins of chip 1
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
HSlider slider(105, 135, 0);
const uint8_t qtdHSlider = 1;
HSlider *arrayHslider[qtdHSlider] = {&slider};
int hslider0_val = 0; // Global variable that stores the value of the widget slider
// ------- ESCRITO MANUAL
int ledPin = 16;
// ------- FIM

void setup(){
    Serial.begin(115200);
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, false);
    tft->begin(DISP_FREQUENCY);
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SCL, TC_MOSI, TC_MISO, TC_CS, tft, TOUCH_FREQUENCY, DISP_FREQUENCY, DISP_CS);
    //myDisplay.enableTouchLog();
    //myDisplay.recalibrate();
    myDisplay.checkCalibration();
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    //myDisplay.startAutoClick(); // Start auto click
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    // ------- ESCRITO MANUAL
    pinMode(ledPin, OUTPUT);
    // ------- FIM
}

void loop(){
    // Use the variable hslider0_val to check value/status of widget slider
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_GREY10);
    WidgetBase::backgroundColor = CFK_GREY10;
    //This screen has a/an slider
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    HSliderConfig configHSlider0 = {
            .callback = slider_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 255,
            .radius = 26,
            .width = 271,
            .pressedColor = CFK_COLOR01
        };
    slider.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);

}

// This function is a callback of this element slider.
// You dont need call it. Make sure it is as short and quick as possible.
void slider_cb(){
    hslider0_val = slider.getValue();
    // ------- ESCRITO MANUAL
    analogWrite(ledPin, hslider0_val);
    // ------- FIM
}
