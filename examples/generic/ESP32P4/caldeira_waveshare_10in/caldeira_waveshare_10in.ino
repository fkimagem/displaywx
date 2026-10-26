#include "iPinionpng.h"
#include "iStoppng.h"
#include "iRightpng.h"
#include "iHelppng.h"
// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>
// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1280;
const int DISPLAY_H = 800;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;
// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
void rectbutton_cb();
void motor_cb();
void motor2_cb();
void motor3_cb();
void motor4_cb();
void slider_cb();
void slider1_cb();
void cbgas_cb();
void cbresf_cb();
void cbsuperresf_cb();
void pinionpng_cb();
void stoppng_cb();
void rightpng_cb();
void helppng_cb();
void spinbox_cb();
void spinbox1_cb();
void spinbox2_cb();
void spinbox3_cb();
void callbackOfGroup_grupo1();
#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
DisplayFK myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
CheckBox cbgas(850, 320, 0);
CheckBox cbresf(850, 355, 0);
CheckBox cbsuperresf(850, 390, 0);
const uint8_t qtdCheckbox = 3;
CheckBox *arrayCheckbox[qtdCheckbox] = {&cbgas, &cbresf, &cbsuperresf};
bool check0_val = false; // Global variable that stores the value of the widget cbgas
bool check1_val = false; // Global variable that stores the value of the widget cbresf
bool check2_val = false; // Global variable that stores the value of the widget cbsuperresf
GaugeSuper gauge(249, 300, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&gauge};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {0,25,50,75};
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR12,CFK_COLOR06,CFK_COLOR03,CFK_COLOR01};
HSlider slider(95, 438, 0);
HSlider slider1(34, 358, 0);
int slider_val = 0;
int slider1_val = 0;
const uint8_t qtdHSlider = 2;
HSlider *arrayHslider[qtdHSlider] = {&slider, &slider1};
Label line1(720, 620, 0);
Label line2(885, 620, 0);
Label text(405, 425, 0);
Label text3(405, 345, 0);
Label term1(545, 460, 0);
Label term15(545, 585, 0);
const uint8_t qtdLabel = 6;
Label *arrayLabel[qtdLabel] = {&line1, &line2, &text, &text3, &term1, &term15};
Led led(510, 185, 0);
Led led1(510, 225, 0);
Led led2(510, 265, 0);
Led led3(510, 310, 0);
const uint8_t qtdLed = 4;
Led *arrayLed[qtdLed] = {&led, &led1, &led2, &led3};
LineChart grafico(643, 470, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&grafico};
Label *seriesGrafico0[2] = {&line1,&line2};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR04,CFK_COLOR15};
RadioGroup grupo1(0);
const uint8_t qtdRadioGroup = 1;
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1};
radio_t radiosOfGroup_grupo1[3] = {{650, 365, CFK_COLOR01, 2},{650, 330, CFK_COLOR01, 1 },{650, 400, CFK_COLOR01, 3}};
uint16_t radiog1 = 2; // Global variable that stores the value of the widget grupo1
RectButton rectbutton(30, 420, 0);
const uint8_t qtdRectBtn = 1;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton};
SpinBox spinbox(855, 710, 0);
SpinBox spinbox1(500, 710, 0);
SpinBox spinbox2(680, 710, 0);
SpinBox spinbox3(855, 710, 0);
const uint8_t qtdSpinbox = 4;
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox, &spinbox1, &spinbox2, &spinbox3};
int spin1_val = 0; // Global variable that stores the value of the widget spinbox1
int spin2_val = 0; // Global variable that stores the value of the widget spinbox2
int spin3_val = 0; // Global variable that stores the value of the widget spinbox3
ToggleButton motor(35, 720, 0);
ToggleButton motor2(145, 720, 0);
ToggleButton motor3(260, 720, 0);
ToggleButton motor4(375, 720, 0);
const uint8_t qtdToggleBtn = 4;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&motor, &motor2, &motor3, &motor4};
bool  switch1_val = false; // Global variable that stores the value of the widget motor
bool  switch2_val = false; // Global variable that stores the value of the widget motor2
bool  switch3_val = false; // Global variable that stores the value of the widget motor3
bool  switch4_val = false; // Global variable that stores the value of the widget motor4
VAnalog barramotor(60, 500, 0);
VAnalog barramotor1(285, 500, 0);
VAnalog barramotor2(175, 500, 0);
VAnalog barramotor3(395, 500, 0);
const uint8_t qtdVAnalog = 4;
VAnalog *arrayVanalog[qtdVAnalog] = {&barramotor, &barramotor1, &barramotor2, &barramotor3};
VBar vbar(865, 70, 0);
VBar vbar1(825, 70, 0);
VBar vbar2(785, 70, 0);
VBar vbar3(750, 70, 0);
VBar vbar4(715, 70, 0);
VBar vbar5(680, 70, 0);
VBar vbar6(645, 70, 0);
const uint8_t qtdVBar = 7;
VBar *arrayVbar[qtdVBar] = {&vbar, &vbar1, &vbar2, &vbar3, &vbar4, &vbar5, &vbar6};
Image pinionpng(1230, 715, 0);
Image stoppng(1120, 715, 0);
Image rightpng(1175, 715, 0);
Image helppng(1065, 715, 0);
const uint8_t qtdImagem = 4;
Image *arrayImagem[qtdImagem] = {&pinionpng, &stoppng, &rightpng, &helppng};
CircularBar load(1165, 145, 0);
CircularBar load1(1164, 315, 0);
CircularBar load2(1165, 480, 0);
CircularBar load3(1165, 650, 0);
const uint8_t qtdCircBar = 4;
CircularBar *arrayCircularbar[qtdCircBar] = {&load, &load1, &load2, &load3};
Thermometer thermometer(500, 415, 0);
Thermometer thermometer1(500, 535, 0);
const uint8_t qtdThermometer = 2;
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer, &thermometer1};


