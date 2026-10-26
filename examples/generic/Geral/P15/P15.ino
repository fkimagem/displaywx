// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 480
#define DISPLAY_H 272
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
#define TFT_BL 2 // LED K
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
const uint8_t qtdToggleBtn = 1;
ToggleButton arrayTogglebtn[qtdToggleBtn] = {ToggleButton(35, 165, 0)};
//SPIClass spi_shared(FSPI);
// Texto de exemplo.

void setup(){
    Serial.begin(115200);

    rgbpanel = new Arduino_ESP32RGBPanel(
    // Sinais de controle
    40,  // DE
    41,  // VSYNC
    39,  // HSYNC
    42,  // PCLK

    // Pinos de dados - canal vermelho (R0 a R4)
    45, 48, 47, 21, 14,

    // Pinos de dados - canal verde (G0 a G5)
    5, 6, 7, 15, 16, 4,

    // Pinos de dados - canal azul (B0 a B4)
    8, 3, 46, 9, 1,

    // Temporização horizontal
    0,   // hsync_polarity
    1,   // hsync_front_porch
    1,   // hsync_pulse_width
    43,  // hsync_back_porch

    // Temporização vertical
    0,   // vsync_polarity
    3,   // vsync_front_porch
    1,   // vsync_pulse_width
    12,  // vsync_back_porch

    // Clock e performance
    1,         // pclk_active_neg
    10000000,  // prefer_speed
    false,     // useBigEndian

    // Configurações adicionais
    0,  // de_idle_high
    0,  // pclk_idle_high
    0   // bounce_buffer_size_px
);


    tft = new Arduino_RGB_Display(
    480,  // Largura
    272,  // Altura
    rgbpanel,
    0,    // Rotação
    true  // auto_flush
    );

    tft->begin(); // Initialize comunication with display
    #if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH); // Turn on backlight for display if TFT_BL exists
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    //void startTouchXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSclk, int8_t pinMosi, int8_t pinMiso, int8_t pinCS, SPIClass *_sharedSPI, Arduino_GFX *_objTFT);
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_XPT2046_SCK, TOUCH_XPT2046_MOSI, TOUCH_XPT2046_MISO, TOUCH_XPT2046_CS, nullptr, tft); //Start comunication with touch controller
    //myDisplay.recalibrate(); // Enable recalibrate for touch
    myDisplay.checkCalibration(); // Check and apply calibration values
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
