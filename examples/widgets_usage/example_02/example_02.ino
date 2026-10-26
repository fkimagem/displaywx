/*

--- English ---

This project is an example of using two ToggleButton widgets to control two LEDs. Each ToggleButton
acts as a switch that toggles between on and off on each touch; when the user touches and the state
changes, the DisplayWX library calls the callback associated with that widget. The program uses
these callbacks to turn the corresponding LED on or off, keeping the LED state in sync with the
toggle position on screen (ball on the right = on, on the left = off).

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Two digital outputs
(pins 16 and 17) are configured to drive LEDs. On screen, two ToggleButtons are shown with the
labels "Led 1" and "Led 2" beside them so the user can tell which control drives which LED.

The ToggleButton widget is a control with a sliding ball that moves from left to right when touched,
toggling between off and on. Each of the two ToggleButtons (toggle1 and toggle2) is configured in
loadWidgets with the same color when on and with its own callback. When the user touches a toggle
and the state changes, only that toggle's callback runs: it reads the new state with getStatus(),
prints to Serial for debug and applies that state to the LED pin with digitalWrite (HIGH when on,
LOW when off). LEDs are driven only by the callbacks.

The main loop is empty because all action is triggered by touch on the ToggleButtons and handled in
the callbacks. The screen is drawn in screen0: white background, the two text labels and
drawWidgetsOnScreen(0) to draw the two ToggleButtons on screen 0.

--- Português ---

Este projeto é um exemplo de uso de dois widgets ToggleButton para comando de dois LEDs. Cada
ToggleButton funciona como um interruptor que alterna entre ligado e desligado a cada toque; quando
o usuário toca e inverte o estado, a biblioteca DisplayWX chama a função de callback associada a
esse widget. O programa usa essas callbacks para ligar ou desligar o LED correspondente, mantendo o
estado do LED sincronizado com a posição do toggle na tela (bola à direita = ligado, à esquerda =
desligado).

O projeto utiliza um display ST7796 colorido, um touch capacitivo FT6336U e um ESP32S3. Duas saídas
digitais (pinos 16 e 17) são configuradas para acionar LEDs. Na tela são exibidos dois ToggleButtons
com os rótulos "Led 1" e "Led 2" ao lado, permitindo identificar qual controle comanda qual LED.

O widget ToggleButton é um controle com uma bola deslizante que se move da esquerda para a direita
ao ser tocado, alternando entre estado desligado e ligado. Cada um dos dois ToggleButtons (toggle1
e toggle2) foi configurado em loadWidgets com a mesma cor quando ligado e com um callback próprio.
Quando o usuário toca em um toggle e o estado muda, apenas o callback daquele toggle é executado:
ele lê o novo estado com getStatus(), envia o valor ao Serial para debug e aplica esse estado ao
pino do LED correspondente com digitalWrite (HIGH quando ligado, LOW quando desligado). Os LEDs são
comandados exclusivamente pelas callbacks.

O loop do programa fica vazio porque toda a ação é disparada pelo toque nos ToggleButtons e tratada
nas callbacks. A tela é desenhada em screen0: fundo branco, os dois rótulos de texto e a chamada
drawWidgetsOnScreen(0) para desenhar os dois ToggleButtons na tela 0.

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
void toggle1_cb();
void toggle2_cb();

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
ToggleButton toggle1(105, 105, 0);
ToggleButton toggle2(105, 165, 0);
const uint8_t qtdToggleBtn = 2;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1, &toggle2};

const int pinLed1 = 16;
const int pinLed2 = 17;

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

    pinMode(pinLed1, OUTPUT);
    pinMode(pinLed2, OUTPUT);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    myDisplay.printText("Led 1", 160, 107, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Led 2", 160, 167, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    ToggleButtonConfig configToggle0 = {
            .callback = toggle1_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR30
        };
    toggle1.setup(configToggle0);
    ToggleButtonConfig configToggle1 = {
            .callback = toggle2_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR30
        };
    toggle2.setup(configToggle1);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


}

// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){

    bool myValue = toggle1.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
    digitalWrite(pinLed1, myValue ? HIGH : LOW);
}
// This function is a callback of this element toggle2.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle2_cb(){

    bool myValue = toggle2.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
    digitalWrite(pinLed2, myValue ? HIGH : LOW);
}
