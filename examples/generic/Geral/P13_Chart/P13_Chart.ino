// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define ROBOTO_15 "ROBOTO_15"

// Prototypes for each screen
void screen0();

// Prototypes for callback functions
void rectbutton_cb();

// Include for plugins of chip 2
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
#define TFT_BL 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
Led ledblue(100, 70, 0);
Led ledgreen(95, 123, 0);
Led ledred(175, 95, 0);
Led ledorange(230, 170, 0);
Led ledyellow(205, 270, 0);
Led ledpink(110, 365, 0);
const uint8_t qtdLed = 6;
Led *arrayLed[qtdLed] = {&ledblue, &ledgreen, &ledred, &ledorange, &ledyellow, &ledpink};

Label text(370, 185, 0);
Label text1(585, 190, 0);
const uint8_t qtdLabel = 2;
Label *arrayLabel[qtdLabel] = {&text, &text1};
Label *seriesGrafico[qtdLabel] = {&text, &text1};

LineChart graficocima(370, 25, 0);
LineChart graficobaixo(375, 265, 0);
const uint8_t qtdLineChart = 2;
LineChart *arrayLinechart[qtdLineChart] = {&graficocima, &graficobaixo};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR28,CFK_COLOR02};
const uint8_t qtdLinesChart1 = 2;
uint16_t colorsChart1[qtdLinesChart1] = {CFK_COLOR05,CFK_COLOR11};
RectButton rectbutton(50, 205, 0);
const uint8_t qtdRectBtn = 1;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton};
// Texto de exemplo.

int contador = 0;

void setup(){
    Serial.begin(115200);
    rgbpanel = new Arduino_ESP32RGBPanel(
    41, 40, 39, 42,
    14, 21, 47, 48, 45,
    9, 46, 3, 8, 16, 1,
    15, 7, 6, 5, 4,
    0, 210, 30, 16,
    0, 22, 13, 10,
    1, 16000000, false,
    0, 0, 0);
    tft = new Arduino_RGB_Display(
    800, 480, rgbpanel,
    0, true);
    tft->begin();
    #if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST);
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(); // Initialize the task to read touch and draw
}

void loop(){
    float anguloRad = contador * PI / 180.0;
    
    float valor1 = (sin(anguloRad) + 1.0) * 50.0; // escala para 0–100
    float valor2 = (cos(anguloRad) + 1.0) * 50.0; // escala para 0–100
    float valor3 = (sin(2 * anguloRad) + sin(3 * anguloRad)) * 20.0 + 50.0;
    contador+=1;
    
    graficocima.push(0, valor1); //Use this line to add value in serie 0.
    graficocima.push(1, valor2); //Use this line to add value in serie 1.
    graficobaixo.push(0, valor1); //Use this line to add value in serie 0.
    graficobaixo.push(1, valor3); //Use this line to add value in serie 1.
    //text.setTextInt(valor1);
    delay(100);
}

void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    //This screen has a/an grafico
    //This screen has a/an rectButton
    //This screen has a/an led
    //This screen has a/an grafico
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
  LabelConfig configLabel0 = {
            .text = "",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR28,
            .backgroundColor = CFK_GREY3
        };
    text.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR02,
            .backgroundColor = CFK_GREY3
        };
    text1.setup(configLabel1);
    myDisplay.setLabel(arrayLabel,qtdLabel);
    
    LedConfig configLed0 = {
            .radius = 8,
            .colorOn = CFK_COLOR18
        };
    ledblue.setup(configLed0);
    LedConfig configLed1 = {
            .radius = 16,
            .colorOn = CFK_COLOR13
        };
    ledgreen.setup(configLed1);
    LedConfig configLed2 = {
            .radius = 24,
            .colorOn = CFK_COLOR01
        };
    ledred.setup(configLed2);
    LedConfig configLed3 = {
            .radius = 24,
            .colorOn = CFK_COLOR04
        };
    ledorange.setup(configLed3);
    LedConfig configLed4 = {
            .radius = 24,
            .colorOn = CFK_COLOR06
        };
    ledyellow.setup(configLed4);
    LedConfig configLed5 = {
            .radius = 24,
            .colorOn = CFK_COLOR27
        };
    ledpink.setup(configLed5);
    myDisplay.setLed(arrayLed,qtdLed);
    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = seriesGrafico,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 342,
            .height = 135,
            .gridColor = CFK_GREY4,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .verticalDivision = 2,
            .maxPointsAmount = 20,
            .amountSeries = qtdLinesChart0,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = true
        };
    graficocima.setup(configLineChart0);
    LineChartConfig configLineChart1 = {
            .colorsSeries = colorsChart1,
            .subtitles = nullptr,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 338,
            .height = 167,
            .gridColor = CFK_GREY4,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .verticalDivision = 2,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart1,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = true,
            .showDots = true
        };
    graficobaixo.setup(configLineChart1);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);
    RectButtonConfig configRectButton0 = {
            .callback = rectbutton_cb,
            .width = 87,
            .height = 72,
            .pressedColor = CFK_COLOR20
        };
rectbutton.setup(configRectButton0);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);
}

// This function is a callback of this element rectbutton.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton_cb(){
    bool myValue = rectbutton.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
    ledblue.setState(myValue);
    ledgreen.setState(myValue);
    ledred.setState(myValue);
    ledorange.setState(myValue);
    ledyellow.setState(myValue);
    ledpink.setState(myValue);
}