// create 7 'int count' with constexpr offset between 0 and 6
constexpr int offsetCount = 20;
int countGeral = 0;
const int countSize = 7;
float counters[countSize] = {
    0,0,0,0,0,0,0
};


void increaseCounters(){
    for (size_t i = 0; i < countSize; i++)
    {
        counters[i] = sin((countGeral + (offsetCount * i)) * DEGTORAD);
        counters[i] = counters[i] * 50 + 50;// De 0 a 100
    }
    countGeral++;
}

bool flagAtualizarTela = false;

TimerHandle_t despertador;
void vTimerCallback(TimerHandle_t xTimer)
{
    if(!flagAtualizarTela){
        flagAtualizarTela = true;      
    }
  
}

void iniciarDespertador(){
    despertador = xTimerCreate(
    "Timer200ms",                 // nome do timer (opcional)
    pdMS_TO_TICKS(200),           // período em ticks (100ms)
    pdTRUE,                       // pdTRUE = auto-reload (infinito)
    (void*)0,                     // ID opcional do timer
    vTimerCallback                // função de callback
  );

  if (despertador != NULL) {
    // Inicia o timer (sem bloqueio)
    xTimerStart(despertador, 0);
    Serial.println("Timer iniciado!");
  } else {
    Serial.println("Falha ao criar o timer!");
  }
}

