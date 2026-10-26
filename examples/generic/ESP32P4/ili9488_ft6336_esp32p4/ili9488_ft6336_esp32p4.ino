// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>

// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;


const int DISP_MOSI = 23;
const int DISP_MISO = -1;
const int DISP_SCLK = 26;
const int DISP_CS = 20;
const int DISP_DC = 22;
const int DISP_RST = 21;

const int TOUCH_SCL = 18;//8
const int TOUCH_SDA = 17;//7
const int TOUCH_INT = 47;
const int TOUCH_RST = 33;


// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void toggle_cb();
void slider_cb();

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
HSlider slider(85, 270, 0);
const uint8_t qtdHSlider = 1;
HSlider *arrayHslider[qtdHSlider] = {&slider};
ToggleButton toggle(170, 180, 0);
const uint8_t qtdToggleBtn = 1;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle};
CircularBar circload(390, 90, 0);
const uint8_t qtdCircBar = 1;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload};

void setup(){
    Serial.begin(115200);
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, true);
    tft->begin(DISP_FREQUENCY);
    //WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.setDrawObject(tft);
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    //myDisplay.startAutoClick(); // Start auto click
    loadWidgets(); // This function is used to setup with widget individualy
    //WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.loadScreen(screen0);
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){
    circload.setValue(random(0, 100)); //Use this command to change widget value.
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an slider
    //This screen has a/an toggleButton
    //This screen has a/an circularBar
    tft->fillCircle(65, 45, 40, CFK_COLOR01);
    tft->drawCircle(65, 45, 40, CFK_COLOR01);
    tft->fillCircle(94, 92, 40, CFK_COLOR11);
    tft->drawCircle(94, 92, 40, CFK_COLOR01);
    tft->fillCircle(41, 95, 40, CFK_COLOR22);
    tft->drawCircle(41, 95, 40, CFK_COLOR01);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    HSliderConfig configHSlider0 = {
            .callback = slider_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 16,
            .width = 279,
            .pressedColor = CFK_COLOR03
        };
    slider.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);

    ToggleButtonConfig configToggle0 = {
            .width = 94,
            .height = 47,
            .pressedColor = CFK_COLOR01,
            .callback = toggle_cb
        };
    toggle.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    CircularBarConfig configCirculaBar0 = {
            .radius = 80,
            .minValue = 0,
            .maxValue = 100,
            .startAngle = 0,
            .endAngle = 360,
            .thickness = 20,
            .color = CFK_COLOR19,
            .backgroundColor = CFK_WHITE,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_WHITE,
            .showValue = true,
            .inverted = false
        };
    circload.setup(configCirculaBar0);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);

}

// This function is a callback of this element slider.
// You dont need call it. Make sure it is as short and quick as possible.
void slider_cb(){
    int myValue = slider.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle_cb(){
    bool myValue = toggle.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
    if(myValue){
      myDisplay.refreshScreen();  
    }
}
