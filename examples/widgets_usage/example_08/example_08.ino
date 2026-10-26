/*

--- English ---

This project is an example of using two Image widgets for navigation between two screens. Each image
is loaded from embedded pixel data (headers irightpng.h and ileftpng.h) and has a touch callback.
When the user touches the image on screen 0 (irightpng), the DisplayWX library calls irightpng_cb,
which prints to Serial and calls myDisplay.loadScreen(screen1) to switch to screen 1. When the user
touches the image on screen 1 (ileftpng), ileftpng_cb runs and calls myDisplay.loadScreen(screen0)
to return to screen 0. The images thus act as navigation arrows or buttons between the two screens.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Screen 0 shows the
title "Screen 0" and the right image (e.g. a right arrow); screen 1 shows "Screen 1" and the left
image (e.g. a left arrow). Both screens use a white background and drawWidgetsOnScreen() for the
current screen index.

The Image widget displays bitmap graphics and can have a callback for touch. Both images are
configured with setupFromPixels() using pixel data, dimensions and an optional alpha mask. The
callbacks only perform navigation and debug output; the main loop is empty. Screen content is
drawn in screen0() and screen1(), each filling the background and drawing the widgets for that
screen.

--- Português ---

Este projeto é um exemplo de uso de dois widgets Image para navegação entre duas telas. Cada imagem
é carregada a partir de dados de pixels embutidos (headers irightpng.h e ileftpng.h) e possui um
callback de toque. Quando o usuário toca na imagem da tela 0 (irightpng), a biblioteca DisplayWX
chama irightpng_cb, que imprime no Serial e chama myDisplay.loadScreen(screen1) para ir à tela 1.
Quando o usuário toca na imagem da tela 1 (ileftpng), ileftpng_cb é executado e chama
myDisplay.loadScreen(screen0) para voltar à tela 0. As imagens funcionam assim como setas ou botões
de navegação entre as duas telas.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. A tela 0 exibe o
título "Screen 0" e a imagem da direita (por exemplo uma seta para a direita); a tela 1 exibe
"Screen 1" e a imagem da esquerda (por exemplo uma seta para a esquerda). Ambas as telas usam
fundo branco e drawWidgetsOnScreen() para o índice da tela atual.

O widget Image exibe gráficos em bitmap e pode ter um callback para toque. As duas imagens são
configuradas com setupFromPixels() usando dados de pixels, dimensões e máscara de transparência
opcional. As callbacks apenas realizam a navegação e a saída de debug; o loop principal fica vazio.
O conteúdo de cada tela é desenhado em screen0() e screen1(), cada uma preenchendo o fundo e
desenhando os widgets daquela tela.

*/

#include "irightpng.h"
#include "ileftpng.h"
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
void screen1();
void loadWidgets();

// Prototypes for callback functions
void irightpng_cb();
void ileftpng_cb();

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
Image irightpng(435, 15, 0);
Image ileftpng(10, 10, 1);
const uint8_t qtdImagem = 2;
Image *arrayImagem[qtdImagem] = {&irightpng, &ileftpng};

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

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Screen 0", 175, 22, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an imagem
    myDisplay.drawWidgetsOnScreen(0);
}
void screen1(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Screen 1", 175, 22, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an imagem
    myDisplay.drawWidgetsOnScreen(1);
}

// Configure each widgtes to be used
void loadWidgets(){

    ImageFromPixelsConfig configImage0 = {
            .pixels = irightpngPixels,
            .maskAlpha = irightpngMask,
            .cb = irightpng_cb,
            .angle = 0,
            .width = irightpngW,
            .height = irightpngH,
            .backgroundColor = CFK_WHITE
        };
    irightpng.setupFromPixels(configImage0);
    ImageFromPixelsConfig configImage1 = {
            .pixels = ileftpngPixels,
            .maskAlpha = ileftpngMask,
            .cb = ileftpng_cb,
            .angle = 0,
            .width = ileftpngW,
            .height = ileftpngH,
            .backgroundColor = CFK_WHITE
        };
    ileftpng.setupFromPixels(configImage1);
    myDisplay.setImage(arrayImagem,qtdImagem);


}

// This function is a callback of this element irightpng.
// You dont need call it. Make sure it is as short and quick as possible.
void irightpng_cb(){

    // Image Rightpng clicked
    Serial.println("Image Rightpng clicked");
    myDisplay.loadScreen(screen1);
}
// This function is a callback of this element ileftpng.
// You dont need call it. Make sure it is as short and quick as possible.
void ileftpng_cb(){

    // Image Leftpng clicked
    Serial.println("Image Leftpng clicked");
    myDisplay.loadScreen(screen0);
}
