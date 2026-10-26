/*

--- English ---

This project is an example of a LineChart with one series and navigation between two screens. Screen 0
shows the chart (ADC on pin 16 mapped to 0–100, updated every 10 ms) and a "Next" button; screen 1
shows a message and a "Click here" button to return. The LineChart is configured with
workInBackground = true: the chart keeps receiving new values via push() even when the user is on
screen 1, so when they go back to screen 0 the graph already has the latest data and no points are
lost while the chart was not visible.

The number of series in the chart is one. By default the screen editor uses two series; here the
second line was given the same colour as the graph background, so it was removed during code
generation and only one series remains (one line and one Label as subtitle).

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. The loop reads the
ADC, maps to 0–100, and calls linechart1.push(0, mappedValue) inside startCustomDraw and
finishCustomDraw. The "Next" button callback loads screen 1; the "Click here" button callback loads
screen 0. Screen drawing is done in screen0() and screen1() with drawWidgetsOnScreen() for the
current screen index.

--- Português ---

Este projeto é um exemplo de LineChart com uma série e navegação entre duas telas. A tela 0 exibe o
gráfico (ADC no pino 16 mapeado para 0–100, atualizado a cada 10 ms) e um botão "Next"; a tela 1
exibe uma mensagem e um botão "Click here" para voltar. O LineChart está configurado com
workInBackground = true: o gráfico continua recebendo novos valores via push() mesmo quando o
usuário está na tela 1, de modo que ao voltar para a tela 0 o gráfico já tem os dados mais recentes
e nenhum ponto é perdido enquanto o gráfico não estava visível.

A quantidade de séries no gráfico é uma. Por padrão o editor de tela usa duas séries; aqui a cor da
segunda linha ficou igual à cor de fundo do gráfico, então ela foi removida na geração de código e
permanece apenas uma série (uma linha e um Label como legenda).

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. O loop lê o ADC,
mapeia para 0–100 e chama linechart1.push(0, mappedValue) entre startCustomDraw e finishCustomDraw.
O callback do botão "Next" carrega a tela 1; o callback do botão "Click here" carrega a tela 0. O
desenho das telas é feito em screen0() e screen1() com drawWidgetsOnScreen() para o índice da tela
atual.

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
void screen1();
void loadWidgets();

// Prototypes for callback functions
void btnnext_cb();
void btnbackpage_cb();

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
Label labelchart11(170, 225, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&labelchart11};
LineChart linechart1(90, 80, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&linechart1};
Label *seriesGrafico0[1] = {&labelchart11};
const uint8_t qtdLinesChart0 = 1;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR08};
TextButton btnnext(370, 15, 0);
TextButton btnbackpage(125, 150, 1);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btnnext, &btnbackpage};

const int pinAdcFrequencyGenerator = 16;

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

    pinMode(pinAdcFrequencyGenerator, INPUT);
}

void loop(){

    int valueAdc = analogRead(pinAdcFrequencyGenerator);
    int mappedValue = map(valueAdc, 0, 4095, 0, 100);

    myDisplay.startCustomDraw();
    linechart1.push(0, mappedValue); //Use this line to add value in serie 0.
    myDisplay.finishCustomDraw();
    delay(10);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(66, 207, 313, 56, CFK_GREY3);
    tft->drawRect(66, 207, 313, 56, CFK_BLACK);
    //This screen has a/an grafico
    //This screen has a/an label
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(0);
}
void screen1(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Click on button to back screen", 110, 87, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(1);
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
            .workInBackground = true,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = false
        };
    linechart1.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);


    TextButtonConfig configTextButton0 = {
            .text = "Next",
            .callback = btnnext_cb,
            .fontFamily = &RobotoBold8pt7b,
            .width = 97,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_COLOR24,
            .textColor = CFK_WHITE
        };

    btnnext.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Click here",
            .callback = btnbackpage_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 243,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE
        };

    btnbackpage.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

// This function is a callback of this element btnnext.
// You dont need call it. Make sure it is as short and quick as possible.
void btnnext_cb(){

    Serial.print("Clicked on: ");Serial.println("btnnext_cb");
    myDisplay.loadScreen(screen1);
}
// This function is a callback of this element btnbackpage.
// You dont need call it. Make sure it is as short and quick as possible.
void btnbackpage_cb(){

    Serial.print("Clicked on: ");Serial.println("btnbackpage_cb");
    myDisplay.loadScreen(screen0);
}
