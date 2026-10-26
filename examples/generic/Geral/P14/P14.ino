// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define RED 0xf800
#define GREEN 0x07e0
#define BLUE 0x001f
#define WHITE 0xffff
#define BLACK 0x0000
#define MAGENTA 0xf81f
#define YELLOW 0xffe0
#define CYAN 0x07ff

// Prototypes for each screen
void screen0();

// Prototypes for callback functions
void toggle0tgb_cb();

// Include for plugins of chip 2
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
#define DISP_BL 2
Arduino_ESP32RGBPanel *bus = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 3;
    DisplayWX myDisplay;
const uint8_t qtdToggleBtn = 1;
ToggleButton arrayTogglebtn[qtdToggleBtn] = {ToggleButton(35, 165, 0)};
// Texto de exemplo.

void setup(){
    Serial.begin(115200);
    bus = new Arduino_ESP32RGBPanel(
  /* de          */ 40,
  /* vsync       */ 41,
  /* hsync       */ 39,
  /* pclk        */ 42,

  /* r0-r4       */ 45, 48, 47, 21, 14,
  /* g0-g5       */  5,  6,  7, 15, 16,  4,
  /* b0-b4       */  8,  3, 46,  9,  1,

  /* hsync */ 0, 8, 4, 8,
  /* vsync */ 0, 8, 4, 8,
  /* pclk_active_neg */ 1,
  /* prefer_speed     */ 16000000,
  /* useBigEndian     */ false,

  /* de_idle_high     */ 0,
  /* pclk_idle_high   */ 0,
  /* bounce_buffer_px */ 0
);

tft = new Arduino_RGB_Display(800 /* width */, 480 /* height */, bus, 0 /* rotation */, true /* auto_flush */);



    tft->begin(); // Initialize comunication with display
    #if defined(DISP_BL)
    pinMode(DISP_BL, OUTPUT);
    digitalWrite(DISP_BL, HIGH); // Turn on backlight for display if TFT_BL exists
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST); //Start comunication with touch controller
    // myDisplay.recalibrate(); // Enable recalibrate for touch
    //myDisplay.checkCalibration(); // Check and apply calibration values
    myDisplay.enableTouchLog(); // Disable touch log (magenta dots when press)
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(); // Initialize the task to read touch and draw
}

void loop(){
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(10, 10, 50, 50, RED);
    tft->fillRect(20, 20, 50, 50, GREEN);
    tft->fillRect(30, 30, 50, 50, BLUE);
    tft->fillRect(40, 40, 50, 50, YELLOW);
    tft->fillRect(50, 50, 50, 50, CYAN);
    tft->fillRect(60, 60, 50, 50, MAGENTA);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    ToggleButtonConfig configToggle0 = {
            .callback = toggle0tgb_cb,
            .width = 109,
            .height = 55,
            .pressedColor = CFK_RED
        };
    arrayTogglebtn[0].setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);
}

// This function is a callback of this element arrayTogglebtn[0].
// You dont need call it. Make sure it is as short and quick as possible.
void toggle0tgb_cb(){
    bool myValue = arrayTogglebtn[0].getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
