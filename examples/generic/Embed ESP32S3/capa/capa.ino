// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define TC_SCL	20
#define TC_SDA	19
#define TC_INT	-1
#define TC_RST	38
#define TOUCH_FREQUENCY	2500000
#define TOUCH_INVERT_X	false
#define TOUCH_INVERT_Y	false
#define TOUCH_MAP_X0	0
#define TOUCH_MAP_X1	800
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	480
#define TOUCH_SWAP_XY	false

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void toggle_cb();
void toggle1_cb();
void toggle2_cb();
void toggle3_cb();
void slider_cb();
void slider1_cb();
void slider2_cb();
void checkbox_cb();
void checkbox1_cb();
void checkbox2_cb();
void spinbox_cb();
void spinbox1_cb();
void spinbox2_cb();
void callbackOfGroup_grupo1();

// Include for plugins of chip 1
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
#define DISP_LED 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
CheckBox checkbox(475, 185, 0);
CheckBox checkbox1(475, 220, 0);
CheckBox checkbox2(475, 255, 0);
const uint8_t qtdCheckbox = 3;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox, &checkbox1, &checkbox2};
GaugeSuper widget(160, 155, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&widget};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {0,25,50,75};
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR08,CFK_COLOR06,CFK_COLOR05,CFK_COLOR01};
HSlider slider(15, 255, 0);
HSlider slider1(15, 220, 0);
HSlider slider2(15, 182, 0);
const uint8_t qtdHSlider = 3;
HSlider *arrayHslider[qtdHSlider] = {&slider, &slider1, &slider2};
Label line1(575, 300, 0);
Label line2(540, 300, 0);
const uint8_t qtdLabel = 2;
Label *arrayLabel[qtdLabel] = {&line1, &line2};
LineChart linechart(265, 294, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&linechart};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR03,CFK_COLOR10};
RadioGroup grupo1(0);
const uint8_t qtdRadioGroup = 1;
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1};
radio_t radiosOfGroup_grupo1[4] = {{290, 190, CFK_COLOR01, 1},{290, 215, CFK_COLOR01, 1},{290, 241, CFK_COLOR01, 1},{290, 268, CFK_COLOR01, 1}};
SpinBox spinbox(285, 425, 0);
SpinBox spinbox1(385, 425, 0);
SpinBox spinbox2(495, 425, 0);
const uint8_t qtdSpinbox = 3;
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox, &spinbox1, &spinbox2};
ToggleButton toggle(20, 425, 0);
ToggleButton toggle1(85, 425, 0);
ToggleButton toggle2(215, 425, 0);
ToggleButton toggle3(150, 425, 0);
const uint8_t qtdToggleBtn = 4;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle, &toggle1, &toggle2, &toggle3};
VAnalog vertalog(20, 295, 0);
VAnalog vertalog1(210, 295, 0);
VAnalog vertalog2(80, 295, 0);
VAnalog vertalog3(150, 295, 0);
const uint8_t qtdVAnalog = 4;
VAnalog *arrayVanalog[qtdVAnalog] = {&vertalog, &vertalog1, &vertalog2, &vertalog3};
VBar vertbar(324, 45, 0);
VBar vertbar1(354, 45, 0);
VBar vertbar2(384, 44, 0);
VBar vertbar3(419, 44, 0);
VBar vertbar4(449, 43, 0);
VBar vertbar5(481, 43, 0);
VBar vertbar6(514, 43, 0);
const uint8_t qtdVBar = 7;
VBar *arrayVbar[qtdVBar] = {&vertbar, &vertbar1, &vertbar2, &vertbar3, &vertbar4, &vertbar5, &vertbar6};
Image pinionpng(640, 420, 0);
Image rightpng(750, 420, 0);
Image stoppng(695, 420, 0);
const uint8_t qtdImagem = 3;
Image *arrayImagem[qtdImagem] = {&pinionpng, &rightpng, &stoppng};
CircularBar circload(715, 81, 0);
CircularBar circload1(715, 216, 0);
CircularBar circload2(715, 351, 0);
const uint8_t qtdCircBar = 3;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload, &circload1, &circload2};
// Texto de exemplo.

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
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, TC_INT, TC_RST);
    //myDisplay.enableTouchLog();
    //myDisplay.recalibrate();
    //myDisplay.checkCalibration();
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(); // Initialize the task to read touch and draw
}