void setup(){
    Serial.begin(115200);
    bus = new Arduino_ESP32DSIPanel(
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    60000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    800 /* width default */, 1280 /* height default */, bus, 1, true,
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    //myDisplay.enableTouchLog();
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    //myDisplay.startAutoClick(); // Start auto click
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    iniciarDespertador();
}
void loop(){
    if(flagAtualizarTela){
        myDisplay.startCustomDraw(); // Bloqueia a task do displayfk de desenhar
        gauge.setValue(counters[0]);
        text.setTextInt(slider_val);
        text3.setTextInt(slider1_val);
        led.setState(counters[0] > 50);
        led1.setState(counters[1] > 50);
        led2.setState(counters[2] > 50);
        led3.setState(counters[3] > 50);
        grafico.push(0, counters[0]);
        grafico.push(1, counters[2]); 
        barramotor.setValue(counters[0], true); 
        barramotor1.setValue(counters[2], true);
        barramotor2.setValue(counters[4], true); 
        barramotor3.setValue(counters[6], true); 
        vbar.setValue(counters[0]); 
        vbar1.setValue(counters[1]); 
        vbar2.setValue(counters[2]); 
        vbar3.setValue(counters[3]); 
        vbar4.setValue(counters[4]); 
        vbar5.setValue(counters[5]); 
        vbar6.setValue(counters[6]); 
        load.setValue(counters[3]); 
        load1.setValue(counters[4]); 
        load2.setValue(counters[5]); 
        load3.setValue(counters[6]); 
        thermometer.setValue(counters[2]); 
        thermometer1.setValue(counters[3]); 
        myDisplay.finishCustomDraw();// Libera a task do displayfk para desenha
        flagAtualizarTela = false;
        increaseCounters();
    }
    delay(1);
}
void screen0(){
    tft->fillScreen(CFK_GREY5);
    WidgetBase::backgroundColor = CFK_GREY5;
    tft->fillRoundRect(630, 430, 387, 221, 6, CFK_GREY3);
    tft->fillRoundRect(485, 375, 134, 282, 6, CFK_GREY3);
    tft->fillRoundRect(20, 108, 454, 208, 6, CFK_GREY3);
    tft->fillRoundRect(17, 405, 457, 73, 6, CFK_GREY3);
    tft->fillRoundRect(15, 487, 461, 286, 6, CFK_GREY3);
    tft->fillRoundRect(484, 661, 541, 115, 6, CFK_GREY3);
    tft->drawRoundRect(630, 280, 187, 143, 10, CFK_GREY3);
    tft->drawRoundRect(835, 280, 187, 143, 10, CFK_GREY3);
    tft->drawRoundRect(628, 25, 394, 241, 10, CFK_GREY3);
    tft->fillRect(1047, 25, 3, 750, CFK_GREY3);
    //This screen has a/an ponteiroAnalog
    //This screen has a/an slider
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an rectRange
    tft->fillRect(925, 70, 20, 20, CFK_COLOR25);
    tft->drawRect(925, 70, 20, 20, CFK_BLACK);
    tft->fillRect(925, 95, 20, 20, CFK_COLOR22);
    tft->drawRect(925, 95, 20, 20, CFK_BLACK);
    tft->fillRect(925, 120, 20, 20, CFK_COLOR19);
    tft->drawRect(925, 120, 20, 20, CFK_BLACK);
    tft->fillRect(925, 145, 20, 20, CFK_COLOR12);
    tft->drawRect(925, 145, 20, 20, CFK_BLACK);
    tft->fillRect(925, 170, 20, 20, CFK_COLOR06);
    tft->drawRect(925, 170, 20, 20, CFK_BLACK);
    tft->fillRect(925, 195, 20, 20, CFK_COLOR03);
    tft->drawRect(925, 195, 20, 20, CFK_BLACK);
    tft->fillRect(925, 220, 20, 20, CFK_COLOR01);
    tft->drawRect(925, 220, 20, 20, CFK_BLACK);
    //This screen has a/an verticalAnalog
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    //This screen has a/an grafico
    //This screen has a/an toggleButton
    //This screen has a/an spinbox
    //This screen has a/an imagem
    //This screen has a/an imagem
    //This screen has a/an imagem
    //This screen has a/an imagem
    myDisplay.printText("Ajuda", 1070, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Parar", 1125, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Prox.", 1180, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    myDisplay.printText("Config", 1230, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    myDisplay.printText("Temp Caldeira 4", 1165, 566, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Temp Caldeira 3", 1165, 397, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Temp Caldeira 2", 1160, 236, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Temp Caldeira 1", 1165, 61, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Media 1", 950, 76, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 2", 950, 101, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 3", 950, 126, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 4", 950, 151, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 5", 950, 176, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 6", 950, 201, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media 7", 950, 226, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Media nos dutos", 745, 37, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoBold10pt7b);
    myDisplay.printText("Vistoria", 670, 396, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Plena", 670, 361, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Monitoramento", 670, 326, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Modo", 700, 291, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Valvulas ativas", 880, 291, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Gas", 880, 326, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Resfriamento", 880, 361, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    myDisplay.printText("Super refriamento", 880, 396, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    //This screen has a/an label
    //This screen has a/an label
    tft->fillRoundRect(490, 667, 529, 102, 6, CFK_GREY5);
    tft->fillRoundRect(22, 667, 447, 102, 6, CFK_GREY5);
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    myDisplay.printText("Vazao 1", 550, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Vazao 2", 725, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Vazao 3", 905, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    myDisplay.printText("Motor 1", 50, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Motor 2", 160, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Motor 3", 275, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Motor 4", 390, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    //This screen has a/an label
    //This screen has a/an rectButton
    tft->fillRoundRect(18, 325, 456, 73, 6, CFK_GREY3);
    //This screen has a/an slider
    //This screen has a/an label
    myDisplay.printText("Duto principal", 190, 127, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoBold10pt7b);
    myDisplay.printText("Sistema de controle", 260, 19, TC_DATUM, CFK_COLOR14, CFK_GREY5, &RobotoRegular20pt7b);
    myDisplay.printText("Caldeiras", 250, 68, TC_DATUM, CFK_COLOR14, CFK_GREY5, &RobotoRegular15pt7b);
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an label
    //This screen has a/an label
    myDisplay.printText("Fluidos", 520, 387, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("Consumo (kW)", 770, 442, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    tft->fillRoundRect(485, 106, 134, 257, 6, CFK_GREY3);
    myDisplay.printText("Esteiras", 515, 122, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    myDisplay.printText("Linha 1", 535, 176, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    myDisplay.printText("Linha 2", 535, 216, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    myDisplay.printText("Linha 3", 535, 256, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    myDisplay.printText("Linha 4", 535, 301, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    myDisplay.printText("Potencia caldeira", 160, 331, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    myDisplay.printText("Potencia sub caldeira", 165, 411, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets(){
    CheckBoxConfig configCheckbox0 = {
            .callback = cbgas_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR17,
            .uncheckedColor = CFK_GREY10
        };
    cbgas.setup(configCheckbox0);
    CheckBoxConfig configCheckbox1 = {
            .callback = cbresf_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR19,
            .uncheckedColor = CFK_GREY10
        };
    cbresf.setup(configCheckbox1);
    CheckBoxConfig configCheckbox2 = {
            .callback = cbsuperresf_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR21,
            .uncheckedColor = CFK_GREY10
        };
    cbsuperresf.setup(configCheckbox2);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);

    GaugeConfig configGauge0 = {
            .title = "Pressao",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 428,
            .height = 140,
            .borderColor = CFK_GREY5,
            .textColor = CFK_WHITE,
            .backgroundColor = CFK_GREY4,
            .titleColor = CFK_WHITE,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    gauge.setup(configGauge0);
    myDisplay.setGauge(arrayGauge,qtdGauge);

    HSliderConfig configHSlider0 = {
            .callback = slider_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 12,
            .width = 291,
            .pressedColor = CFK_COLOR04,
            .backgroundColor = CFK_WHITE
        };
    slider.setup(configHSlider0);
    HSliderConfig configHSlider1 = {
            .callback = slider1_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 12,
            .width = 352,
            .pressedColor = CFK_COLOR30,
            .backgroundColor = CFK_WHITE
        };
    slider1.setup(configHSlider1);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);

    LabelConfig configLabel0 = {
            .text = "Line 1",
            .prefix = "Motor A: ",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR04,
            .backgroundColor = CFK_GREY3
        };
    line1.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "Motor B ",
            .prefix = "Motor B:",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR15,
            .backgroundColor = CFK_GREY3
        };
    line2.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "50",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular15pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY3
        };
    text.setup(configLabel2);
    LabelConfig configLabel3 = {
            .text = "100",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular15pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY3
        };
    text3.setup(configLabel3);
    LabelConfig configLabel4 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR03,
            .backgroundColor = CFK_GREY3
        };
    term1.setup(configLabel4);
    LabelConfig configLabel5 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR20,
            .backgroundColor = CFK_GREY3
        };
    term15.setup(configLabel5);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR11,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led.setup(configLed0);
    LedConfig configLed1 = {
            .radius = 16,
            .colorOn = CFK_COLOR11,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led1.setup(configLed1);
    LedConfig configLed2 = {
            .radius = 16,
            .colorOn = CFK_COLOR11,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led2.setup(configLed2);
    LedConfig configLed3 = {
            .radius = 16,
            .colorOn = CFK_COLOR11,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led3.setup(configLed3);
    myDisplay.setLed(arrayLed,qtdLed);

    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = seriesGrafico0,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 360,
            .height = 137,
            .gridColor = CFK_GREY4,
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
    grafico.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);

    RadioGroupConfig configRadioGroup0 = {
            .buttons = radiosOfGroup_grupo1,
            .callback = callbackOfGroup_grupo1,
            .radius = 10,
            .group = 1,
            .amount = 3,
            .defaultClickedId = 2
        };
    grupo1.setup(configRadioGroup0);
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);

    RectButtonConfig configRectButton0 = {
            .callback = rectbutton_cb,
            .width = 51,
            .height = 39,
            .pressedColor = CFK_COLOR04
        };
rectbutton.setup(configRectButton0);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);

    SpinBoxConfig configSpinBox0 = {
            .callback = spinbox_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 152,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR20,
            .textColor = CFK_BLACK
        };
    spinbox.setup(configSpinBox0);
    SpinBoxConfig configSpinBox1 = {
            .callback = spinbox1_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 152,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR08,
            .textColor = CFK_BLACK
        };
    spinbox1.setup(configSpinBox1);
    SpinBoxConfig configSpinBox2 = {
            .callback = spinbox2_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 152,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR08,
            .textColor = CFK_BLACK
        };
    spinbox2.setup(configSpinBox2);
    SpinBoxConfig configSpinBox3 = {
            .callback = spinbox3_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 152,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR08,
            .textColor = CFK_BLACK
        };
    spinbox3.setup(configSpinBox3);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);

    ToggleButtonConfig configToggle1 = {
            .callback = motor_cb,
            .width = 83,
            .height = 42,
            .pressedColor = CFK_COLOR02
        };
    motor.setup(configToggle1);
    ToggleButtonConfig configToggle2 = {
            .callback = motor2_cb,
            .width = 83,
            .height = 42,
            .pressedColor = CFK_COLOR02
        };
    motor2.setup(configToggle2);
    ToggleButtonConfig configToggle3 = {
            .callback = motor3_cb,
            .width = 83,
            .height = 42,
            .pressedColor = CFK_COLOR02
        };
    motor3.setup(configToggle3);
    ToggleButtonConfig configToggle4 = {
            .callback = motor4_cb,
            .width = 83,
            .height = 42,
            .pressedColor = CFK_COLOR02
        };
    motor4.setup(configToggle4);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    VerticalAnalogConfig configVAnalog0 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 41,
            .height = 144,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    barramotor.setup(configVAnalog0);
    VerticalAnalogConfig configVAnalog1 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 43,
            .height = 146,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    barramotor1.setup(configVAnalog1);
    VerticalAnalogConfig configVAnalog2 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 43,
            .height = 146,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    barramotor2.setup(configVAnalog2);
    VerticalAnalogConfig configVAnalog3 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 45,
            .height = 148,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    barramotor3.setup(configVAnalog3);
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog);

    VerticalBarConfig configVBar0 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 22,
            .height = 174,
            .filledColor = CFK_COLOR01
        };
    vbar.setup(configVBar0);
    VerticalBarConfig configVBar1 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 175,
            .filledColor = CFK_COLOR03
        };
    vbar1.setup(configVBar1);
    VerticalBarConfig configVBar2 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 175,
            .filledColor = CFK_COLOR06
        };
    vbar2.setup(configVBar2);
    VerticalBarConfig configVBar3 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 174,
            .filledColor = CFK_COLOR12
        };
    vbar3.setup(configVBar3);
    VerticalBarConfig configVBar4 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 176,
            .filledColor = CFK_COLOR19
        };
    vbar4.setup(configVBar4);
    VerticalBarConfig configVBar5 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 176,
            .filledColor = CFK_COLOR22
        };
    vbar5.setup(configVBar5);
    VerticalBarConfig configVBar6 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 6,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 177,
            .filledColor = CFK_COLOR25
        };
    vbar6.setup(configVBar6);
    myDisplay.setVBar(arrayVbar,qtdVBar);

    ImageFromPixelsConfig configImage0 = {
            .pixels = iPinionpngPixels,
            .maskAlpha = iPinionpngMask,
            .cb = pinionpng_cb,
            .angle = 0,
            .width = iPinionpngW,
            .height = iPinionpngH,
            .backgroundColor = CFK_GREY5
        };
    pinionpng.setupFromPixels(configImage0);
    ImageFromPixelsConfig configImage1 = {
            .pixels = iStoppngPixels,
            .maskAlpha = iStoppngMask,
            .cb = stoppng_cb,
            .angle = 0,
            .width = iStoppngW,
            .height = iStoppngH,
            .backgroundColor = CFK_GREY5
        };
    stoppng.setupFromPixels(configImage1);
    ImageFromPixelsConfig configImage2 = {
            .pixels = iRightpngPixels,
            .maskAlpha = iRightpngMask,
            .cb = rightpng_cb,
            .angle = 0,
            .width = iRightpngW,
            .height = iRightpngH,
            .backgroundColor = CFK_GREY5
        };
    rightpng.setupFromPixels(configImage2);
    ImageFromPixelsConfig configImage3 = {
            .pixels = iHelppngPixels,
            .maskAlpha = iHelppngMask,
            .cb = helppng_cb,
            .angle = 0,
            .width = iHelppngW,
            .height = iHelppngH,
            .backgroundColor = CFK_GREY5
        };
    helppng.setupFromPixels(configImage3);
    myDisplay.setImage(arrayImagem,qtdImagem);

    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 60,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR04,
            .backgroundColor = CFK_GREY8,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_GREY5,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    load.setup(configCirculaBar0);
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 60,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR15,
            .backgroundColor = CFK_GREY8,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_GREY5,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    load1.setup(configCirculaBar1);
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 60,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR14,
            .backgroundColor = CFK_GREY8,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_GREY5,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    load2.setup(configCirculaBar2);
    CircularBarConfig configCirculaBar3 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 60,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR02,
            .backgroundColor = CFK_GREY8,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_GREY5,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    load3.setup(configCirculaBar3);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);

    ThermometerConfig configThermometer0 = {
            .subtitle = &term1,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 34,
            .height = 107,
            .filledColor = CFK_COLOR03,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer.setup(configThermometer0);
    ThermometerConfig configThermometer1 = {
            .subtitle = &term15,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 34,
            .height = 107,
            .filledColor = CFK_COLOR20,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1.setup(configThermometer1);
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);

}
// This function is a callback of this element cbgas.
// You dont need call it. Make sure it is as short and quick as possible.
void cbgas_cb(){
    check0_val = cbgas.getStatus();
}
// This function is a callback of this element cbresf.
// You dont need call it. Make sure it is as short and quick as possible.
void cbresf_cb(){
    check1_val = cbresf.getStatus();
}
// This function is a callback of this element cbsuperresf.
// You dont need call it. Make sure it is as short and quick as possible.
void cbsuperresf_cb(){
    check2_val = cbsuperresf.getStatus();
}
// This function is a callback of this element slider.
// You dont need call it. Make sure it is as short and quick as possible.
void slider_cb(){
    slider_val = slider.getValue();
    Serial.print("New value for slider is: ");Serial.println(slider_val);
}
// This function is a callback of this element slider1.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1_cb(){
    slider1_val = slider1.getValue();
    Serial.print("New value for slider is: ");Serial.println(slider1_val);
}
// This function is a callback of radio buttons of group 1.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo1(){
    radiog1 = grupo1.getSelected();
}
// This function is a callback of this element rectbutton.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton_cb(){
    bool myValue = rectbutton.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
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
    spin1_val = spinbox1.getValue();
}
// This function is a callback of this element spinbox2.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox2_cb(){
    spin2_val = spinbox2.getValue();
}
// This function is a callback of this element spinbox3.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox3_cb(){
    spin3_val = spinbox3.getValue();
}

