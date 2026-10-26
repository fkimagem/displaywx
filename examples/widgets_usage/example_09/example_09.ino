/*

--- English ---

This project is an example of displaying one analog value (from ADC on pin 16) using several
different visual widgets: VBar (vertical and horizontal), VAnalog, CircularBar, Thermometer and
Label. There are no touch callbacks; the loop reads the ADC, maps the value to 0–100 and to
-100–100, then updates all widgets inside startCustomDraw and finishCustomDraw so the display is
refreshed once per cycle. The same reading is shown as a vertical bar, a horizontal bar, a circular
progress bar, a thermometer (with a Label as subtitle for the numeric value) and a vertical analog
scale with arrow, so you can compare how each widget represents the value.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Pin 16 is used as
analog input. The screen shows the title "Read ADC" and all widgets on a white background. The
Thermometer widget is linked to a Label that displays the current value with suffix "C".

The VBar, VAnalog, CircularBar and Thermometer widgets are updated in the loop with setValue(); the
VAnalog uses the -100 to 100 range so the arrow can move in both directions. The Label is updated
automatically by the Thermometer via its subtitle pointer. The loop runs every 2 seconds. Screen
drawing is done in screen0 with drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de exibição de um valor analógico (do ADC no pino 16) em vários widgets
visuais: VBar (vertical e horizontal), VAnalog, CircularBar, Thermometer e Label. Não há callbacks
de toque; o loop lê o ADC, mapeia o valor para 0–100 e para -100–100 e atualiza todos os widgets
entre startCustomDraw e finishCustomDraw para que o display seja atualizado uma vez por ciclo. A
mesma leitura é mostrada como barra vertical, barra horizontal, barra circular de progresso,
termômetro (com um Label como legenda do valor numérico) e escala analógica vertical com seta, para
comparar como cada widget representa o valor.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. O pino 16 é usado
como entrada analógica. A tela exibe o título "Read ADC" e todos os widgets sobre fundo branco. O
widget Thermometer está ligado a um Label que exibe o valor atual com sufixo "C".

Os widgets VBar, VAnalog, CircularBar e Thermometer são atualizados no loop com setValue(); o
VAnalog usa a faixa -100 a 100 para que a seta se mova nos dois sentidos. O Label é atualizado
automaticamente pelo Thermometer através do ponteiro subtitle. O loop executa a cada 2 segundos. O
desenho da tela é feito em screen0 com drawWidgetsOnScreen(0).

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
Label labelthermometer11(375, 255, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&labelthermometer11};
VAnalog vertalog1(285, 145, 0);
const uint8_t qtdVAnalog = 1;
VAnalog *arrayVanalog[qtdVAnalog] = {&vertalog1};
VBar vertbar1(15, 175, 0);
VBar vertbar2(40, 145, 0);
const uint8_t qtdVBar = 2;
VBar *arrayVbar[qtdVBar] = {&vertbar1, &vertbar2};
CircularBar circload1(205, 235, 0);
const uint8_t qtdCircBar = 1;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload1};
Thermometer thermometer1(390, 95, 0);
const uint8_t qtdThermometer = 1;
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer1};

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

    pinMode(pinAdc, INPUT);
}

void loop(){

    int valueAdc = analogRead(pinAdc);
    int mappedValue0to100 = map(valueAdc, 0, 4095, 0, 100);
    int mappedValueMinus100to100 = map(valueAdc, 0, 4095, -100, 100);

    myDisplay.startCustomDraw();
    vertalog1.setValue(mappedValueMinus100to100, true); //Use this command to change widget value.
    vertbar1.setValue(mappedValue0to100); //Use this command to change widget value.
    vertbar2.setValue(mappedValue0to100); //Use this command to change widget value.
    circload1.setValue(mappedValue0to100); //Use this command to change widget value.
    thermometer1.setValue(mappedValue0to100); //Use this command to change widget value.
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an verticalAnalog
    //This screen has a/an circularBar
    //This screen has a/an thermometer
    //This screen has a/an label
    myDisplay.printText("Read ADC", 190, 27, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "label_th",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR04,
            .backgroundColor = CFK_WHITE
        };
    labelthermometer11.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    VerticalAnalogConfig configVAnalog0 = {
            .minValue = -100,
            .maxValue = 100,
            .width = 41,
            .height = 154,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog1.setup(configVAnalog0);
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog);


    VerticalBarConfig configVBar0 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 116,
            .filledColor = CFK_COLOR30
        };
    vertbar1.setup(configVBar0);
    VerticalBarConfig configVBar1 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::HORIZONTAL,
            .width = 133,
            .height = 21,
            .filledColor = CFK_COLOR30
        };
    vertbar2.setup(configVBar1);
    myDisplay.setVBar(arrayVbar,qtdVBar);


    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 52,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR20,
            .backgroundColor = CFK_GREY11,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_WHITE,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    circload1.setup(configCirculaBar0);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);


    ThermometerConfig configThermometer0 = {
            .subtitle = &labelthermometer11,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 41,
            .height = 138,
            .filledColor = CFK_COLOR04,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1.setup(configThermometer0);
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);


}


