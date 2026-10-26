// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 480 // Largura do display
#define DISPLAY_H 320 // Altura do display
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
void screen1();

// Prototypes for callback functions
void toggle0tgb_cb();

// Include for plugins of chip 2
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>
SPIClass spi_shared(FSPI); // Objeto para a comunicação SPI
Arduino_DataBus *bus = nullptr; // Objeto para a comunicação com o display (Arduino GFX)
Arduino_GFX *tft = nullptr; // Objeto para a comunicação com o display (Arduino GFX)
uint8_t rotationScreen = 1; // Valor de rotação do display
DisplayWX myDisplay;
const uint8_t qtdToggleBtn = 1;
ToggleButton arrayTogglebtn[qtdToggleBtn] = {ToggleButton(35, 165, 0)};
//SPIClass spi_shared(FSPI);
// Texto de exemplo.

void setup(){
    Serial.begin(115200);
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI); // Inicializa a comunicação SPI
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared); // Inicializa o objeto para a comunicação com o display
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, false); // Inicializa o objeto para a comunicação com o display
    tft->begin(DISP_FREQUENCY); // Inicializa a comunicação com o display
    
    WidgetBase::objTFT = tft; // Referência para o objeto para desenhar na tela
    //void startTouchXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSclk, int8_t pinMosi, int8_t pinMiso, int8_t pinCS, SPIClass *_sharedSPI, Arduino_GFX *_objTFT);
    myDisplay.startTouchFT6236U(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_FT6X36_SDA, TOUCH_FT6X36_SCL, -1, -1); //Start comunication with touch controller
    //myDisplay.recalibrate(); // Enable recalibrate for touch
    //myDisplay.checkCalibration(); // Check and apply calibration values
    myDisplay.enableTouchLog(); // Disable touch log (magenta dots when press)
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen1; // Use this line to change between screens
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
void screen1(){
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    uint8_t largura = 20;
    uint8_t offset = largura/2;
    uint16_t cor = CFK_GREY13;
    //Em pé
    tft->fillRect(0, 0, largura, DISPLAY_H, cor);
    tft->fillRect((DISPLAY_W / 2) - offset, 0, largura, DISPLAY_H, cor);
    tft->fillRect(DISPLAY_W - largura, 0, largura, DISPLAY_H, cor);
    

    tft->fillRect(0, 0, DISPLAY_W, largura, cor);
    tft->fillRect(0, (DISPLAY_H / 2 - offset), DISPLAY_W, largura, cor);
    tft->fillRect(0, DISPLAY_H - largura, DISPLAY_W, largura, cor);
    
    myDisplay.drawWidgetsOnScreen(1);
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
