/*

--- English ---

This project is an example of using one LineChart widget with two series to display ADC readings over
time. Two Labels are used as subtitles (legends) for the two lines. There are no touch callbacks.
The loop reads the ADC on pin 16, maps the value to 0–100 and to 100–0 (inverse), then pushes both
values to the chart inside startCustomDraw and finishCustomDraw: series 0 gets the direct value and
series 1 gets the inverse, so the two lines move in opposite directions. The chart is updated every
2 seconds with new points, building a simple real-time plot.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Pin 16 is the analog
input. The screen has a white background, a grey area for the legend, and the LineChart with grid
and two coloured lines. The Labels (labelchart11 and labelchart22) appear as subtitles below the
chart and identify each series by colour (green and pink in the config).

The LineChart widget is configured with two series, two colours and an array of Label pointers as
subtitles. Values are added with push(seriesIndex, value). The chart range is 0–100; the grid and
border use the configured colours. Screen drawing is done in screen0 with drawWidgetsOnScreen(0).
If drawBackground() is required by the LineChart for initial layout, it should be called once after
setup() in loadWidgets.

--- Português ---

Este projeto é um exemplo de uso de um widget LineChart com duas séries para exibir leituras do ADC
ao longo do tempo. Dois Labels são usados como legendas das duas linhas. Não há callbacks de toque.
O loop lê o ADC no pino 16, mapeia o valor para 0–100 e para 100–0 (inverso) e envia os dois
valores ao gráfico entre startCustomDraw e finishCustomDraw: a série 0 recebe o valor direto e a
série 1 o inverso, de modo que as duas linhas se movem em sentidos opostos. O gráfico é atualizado
a cada 2 segundos com novos pontos, formando um plot em tempo real simples.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. O pino 16 é a entrada
analógica. A tela tem fundo branco, uma área cinza para a legenda e o LineChart com grade e duas
linhas coloridas. Os Labels (labelchart11 e labelchart22) aparecem como legendas abaixo do gráfico e
identificam cada série pela cor (verde e rosa na configuração).

O widget LineChart é configurado com duas séries, duas cores e um array de ponteiros para Label
como subtítulos. Os valores são adicionados com push(índiceDaSérie, valor). A faixa do gráfico é
0–100; a grade e a borda usam as cores configuradas. O desenho da tela é feito em screen0 com
drawWidgetsOnScreen(0). Se o LineChart exigir drawBackground() para o layout inicial, ele deve ser
chamado uma vez após setup() em loadWidgets.

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
Label labelchart11(90, 220, 0);
Label labelchart22(230, 220, 0);
const uint8_t qtdLabel = 2;
Label *arrayLabel[qtdLabel] = {&labelchart11, &labelchart22};
LineChart linechart1(90, 80, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&linechart1};
Label *seriesGrafico0[2] = {&labelchart11,&labelchart22};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR08,CFK_COLOR28};

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

    myDisplay.startCustomDraw();

    int valueAdc = analogRead(pinAdc);
    int mappedValue = map(valueAdc, 0, 4095, 0, 100);
    int mappedValueInverse = map(valueAdc, 0, 4095, 100, 0);
    linechart1.push(0, mappedValue); //Use this line to add value in serie 0.
    linechart1.push(1, mappedValueInverse); //Use this line to add value in serie 1.
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(66, 207, 313, 56, CFK_GREY3);
    tft->drawRect(66, 207, 313, 56, CFK_BLACK);
    //This screen has a/an grafico
    //This screen has a/an label
    //This screen has a/an label
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "label_chart 1",
            .prefix = "Line",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR08,
            .backgroundColor = CFK_GREY3
        };
    labelchart11.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "label_chart 2",
            .prefix = "Line",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR28,
            .backgroundColor = CFK_GREY3
        };
    labelchart22.setup(configLabel1);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = seriesGrafico0,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 258,
            .height = 119,
            .gridColor = CFK_BLACK,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE,
            .verticalDivision = 5,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = false
        };
    linechart1.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);


}