void loop(){
    widget.setValue(random(0, 100)); //Use this command to change widget value.
    line1.setText("Vac"); //Use this command to change widget value.
    line2.setText("I ac"); //Use this command to change widget value.
    linechart.push(0, random(0, 100)); //Use this line to add value in serie 0.
    linechart.push(1, random(0, 100)); //Use this line to add value in serie 1.
    vertalog.setValue(random(0, 45), true); //Use this command to change widget value.
    vertalog1.setValue(random(0, 100), true); //Use this command to change widget value.
    vertalog2.setValue(random(12, 23), true); //Use this command to change widget value.
    vertalog3.setValue(random(0, 100), true); //Use this command to change widget value.
    vertbar.setValue(random(0, 77)); //Use this command to change widget value.
    vertbar1.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar2.setValue(random(0, 45)); //Use this command to change widget value.
    vertbar3.setValue(random(0, 77)); //Use this command to change widget value.
    vertbar4.setValue(random(0, 77)); //Use this command to change widget value.
    vertbar5.setValue(random(0, 77)); //Use this command to change widget value.
    vertbar6.setValue(random(0, 77)); //Use this command to change widget value.
    circload.setValue(random(0, 100)); //Use this command to change widget value.
    circload1.setValue(random(0, 100)); //Use this command to change widget value.
    circload2.setValue(random(0, 100)); //Use this command to change widget value.
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_GREY6);
    WidgetBase::backgroundColor = CFK_GREY6;
    tft->fillRect(5, 405, 609, 74, CFK_GREY3);
    tft->drawRect(5, 405, 609, 74, CFK_BLACK);
    tft->fillRect(9, 409, 600, 66, CFK_GREY5);
    tft->drawRect(9, 409, 600, 66, CFK_BLACK);
    tft->fillRect(24, 21, 280, 137, CFK_BLACK);
    tft->drawRect(24, 21, 280, 137, CFK_BLACK);
    //This screen has a/an radiobutton
    //This screen has a/an toggleButton
    //This screen has a/an circularBar
    //This screen has a/an ponteiroAnalog
    myDisplay.printText("Temp. Caldeira1", 677, 18, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    tft->fillRect(630, 5, 3, 463, CFK_GREY5);
    tft->drawRect(630, 5, 3, 463, CFK_BLACK);
    myDisplay.printText("Motor1", 25, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an imagem
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an grafico
    //This screen has a/an label
    //This screen has a/an label
    myDisplay.printText("Motor3", 155, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Motor2", 90, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Motor1", 55, 486, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Motor1", 65, 496, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Motor4", 220, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an slider
    //This screen has a/an slider
    myDisplay.printText("Potência. Caldeira 1", 95, 171, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Potência. Caldeira 2", 95, 209, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Stop", 699, 459, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    //This screen has a/an circularBar
    //This screen has a/an imagem
    tft->fillRect(20, 285, 106, 5, CFK_GREY5);
    tft->drawRect(20, 285, 106, 5, CFK_BLACK);
    tft->fillRoundRect(460, 175, 159, 114, 10, CFK_GREY5);
    tft->drawRoundRect(460, 175, 159, 114, 10, CFK_GREY1);
    myDisplay.printText("RPM", 130, 281, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    tft->fillRect(160, 285, 92, 4, CFK_GREY5);
    tft->drawRect(160, 285, 92, 4, CFK_BLACK);
    myDisplay.printText("Temp. Caldeira2", 675, 152, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    //This screen has a/an checkbox
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    //This screen has a/an circularBar
    tft->fillRoundRect(271, 174, 176, 114, 10, CFK_GREY5);
    tft->drawRoundRect(271, 174, 176, 114, 10, CFK_GREY1);
    //This screen has a/an rectRange
    //This screen has a/an slider
    myDisplay.printText("Potência. Caldeira 3", 92, 246, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Screen 2", 745, 458, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    //This screen has a/an imagem
    myDisplay.printText("Config", 640, 459, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Vazão 1", 315, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Duto principal ", 105, 32, TL_DATUM, CFK_BLACK, CFK_GREY11, &RobotoRegular10pt7b);
    myDisplay.printText("Vazão 2", 410, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Vazão 3", 525, 456, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    tft->fillRect(545, 45, 16, 15, CFK_COLOR01);
    tft->drawRect(545, 45, 16, 15, CFK_BLACK);
    myDisplay.printText("EVOLUÇÃO DO FLUXO NO DUTO", 365, 21, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("OPERAÇAO EM TESTE", 310, 186, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("APENAS MONITORAMENTO", 310, 211, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("OPERAÇÃO PLENA", 310, 237, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an radiobutton
    myDisplay.printText("MODO VISTORIA", 310, 263, TL_DATUM, CFK_COLOR12, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("VÁLVULAS ATIVAS", 505, 191, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("VÁLVULAS 12V", 505, 261, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    myDisplay.printText("VÁLVULAS DT 2", 504, 226, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular5pt7b);
    tft->fillRect(545, 70, 16, 15, CFK_COLOR03);
    tft->drawRect(545, 70, 16, 15, CFK_BLACK);
    tft->fillRect(545, 95, 16, 15, CFK_COLOR20);
    tft->drawRect(545, 95, 16, 15, CFK_BLACK);
    tft->fillRect(545, 120, 16, 15, CFK_COLOR18);
    tft->drawRect(545, 120, 16, 15, CFK_BLACK);
    tft->fillRect(545, 145, 16, 15, CFK_COLOR15);
    tft->drawRect(545, 145, 16, 15, CFK_BLACK);
    myDisplay.printText("Normal", 565, 148, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Média", 565, 123, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Temp. Caldeira 3", 679, 288, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.printText("Média", 567, 99, TL_DATUM, CFK_COLOR12, CFK_GREY6, &RobotoRegular5pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    CheckBoxConfig configCheckbox0 = {
            .callback = checkbox_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR28,
            .uncheckedColor = 0
        };
    checkbox.setup(configCheckbox0);
    CheckBoxConfig configCheckbox1 = {
            .callback = checkbox1_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR12,
            .uncheckedColor = 0
        };
    checkbox1.setup(configCheckbox1);
    CheckBoxConfig configCheckbox2 = {
            .callback = checkbox2_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR12,
            .uncheckedColor = 0
        };
    checkbox2.setup(configCheckbox2);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);
    GaugeConfig configGauge0 = {
            .title = "Title",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 283,
            .borderColor = CFK_GREY5,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_GREY11,
            .titleColor = CFK_BLACK,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    widget.setup(configGauge0);
    myDisplay.setGauge(arrayGauge,qtdGauge);
    HSliderConfig configHSlider0 = {
            .callback = slider_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 10,
            .width = 238,
            .pressedColor = CFK_COLOR28,
            .backgroundColor = 0
        };
    slider.setup(configHSlider0);
    HSliderConfig configHSlider1 = {
            .callback = slider1_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 10,
            .width = 238,
            .pressedColor = CFK_COLOR22,
            .backgroundColor = 0
        };
    slider1.setup(configHSlider1);
    HSliderConfig configHSlider2 = {
            .callback = slider2_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 10,
            .width = 238,
            .pressedColor = CFK_COLOR03,
            .backgroundColor = 0
        };
    slider2.setup(configHSlider2);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);
    LabelConfig configLabel0 = {
            .text = "Vac",
            .prefix = "pref",
            .suffix = "suf",
            .fontFamily = &RobotoRegular5pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR03,
            .backgroundColor = CFK_GREY6
        };
    line1.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "I ac",
            .prefix = "pref",
            .suffix = "suf",
            .fontFamily = &RobotoRegular5pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR10,
            .backgroundColor = CFK_GREY6
        };
    line2.setup(configLabel1);
    myDisplay.setLabel(arrayLabel,qtdLabel);
    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = nullptr,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 349,
            .height = 116,
            .gridColor = CFK_GREY9,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .verticalDivision = 10,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = true,
            .showDots = true
        };
    linechart.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);
    RadioGroupConfig configRadioGroup0 = {
            .buttons = radiosOfGroup_grupo1,
            .callback = callbackOfGroup_grupo1,
            .radius = 10,
            .group = 1,
            .amount = 4,
            .defaultClickedId = 1
        };
    grupo1.setup(configRadioGroup0);
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);
    SpinBoxConfig configSpinBox0 = {
            .callback = spinbox_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 81,
            .height = 26,
            .step = 1,
            .color = CFK_COLOR12,
            .textColor = CFK_BLACK
        };
    spinbox.setup(configSpinBox0);
    SpinBoxConfig configSpinBox1 = {
            .callback = spinbox1_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 81,
            .height = 26,
            .step = 1,
            .color = CFK_COLOR12,
            .textColor = CFK_BLACK
        };
    spinbox1.setup(configSpinBox1);
    SpinBoxConfig configSpinBox2 = {
            .callback = spinbox2_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 81,
            .height = 26,
            .step = 1,
            .color = CFK_COLOR27,
            .textColor = CFK_BLACK
        };
    spinbox2.setup(configSpinBox2);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);
    ToggleButtonConfig configToggle0 = {
            .callback = toggle_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR01
        };
    toggle.setup(configToggle0);
    ToggleButtonConfig configToggle1 = {
            .callback = toggle1_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR01
        };
    toggle1.setup(configToggle1);
    ToggleButtonConfig configToggle2 = {
            .callback = toggle2_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR01
        };
    toggle2.setup(configToggle2);
    ToggleButtonConfig configToggle3 = {
            .callback = toggle3_cb,
            .width = 41,
            .height = 21,
            .pressedColor = CFK_COLOR01
        };
    toggle3.setup(configToggle3);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);
    VerticalAnalogConfig configVAnalog0 = {
            .minValue = 0,
            .maxValue = 45,
            .width = 42,
            .height = 102,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_GREY12,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog.setup(configVAnalog0);
    VerticalAnalogConfig configVAnalog1 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 44,
            .height = 104,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_GREY12,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog1.setup(configVAnalog1);
    VerticalAnalogConfig configVAnalog2 = {
            .minValue = 12,
            .maxValue = 23,
            .width = 44,
            .height = 104,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_GREY12,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog2.setup(configVAnalog2);
    VerticalAnalogConfig configVAnalog3 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 44,
            .height = 104,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_GREY12,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog3.setup(configVAnalog3);
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog);
    VerticalBarConfig configVBar0 = {
            .minValue = 0,
            .maxValue = 77,
            .width = 20,
            .height = 112,
            .filledColor = CFK_COLOR20
        };
    vertbar.setup(configVBar0);
    VerticalBarConfig configVBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 20,
            .height = 114,
            .filledColor = CFK_COLOR15
        };
    vertbar1.setup(configVBar1);
    VerticalBarConfig configVBar2 = {
            .minValue = 0,
            .maxValue = 45,
            .width = 20,
            .height = 115,
            .filledColor = CFK_COLOR03
        };
    vertbar2.setup(configVBar2);
    VerticalBarConfig configVBar3 = {
            .minValue = 0,
            .maxValue = 77,
            .width = 20,
            .height = 115,
            .filledColor = CFK_COLOR20
        };
    vertbar3.setup(configVBar3);
    VerticalBarConfig configVBar4 = {
            .minValue = 0,
            .maxValue = 77,
            .width = 20,
            .height = 116,
            .filledColor = CFK_COLOR20
        };
    vertbar4.setup(configVBar4);
    VerticalBarConfig configVBar5 = {
            .minValue = 0,
            .maxValue = 77,
            .width = 20,
            .height = 116,
            .filledColor = CFK_COLOR18
        };
    vertbar5.setup(configVBar5);
    VerticalBarConfig configVBar6 = {
            .minValue = 0,
            .maxValue = 77,
            .width = 20,
            .height = 116,
            .filledColor = CFK_COLOR02
        };
    vertbar6.setup(configVBar6);
    myDisplay.setVBar(arrayVbar,qtdVBar);
    ImageFromFileConfig configImage0 = {
            .path = "/Pinionpng.fki",
            .cb = nullptr,
            .source = SourceFile::SPIFFS
        };
    pinionpng.setup(configImage0);
    ImageFromFileConfig configImage1 = {
            .path = "/Rightpng.fki",
            .cb = nullptr,
            .source = SourceFile::SPIFFS
        };
    rightpng.setup(configImage1);
    ImageFromFileConfig configImage2 = {
            .path = "/Stoppng.fki",
            .cb = nullptr,
            .source = SourceFile::SPIFFS
        };
    stoppng.setup(configImage2);
    myDisplay.setImage(arrayImagem,qtdImagem);
    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 50,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR01,
            .backgroundColor = CFK_WHITE,
            .textColor = CFK_BLACK,
            .thickness = 15,
            .showValue = true,
            .inverted = false
        };
    circload.setup(configCirculaBar0);
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 50,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR12,
            .backgroundColor = CFK_WHITE,
            .textColor = CFK_BLACK,
            .thickness = 15,
            .showValue = true,
            .inverted = false
        };
    circload1.setup(configCirculaBar1);
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 50,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR03,
            .backgroundColor = CFK_WHITE,
            .textColor = CFK_BLACK,
            .thickness = 15,
            .showValue = true,
            .inverted = false
        };
    circload2.setup(configCirculaBar2);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);
}

// This function is a callback of this element checkbox.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox_cb(){
    bool myValue = checkbox.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}
// This function is a callback of this element checkbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox1_cb(){
    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}
// This function is a callback of this element checkbox2.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox2_cb(){
    bool myValue = checkbox2.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}
// This function is a callback of this element slider.
// You dont need call it. Make sure it is as short and quick as possible.
void slider_cb(){
    int myValue = slider.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element slider1.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1_cb(){
    int myValue = slider1.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element slider2.
// You dont need call it. Make sure it is as short and quick as possible.
void slider2_cb(){
    int myValue = slider2.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of radio buttons of group 1.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo1(){
    uint16_t selectedOption = grupo1.getSelected();
    Serial.print("New value for radiogroup is: ");Serial.println(selectedOption);
}
// This function is a callback of this element spinbox.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox_cb(){
    int myValue = spinbox.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element spinbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox1_cb(){
    int myValue = spinbox1.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element spinbox2.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox2_cb(){
    int myValue = spinbox2.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle_cb(){
    bool myValue = toggle.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){
    bool myValue = toggle1.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle2.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle2_cb(){
    bool myValue = toggle2.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle3.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle3_cb(){
    bool myValue = toggle3.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
