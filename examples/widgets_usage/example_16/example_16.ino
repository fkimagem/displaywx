/*

--- English ---

This project demonstrates control of an external LED (pin 16) via a TouchArea and visual feedback with a
Led widget. When the user touches the TouchArea, the callback (touchaarea2_cb) toggles the variable
isLedOn, writes the new state to the GPIO with digitalWrite, sets isLedStateChanged to true and prints
a message to Serial. In the loop, when isLedStateChanged is true, the code updates the on-screen Led
with led1.setState(isLedOn) inside startCustomDraw and finishCustomDraw, then clears the flag. The
Led widget shows the same on/off state as the physical LED.

The TouchArea widget is an invisible (or custom-drawn) rectangular region that triggers a callback
when the user touches it. It does not display a value; it is used for buttons, hotspots or custom
gestures. Here the TouchArea acts as a toggle: the callback inverts the LED state and sets a flag so
the loop updates the on-screen Led. The Led widget is a visual indicator that shows on or off state;
the program sets the state with setState() so it stays in sync with the hardware (e.g. the physical
LED on pin 16).

The screen title is "LED Controller". The display uses ST7796 and FT6336U on ESP32S3. The loop
ends with delay(100).

--- Português ---

Este projeto demonstra o controle de um LED externo (pino 16) por meio de uma TouchArea e feedback
visual com um widget Led. Quando o usuário toca na TouchArea, o callback (touchaarea2_cb) alterna a
variável isLedOn, escreve o novo estado no GPIO com digitalWrite, define isLedStateChanged como true
e imprime uma mensagem na Serial. No loop, quando isLedStateChanged é true, o código atualiza o Led
na tela com led1.setState(isLedOn) entre startCustomDraw e finishCustomDraw, depois zera o flag. O
widget Led exibe o mesmo estado ligado/desligado do LED físico.

O widget TouchArea é uma região retangular invisível (ou desenhada por você) que dispara um callback
quando o usuário a toca. Não exibe valor; é usado para botões, áreas clicáveis ou gestos. Aqui a
TouchArea age como um toggle: o callback inverte o estado do LED e seta um flag para o loop
atualizar o Led na tela. O widget Led é um indicador visual que mostra estado ligado ou desligado; o
programa define o estado com setState() para mantê-lo sincronizado com o hardware (ex.: o LED físico
no pino 16).

O título da tela é "LED Controller". O display usa ST7796 e FT6336U no ESP32S3. O loop termina com
delay(100).

*/

// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3
    * Display: ST7796
    * Touch: FT6336U
    */
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
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
const int TOUCH_SCL = 13;
const int TOUCH_SDA = 14;
const int TOUCH_INT = 18;
const int TOUCH_RST = 17;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void touchaarea2_cb();

// Create global SPI object
#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
Led led1(230, 130, 0);
const uint8_t qtdLed = 1;
Led *arrayLed[qtdLed] = {&led1};
TouchArea touchaarea2(200, 100, 0);
const uint8_t qtdTouchArea = 1;
TouchArea *arrayToucharea[qtdTouchArea] = {&touchaarea2};

const int pinLed = 16;
int isLedOn = LOW;
bool isLedStateChanged = false;

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw

    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, isLedOn);
}

void loop(){

    if(isLedStateChanged){
        myDisplay.startCustomDraw();
        led1.setState(isLedOn);
        myDisplay.finishCustomDraw();
        isLedStateChanged = false;
    }
    delay(100);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("LED Controller", 165, 42, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an led
    //This screen has a/an touchArea
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LedConfig configLed0 = {
            .radius = 24,
            .colorOn = CFK_COLOR14,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led1.setup(configLed0);
    myDisplay.setLed(arrayLed,qtdLed);


    TouchAreaConfig configTouchArea0 = {
            .callback = touchaarea2_cb,
            .width = 57,
            .height = 57
        };
    touchaarea2.setup(configTouchArea0);
    myDisplay.setTouchArea(arrayToucharea,qtdTouchArea);


}

// This function is a callback of this element touchaarea2.
// You dont need call it. Make sure it is as short and quick as possible.
void touchaarea2_cb(){
    isLedOn = !isLedOn;
    digitalWrite(pinLed, isLedOn);
    isLedStateChanged = true;
    Serial.println("AreaTouch was clicked");
}
