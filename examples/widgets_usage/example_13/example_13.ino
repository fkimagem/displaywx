/*

--- English ---

This project implements an RGB colour picker using three HSlider widgets (R, G and B, each 0–255). Each
slider has a callback that reads its value with getValue(), stores it in a global variable
(hslider0_val, hslider1_val, hslider2_val) and calls updateColorCircle(). That function builds a
16-bit colour with convertRGBto565(R, G, B), assigns it to colorCircle and sets isColorCircleChanged
to true. In the loop, inside startCustomDraw and finishCustomDraw, when isColorCircleChanged is true
the code redraws a circle at a fixed position with the new colour (fillCircle plus drawCircle for the
border), then clears the flag. The circle is also drawn once in screen0 so it appears on load. Labels
"R", "G" and "B" are printed beside the sliders. There are no other touch callbacks.

The HSlider widget is a horizontal slider that lets the user choose a numeric value within a min–max
range by dragging a thumb. When the user releases the thumb, the library calls the widget's callback;
the callback typically reads the current value with getValue() and uses it (e.g. to update a colour).
Each of the three HSliders in this example has its own callback that stores the value and triggers a
redraw of the colour preview circle.

The display uses ST7796 and FT6336U on ESP32S3. The loop ends with a delay(2000) to reduce redraw
frequency.

--- Português ---

Este projeto implementa um seletor de cor RGB com três widgets HSlider (R, G e B, cada um 0–255). Cada
slider possui um callback que lê seu valor com getValue(), armazena em uma variável global
(hslider0_val, hslider1_val, hslider2_val) e chama updateColorCircle(). Essa função monta uma cor de
16 bits com convertRGBto565(R, G, B), atribui a colorCircle e define isColorCircleChanged como true.
No loop, entre startCustomDraw e finishCustomDraw, quando isColorCircleChanged é true o código
redesenha um círculo em posição fixa com a nova cor (fillCircle e drawCircle para a borda), depois
zera o flag. O círculo também é desenhado uma vez em screen0 para aparecer ao carregar. As etiquetas
"R", "G" e "B" são impressas ao lado dos sliders. Não há outros callbacks de toque.

O widget HSlider é um slider horizontal que permite ao usuário escolher um valor numérico em uma
faixa min–max arrastando um cursor. Quando o usuário solta o cursor, a biblioteca chama o callback
do widget; o callback normalmente lê o valor atual com getValue() e o utiliza (ex.: para atualizar
uma cor). Cada um dos três HSliders neste exemplo tem seu próprio callback que armazena o valor e
dispara o redesenho do círculo de preview da cor.

O display usa ST7796 e FT6336U no ESP32S3. O loop termina com delay(2000) para reduzir a frequência
de redesenho.

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
void slider1_cb();
void slider1copy_cb();
void slider1copy2_cb();


uint16_t convertRGBto565(uint8_t red, uint8_t green, uint8_t blue){
    return (red >> 3) << 11 | (green >> 2) << 5 | (blue >> 3);
}

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
HSlider slider1(95, 40, 0);
HSlider slider1copy(95, 95, 0);
HSlider slider1copy2(95, 155, 0);
const uint8_t qtdHSlider = 3;
HSlider *arrayHslider[qtdHSlider] = {&slider1, &slider1copy, &slider1copy2};
int hslider0_val = 0; // Global variable that stores the value of the widget slider1
int hslider1_val = 0; // Global variable that stores the value of the widget slider1copy
int hslider2_val = 0; // Global variable that stores the value of the widget slider1copy2

uint16_t colorCircle = CFK_BLACK;// Black
bool isColorCircleChanged = false;
uint16_t circlePosX = 220;
uint16_t circlePosY = 260;
uint16_t circleRadius = 29;

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
}

void loop(){

    myDisplay.startCustomDraw();
    if(isColorCircleChanged){
        tft->fillCircle(circlePosX, circlePosY, circleRadius, colorCircle);
        tft->drawCircle(circlePosX, circlePosY, circleRadius, CFK_BLACK);
        isColorCircleChanged = false;
    }
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("R", 55, 42, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("G", 55, 102, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("B", 55, 162, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an slider
    //This screen has a/an slider
    //This screen has a/an slider
    tft->fillCircle(circlePosX, circlePosY, circleRadius, colorCircle);
    tft->drawCircle(circlePosX, circlePosY, circleRadius, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    HSliderConfig configHSlider0 = {
            .callback = slider1_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 255,
            .radius = 15,
            .width = 254,
            .pressedColor = CFK_COLOR01,
            .backgroundColor = CFK_WHITE
        };
    slider1.setup(configHSlider0);
    HSliderConfig configHSlider1 = {
            .callback = slider1copy_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 255,
            .radius = 15,
            .width = 254,
            .pressedColor = CFK_COLOR11,
            .backgroundColor = CFK_WHITE
        };
    slider1copy.setup(configHSlider1);
    HSliderConfig configHSlider2 = {
            .callback = slider1copy2_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 255,
            .radius = 15,
            .width = 254,
            .pressedColor = CFK_COLOR22,
            .backgroundColor = CFK_WHITE
        };
    slider1copy2.setup(configHSlider2);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);


}

void updateColorCircle(){
    colorCircle = convertRGBto565(hslider0_val, hslider1_val, hslider2_val);
    isColorCircleChanged = true;
}

// This function is a callback of this element slider1.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1_cb(){

    hslider0_val = slider1.getValue();
    updateColorCircle();
}
// This function is a callback of this element slider1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1copy_cb(){

    hslider1_val = slider1copy.getValue();
    updateColorCircle();
}
// This function is a callback of this element slider1copy2.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1copy2_cb(){

    hslider2_val = slider1copy2.getValue();
    updateColorCircle();
}
