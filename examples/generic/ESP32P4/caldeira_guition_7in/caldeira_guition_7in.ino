#include "iClosepng.h"
// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>
// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1024;
const int DISPLAY_H = 600;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 1024;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 600;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = 22;
// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
// Prototypes for callback functions
void rectbutton_cb();
void rectbutton1_cb();
void rectbutton2_cb();
void rectbutton3_cb();
void toggle_cb();
void toggle1_cb();
void toggle2_cb();
void spinbox_cb();
void textbutton_cb();
void textbutton1_cb();
void closepng_cb();

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
DisplayFK myDisplay;

// Create global objects. Constructor is: xPos, yPos and indexScreen
GaugeSuper widget(780, 220, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&widget};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {0,20,70,80};
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR01,CFK_COLOR10,CFK_COLOR05,CFK_COLOR01};
Label term1(290, 350, 0);
Label term11(290, 395, 0);
Label term12(415, 350, 0);
Label term13(415, 395, 0);
const uint8_t qtdLabel = 4;
Label *arrayLabel[qtdLabel] = {&term1, &term11, &term12, &term13};
LineChart linechart(565, 300, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&linechart};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR28,CFK_COLOR08};
RectButton rectbutton(30, 501, 0);
RectButton rectbutton1(100, 501, 0);
RectButton rectbutton2(170, 501, 0);
RectButton rectbutton3(235, 498, 0);
const uint8_t qtdRectBtn = 4;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton, &rectbutton1, &rectbutton2, &rectbutton3};
SpinBox spinbox(320, 510, 0);
const uint8_t qtdSpinbox = 1;
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox};
TextButton textbutton(560, 465, 0);
TextButton textbutton1(560, 520, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton, &textbutton1};
ToggleButton toggle(920, 450, 0);
ToggleButton toggle1(920, 495, 0);
ToggleButton toggle2(920, 540, 0);
const uint8_t qtdToggleBtn = 3;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle, &toggle1, &toggle2};
Image closepng(980, 15, 0);
const uint8_t qtdImagem = 1;
Image *arrayImagem[qtdImagem] = {&closepng};
CircularBar circload(140, 230, 0);
CircularBar circload1(140, 230, 0);
CircularBar circload2(140, 230, 0);
const uint8_t qtdCircBar = 3;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload, &circload1, &circload2};
Thermometer thermometer(295, 120, 0);
Thermometer thermometer1(355, 120, 0);
Thermometer thermometer2(415, 120, 0);
Thermometer thermometer3(475, 120, 0);
const uint8_t qtdThermometer = 4;
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer, &thermometer1, &thermometer2, &thermometer3};

// create 7 'int count' with constexpr offset between 0 and 6
constexpr int offsetCount = 20;
int countGeral = 0, auxCountGeral = 0;
const int countSize = 8;
float counters[countSize] = {
    0,0,0,0,0,0,0,0
};

float getCurve(float x){
  //y\ =\ \sin\left(x\right)+\sin\left(3x\right)+\left(2\cdot\sin\left(x\right)\right)

  float result = sin(x * DEGTORAD) + sin(3 * x * DEGTORAD);

  // current result is from -3 to 3, need to normatize value from 0 to 100.
  // so need mutiply by 30 and add 30
  result = result * 25 + 50;

  return result;
}