// This function is a callback of this element motor.
// You dont need call it. Make sure it is as short and quick as possible.
void motor_cb(){
    switch1_val = motor.getStatus();
}
// This function is a callback of this element motor2.
// You dont need call it. Make sure it is as short and quick as possible.
void motor2_cb(){
    switch2_val = motor2.getStatus();
}
// This function is a callback of this element motor3.
// You dont need call it. Make sure it is as short and quick as possible.
void motor3_cb(){
    switch3_val = motor3.getStatus();
}
// This function is a callback of this element motor4.
// You dont need call it. Make sure it is as short and quick as possible.
void motor4_cb(){
    switch4_val = motor4.getStatus();
}
// This function is a callback of this element pinionpng.
// You dont need call it. Make sure it is as short and quick as possible.
void pinionpng_cb(){
    // Image Pinionpng clicked
    Serial.println("Image Pinionpng clicked");
}
// This function is a callback of this element stoppng.
// You dont need call it. Make sure it is as short and quick as possible.
void stoppng_cb(){
    // Image Stoppng clicked
    Serial.println("Image Stoppng clicked");
}
// This function is a callback of this element rightpng.
// You dont need call it. Make sure it is as short and quick as possible.
void rightpng_cb(){
    // Image Rightpng clicked
    Serial.println("Image Rightpng clicked");
}
// This function is a callback of this element helppng.
// You dont need call it. Make sure it is as short and quick as possible.
void helppng_cb(){
    // Image Helppng clicked
    Serial.println("Image Helppng clicked");
}
