/*

--- English ---

This project displays an analog voltage on screen using a single Label widget. The ESP32 reads the ADC
on pin 16 in the loop, converts the raw value to volts with convertADCtoVolts() (scale 3.3 V / 4095),
then updates the label between startCustomDraw and finishCustomDraw with setTextFloat(voltage, 2) so
the value is shown with two decimal places. The Label is configured with prefix "Voltage: " and suffix
" V", so the user sees text like "Voltage: 1.65 V". The screen title is "ADC volts". There are no
touch callbacks.

The Label widget displays text or numeric values on screen. It has no touch callback; the program
updates the content with setText(), setTextInt() or setTextFloat(). Optional prefix and suffix (e.g.
"Voltage: " and " V") are concatenated with the value so the user sees a full string like "Voltage:
1.65 V" without extra formatting code.

The display uses ST7796 and FT6336U on ESP32S3. The loop ends with delay(2000) to limit update
frequency.

--- Português ---

Este projeto exibe uma tensão analógica na tela usando um único widget Label. O ESP32 lê o ADC no
pino 16 no loop, converte o valor bruto para volts com convertADCtoVolts() (escala 3,3 V / 4095), e
atualiza o label entre startCustomDraw e finishCustomDraw com setTextFloat(voltage, 2), exibindo o
valor com duas casas decimais. O Label está configurado com prefix "Voltage: " e suffix " V", de modo
que o usuário vê texto como "Voltage: 1,65 V". O título da tela é "ADC volts". Não há callbacks de
toque.

O widget Label exibe texto ou valores numéricos na tela. Não possui callback de toque; o programa
atualiza o conteúdo com setText(), setTextInt() ou setTextFloat(). Prefix e suffix opcionais (ex.:
"Voltage: " e " V") são concatenados ao valor para que o usuário veja uma string completa como
"Voltage: 1,65 V" sem código extra de formatação.

O display usa ST7796 e FT6336U no ESP32S3. O loop termina com delay(2000) para limitar a frequência
de atualização.

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
Label txtvolts(230, 120, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&txtvolts};


float convertADCtoVolts(int adcValue){
    return (float)adcValue * (3.3 / 4095.0);
}

const int pinAdc = 16;


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

    int valueAdc = analogRead(pinAdc);
    float voltage = convertADCtoVolts(valueAdc);

    myDisplay.startCustomDraw();
    txtvolts.setTextFloat(voltage, 2); //Use this command to change widget value.
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("ADC volts", 185, 42, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an label
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "---",
            .prefix = "Voltage: ",
            .suffix = " V",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TC_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
    txtvolts.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


}