void increaseCounters(){
    for (size_t i = 0; i < countSize; i++)
    {
        counters[i] = sin((countGeral + (offsetCount * i)) * DEGTORAD);
        counters[i] = counters[i] * 50 + 50;// De 0 a 100
    }
    counters[7] = getCurve(auxCountGeral);
    countGeral+=5;
    auxCountGeral++;
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

void drawHueCircleDark(int16_t cx, int16_t cy, uint16_t radius) {
  const int32_t r2 = radius * radius;

  tft->startWrite();

  for (int16_t dy = -radius; dy <= radius; dy++) {
    int32_t y2 = dy * dy;
    for (int16_t dx = -radius; dx <= radius; dx++) {
      int32_t d2 = dx * dx + y2;
      if (d2 > r2) continue; // fora do círculo

      // Distância normalizada do centro (0.0 no centro, 1.0 na borda)
      float dist = sqrtf((float)d2) / radius;

      // Matiz (ângulo)
      float hue = (atan2f(dy, dx) + PI) / (2.0f * PI);

      // Saturação sempre máxima (1.0), brilho (v) depende da distância
      float s = 1.0f;
      float v = dist; // 0 no centro (preto), 1 na borda

      uint16_t color = hsvToRgb565Fast(hue, s, v);
      tft->writePixel(cx + dx, cy + dy, color);
    }
  }

  tft->endWrite();
}

void drawHueCircleLight(int16_t cx, int16_t cy, uint16_t radius) {
  const int32_t r2 = radius * radius;

  tft->startWrite();

  for (int16_t dy = -radius; dy <= radius; dy++) {
    int32_t y2 = dy * dy;
    for (int16_t dx = -radius; dx <= radius; dx++) {
      int32_t d2 = dx * dx + y2;
      if (d2 > r2) continue;

      float s = sqrtf((float)d2 / (float)r2);           // saturação (0–1)
      float hue = (atan2f(dy, dx) + PI) / (2.0f * PI);  // matiz (0–1)

      uint16_t color = hsvToRgb565Fast(hue, s, 1.0f);
      tft->writePixel(cx + dx, cy + dy, color);
    }
  }

  tft->endWrite();
}



void testColorScreen(){
    if (!tft) return;
    tft->fillScreen(CFK_BLACK);
    drawHueCircleLight(DISPLAY_W / 2, DISPLAY_H / 2, 250);
    delay(2000);
    tft->fillScreen(CFK_WHITE);
    drawHueCircleDark(DISPLAY_W / 2, DISPLAY_H / 2, 250);
    delay(2000);
}

/*void drawTestColor(){
    // create variable uint16_t for: red, gree, blue, yellow, cyan, magenta
    const uint16_t red = 0xF800;
    const uint16_t green = 0x07E0;
    const uint16_t blue = 0x001F;
    const uint16_t yellow = 0xFFE0;
    const uint16_t cyan = 0x07FF;
    const uint16_t magenta = 0xF81F;

    // draw circle in row with radius 20px, offset 20px, starting x = offset and y = half height if screen
    uint16_t colors[] = {red, green, blue, yellow, cyan, magenta};
    uint8_t colorLen = 6;
    const int radius = 20;
    const int offset = 20;
    const int halfHeight = DISPLAY_H / 2;

    const uint8_t raio = 50;
    int intervalo = 360 / colorLen;
    int centerX = DISPLAY_W / 2;
    int centerY = DISPLAY_H / 2;


    for(int i = 0; i < colorLen; i++){
        float angulo = i * intervalo;
        float radianos = angulo * DEGTORAD;
        int x = cos(radianos) * raio + centerX;
        int y = sin(radianos) * raio + centerY;
        tft->fillCircle(x, y, radius, colors[i]);
    }
}*/

void setup(){
    Serial.begin(115200);
    bus = new Arduino_ESP32DSIPanel(
    40 /* hsync_pulse_width */, 160 /* hsync_back_porch */, 160 /* hsync_front_porch */,
    10 /* vsync_pulse_width */, 23 /*vsync_back_porch  */, 12 /* vsync_front_porch */,
    48000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    1024 /* width */, 600 /* height */, bus, rotationScreen /* rotation */, true /* auto_flush */,
    27 /* RST */, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));
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
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // myDisplay.enableTouchLog();
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    //myDisplay.startAutoClick(); // Start auto click
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    iniciarDespertador();
}
void loop(){
    if(flagAtualizarTela){
      myDisplay.startCustomDraw();

      widget.setValue(counters[5]); //Use this command to change widget value.
    linechart.push(0, counters[1]); //Use this line to add value in serie 0.
    linechart.push(1, counters[7]); //Use this line to add value in serie 1.
    circload.setValue(counters[0]); //Use this command to change widget value.
    circload1.setValue(counters[3]); //Use this command to change widget value.
    circload2.setValue(counters[5]); //Use this command to change widget value.
    thermometer.setValue(counters[6]); //Use this command to change widget value.
    
    thermometer1.setValue(counters[4]); //Use this command to change widget value.
    thermometer2.setValue(counters[2]); //Use this command to change widget value.
    thermometer3.setValue(counters[0]); //Use this command to change widget value.

        myDisplay.finishCustomDraw();// Libera a task do displayfk para desenha
        flagAtualizarTela = false;
        increaseCounters();
    }
    delay(10);
}
void screen0(){
    testColorScreen();

    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(545, 255, 468, 186, 10, CFK_GREY4);
    tft->drawRoundRect(545, 255, 468, 186, 10, CFK_BLACK);
    tft->fillRoundRect(15, 65, 255, 378, 10, CFK_GREY4);
    tft->drawRoundRect(15, 65, 255, 378, 10, CFK_BLACK);
    myDisplay.printText("Controle da sala", 355, 14, TL_DATUM, CFK_COLOR09, CFK_GREY3, &RobotoRegular20pt7b);
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    tft->fillRect(30, 355, 20, 20, CFK_COLOR01);
    tft->drawRect(30, 355, 20, 20, CFK_BLACK);
    myDisplay.printText("Setor A", 60, 357, TL_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular10pt7b);
    tft->fillRect(30, 390, 20, 20, CFK_COLOR05);
    tft->drawRect(30, 390, 20, 20, CFK_BLACK);
    myDisplay.printText("Setor B", 60, 392, TL_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular10pt7b);
    myDisplay.printText("Limite de corrente por setor (%)", 145, 86, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    tft->fillRoundRect(15, 450, 520, 134, 10, CFK_GREY4);
    tft->drawRoundRect(15, 450, 520, 134, 10, CFK_BLACK);
    tft->fillRoundRect(280, 65, 255, 378, 10, CFK_GREY4);
    tft->drawRoundRect(280, 65, 255, 378, 10, CFK_BLACK);
    tft->fillRoundRect(545, 65, 468, 176, 10, CFK_GREY4);
    tft->drawRoundRect(545, 65, 468, 176, 10, CFK_BLACK);
    //This screen has a/an ponteiroAnalog
    //This screen has a/an imagem
    myDisplay.printText("Temperatura das bombas", 405, 86, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an spinbox
    //This screen has a/an grafico
    myDisplay.printText("Historico de consumo", 790, 271, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    tft->fillRoundRect(545, 445, 466, 134, 10, CFK_GREY4);
    tft->drawRoundRect(545, 445, 466, 134, 10, CFK_BLACK);
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    myDisplay.printText("Control. temperatura", 835, 456, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Control. umidade", 850, 501, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Iluminacao", 865, 546, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Setores (A, B, C, D)", 160, 466, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Nivel ventilacao", 415, 471, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets(){
    GaugeConfig configGauge0 = {
            .title = "Pressurizador (Pa)",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 445,
            .height = 140,
            .borderColor = CFK_GREY2,
            .textColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5,
            .titleColor = CFK_WHITE,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    widget.setup(configGauge0);
    myDisplay.setGauge(arrayGauge,qtdGauge);

    LabelConfig configLabel0 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR03,
            .backgroundColor = CFK_GREY4
        };
    term1.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR05,
            .backgroundColor = CFK_GREY4
        };
    term11.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR10,
            .backgroundColor = CFK_GREY4
        };
    term12.setup(configLabel2);
    LabelConfig configLabel3 = {
            .text = "Term 1",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR19,
            .backgroundColor = CFK_GREY4
        };
    term13.setup(configLabel3);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = nullptr,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 439,
            .height = 129,
            .gridColor = CFK_BLACK,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE,
            .verticalDivision = 10,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = false,
            .showDots = false
        };
    linechart.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);

    RectButtonConfig configRectButton0 = {
            .callback = rectbutton_cb,
            .width = 53,
            .height = 70,
            .pressedColor = CFK_COLOR09
        };
rectbutton.setup(configRectButton0);
    RectButtonConfig configRectButton1 = {
            .callback = rectbutton1_cb,
            .width = 53,
            .height = 70,
            .pressedColor = CFK_COLOR09
        };
rectbutton1.setup(configRectButton1);
    RectButtonConfig configRectButton2 = {
            .callback = rectbutton2_cb,
            .width = 53,
            .height = 70,
            .pressedColor = CFK_COLOR09
        };
rectbutton2.setup(configRectButton2);
    RectButtonConfig configRectButton3 = {
            .callback = rectbutton3_cb,
            .width = 53,
            .height = 73,
            .pressedColor = CFK_COLOR09
        };
rectbutton3.setup(configRectButton3);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);

    SpinBoxConfig configSpinBox0 = {
            .callback = spinbox_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 51,
            .width = 183,
            .height = 59,
            .step = 1,
            .color = CFK_COLOR14,
            .textColor = CFK_BLACK
        };
    spinbox.setup(configSpinBox0);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);

    TextButtonConfig configTextButton0 = {
            .text = "Config",
            .callback = textbutton_cb,
            .width = 111,
            .height = 37,
            .radius = 10,
            .backgroundColor = CFK_COLOR31,
            .textColor = CFK_WHITE
        };

    textbutton.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Desl. tela",
            .callback = textbutton1_cb,
            .width = 109,
            .height = 37,
            .radius = 10,
            .backgroundColor = CFK_GREY3,
            .textColor = CFK_WHITE
        };

    textbutton1.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

    ToggleButtonConfig configToggle0 = {
            .callback = toggle_cb,
            .width = 64,
            .height = 32,
            .pressedColor = CFK_COLOR13
        };
    toggle.setup(configToggle0);
    ToggleButtonConfig configToggle1 = {
            .callback = toggle1_cb,
            .width = 64,
            .height = 32,
            .pressedColor = CFK_COLOR15
        };
    toggle1.setup(configToggle1);
    ToggleButtonConfig configToggle2 = {
            .callback = toggle2_cb,
            .width = 64,
            .height = 32,
            .pressedColor = CFK_COLOR17
        };
    toggle2.setup(configToggle2);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    ImageFromPixelsConfig configImage0 = {
            .pixels = iClosepngPixels,
            .maskAlpha = iClosepngMask,
            .cb = closepng_cb,
            .angle = 0,
            .width = iClosepngW,
            .height = iClosepngH,
            .backgroundColor = CFK_GREY3
        };
    closepng.setupFromPixels(configImage0);
    myDisplay.setImage(arrayImagem,qtdImagem);

    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 100,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR01,
            .backgroundColor = CFK_GREY5,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY4,
            .thickness = 10,
            .showValue = false,
            .inverted = false
        };
    circload.setup(configCirculaBar0);
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 85,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR05,
            .backgroundColor = CFK_GREY5,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY4,
            .thickness = 10,
            .showValue = false,
            .inverted = false
        };
    circload1.setup(configCirculaBar1);
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR10,
            .backgroundColor = CFK_GREY5,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY4,
            .thickness = 10,
            .showValue = false,
            .inverted = false
        };
    circload2.setup(configCirculaBar2);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);

    ThermometerConfig configThermometer0 = {
            .subtitle = &term1,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 38,
            .height = 210,
            .filledColor = CFK_COLOR03,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer.setup(configThermometer0);
    ThermometerConfig configThermometer1 = {
            .subtitle = &term11,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 40,
            .height = 212,
            .filledColor = CFK_COLOR05,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1.setup(configThermometer1);
    ThermometerConfig configThermometer2 = {
            .subtitle = &term12,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 40,
            .height = 212,
            .filledColor = CFK_COLOR10,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer2.setup(configThermometer2);
    ThermometerConfig configThermometer3 = {
            .subtitle = &term13,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 40,
            .height = 212,
            .filledColor = CFK_COLOR19,
            .backgroundColor = CFK_GREY5,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer3.setup(configThermometer3);
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);

}

// This function is a callback of this element rectbutton.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton_cb(){
    bool myValue = rectbutton.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton1.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1_cb(){
    bool myValue = rectbutton1.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton2.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton2_cb(){
    bool myValue = rectbutton2.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton3.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton3_cb(){
    bool myValue = rectbutton3.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element spinbox.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox_cb(){
    int myValue = spinbox.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element textbutton.
// You dont need call it. Make sure it is as short and quick as possible.
void textbutton_cb(){
    Serial.print("Clicked on: ");Serial.println("textbutton_cb");
}
// This function is a callback of this element textbutton1.
// You dont need call it. Make sure it is as short and quick as possible.
void textbutton1_cb(){
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
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
// This function is a callback of this element closepng.
// You dont need call it. Make sure it is as short and quick as possible.
void closepng_cb(){
    // Image Closepng clicked
    Serial.println("Image Closepng clicked");
}
