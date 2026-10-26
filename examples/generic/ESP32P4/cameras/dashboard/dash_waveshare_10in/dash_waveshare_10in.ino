#include "iwebcampng.h"
// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>
#include <ESP_Video.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: JD9365 - DSI
    * Touch: GSL3680
    * Camera: OV02C10
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1280;
const int DISPLAY_H = 800;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int CAM_SDA = 7;
const int CAM_SCL = 8;
const int CAM_FREQUENCY_SCCB = 400000;
const int CAM_INT = -1;
const int CAM_RST = -1;
const int CAM_WIDTH = 1920;
const int CAM_HEIGHT = 1080;
const int CAM_I2C_PORT = 0;
const bool CAM_MIRROR_X = false;
const bool CAM_MIRROR_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void toggle1_cb();
void slidervelocidad_cb();
void iwebcampng_cb();
void btnligar_cb();
void btnparar_cb();

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayFK myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
GaugeSuper gauge(285, 640, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&gauge};
const uint8_t qtdIntervalG0 = 3;
int range0[qtdIntervalG0] = {0,60,80};
uint16_t colors0[qtdIntervalG0] = {CFK_WHITE,CFK_COLOR019,CFK_COLOR013};
HSlider slidervelocidad(125, 450, 0);
const uint8_t qtdHSlider = 1;
HSlider *arrayHslider[qtdHSlider] = {&slidervelocidad};
int hslider0_val = 0; // Global variable that stores the value of the widget slidervelocidad
Label txttotal(1010, 125, 0);
Label txtdanificado(1010, 230, 0);
Label txttempo(1005, 320, 0);
const uint8_t qtdLabel = 3;
Label *arrayLabel[qtdLabel] = {&txttotal, &txtdanificado, &txttempo};
Led ledred(690, 120, 0);
Led ledgreen(745, 120, 0);
const uint8_t qtdLed = 2;
Led *arrayLed[qtdLed] = {&ledred, &ledgreen};
TextButton btnligar(510, 160, 0);
TextButton btnparar(495, 680, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btnligar, &btnparar};
ToggleButton toggle1(695, 310, 0);
const uint8_t qtdToggleBtn = 1;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1};
bool  switch0_val = false; // Global variable that stores the value of the widget toggle1
VBar vertbar1(760, 445, 0);
VBar vertbar1copy(860, 445, 0);
VBar vertbar1copy2(965, 445, 0);
VBar vertbar1copy3(1065, 445, 0);
const uint8_t qtdVBar = 4;
VBar *arrayVbar[qtdVBar] = {&vertbar1, &vertbar1copy, &vertbar1copy2, &vertbar1copy3};
Image iwebcampng(360, 95, 0);
const uint8_t qtdImagem = 1;
Image *arrayImagem[qtdImagem] = {&iwebcampng};
CameraMipi cameraMipi(44, 143, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

uint32_t tempoTotal = 0;
uint32_t intervalo = 1000;
uint32_t totalPecas = 0;
uint32_t tempoFuncionamento = 0;
uint32_t totalPecasDanificadas = 0;
bool esteiraLigada = false;


void setStatusLed(bool status){
    if(status){
        ledred.setState(false);
        ledgreen.setState(true);
    }else{
        ledred.setState(true);
        ledgreen.setState(false);
    }
}

// linha de camera
void setup(){

    Serial.begin(115200);
    // Setup camera
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        myDisplay.getCamera().startPpa();
    }else{
        Serial.println("Camera not started");
    }
    bus = new Arduino_ESP32DSIPanel(
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    60000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    800 /* width default */, 1280 /* height default */, bus, rotationScreen, true,
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
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    setStatusLed(false);
}



void updateContadores(){
    if(!esteiraLigada){
        return;
    }

    if(millis() - tempoTotal >= intervalo){
        tempoTotal = millis();
        tempoFuncionamento++;
        totalPecas++;

        int sorteio = random(0, 100);
        totalPecasDanificadas += sorteio % 2;

        txttotal.setTextInt(totalPecas);
        txtdanificado.setTextInt(totalPecasDanificadas);


        char tempoBuf[6];
        snprintf(tempoBuf, sizeof(tempoBuf), "%02u:%02u",
                 (unsigned)((tempoFuncionamento / 60u) % 100u),
                 (unsigned)(tempoFuncionamento % 60u));
        const char *tempoFmt = tempoBuf;
        txttempo.setText(tempoFmt);
    }
}

void loop(){

    myDisplay.startCustomDraw();

    updateContadores();

    vertbar1.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    myDisplay.finishCustomDraw();

    delay(100);
}

void screen0(){

    tft->fillScreen(CFK_GREY4);
    WidgetBase::backgroundColor = CFK_GREY4;
    tft->fillRoundRect(30, 85, 380, 292, 10, CFK_GREY3);
    tft->drawRoundRect(30, 85, 380, 292, 10, CFK_BLACK);
    tft->fillRoundRect(430, 85, 380, 292, 10, CFK_GREY3);
    tft->drawRoundRect(430, 85, 380, 292, 10, CFK_BLACK);
    tft->fillRoundRect(830, 80, 425, 292, 10, CFK_GREY3);
    tft->drawRoundRect(830, 80, 425, 292, 10, CFK_BLACK);
    tft->fillRoundRect(25, 395, 1223, 262, 10, CFK_GREY3);
    tft->drawRoundRect(25, 395, 1223, 262, 10, CFK_BLACK);
    tft->fillRoundRect(25, 670, 1223, 122, 10, CFK_GREY3);
    tft->drawRoundRect(25, 670, 1223, 122, 10, CFK_BLACK);
    myDisplay.printText("Controle de Esteira", 475, 24, TL_DATUM, CFK_COLOR013, CFK_GREY4, &RobotoRegular20pt7b);
    myDisplay.printText("Visualizar", 170, 97, TL_DATUM, CFK_COLOR013, CFK_GREY3, &RobotoRegular12pt7b);
    //This screen has a/an imagem
    //This screen has a/an cameraMipi
    //This screen has a/an textButton
    //This screen has a/an toggleButton
    //This screen has a/an led
    //This screen has a/an led
    myDisplay.printText("Status", 590, 107, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("Chave", 625, 327, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("Total", 940, 117, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("Danificados", 885, 217, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    myDisplay.printText("Tempo", 925, 307, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    //This screen has a/an texto
    //This screen has a/an texto
    //This screen has a/an texto
    //This screen has a/an slider
    //This screen has a/an ponteiroAnalog
    myDisplay.printText("Velocidade", 245, 412, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    GaugeConfig configGauge0 = {
            .title = "Velocidade",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoRegular8pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 400,
            .height = 130,
            .borderColor = CFK_COLOR007,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .titleColor = CFK_BLACK,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    gauge.setup(configGauge0);
    myDisplay.setGauge(arrayGauge,qtdGauge);


    HSliderConfig configHSlider0 = {
            .callback = slidervelocidad_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 22,
            .width = 323,
            .pressedColor = CFK_COLOR007,
            .backgroundColor = CFK_WHITE,
        };
    slidervelocidad.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);


    LabelConfig configLabel0 = {
            .text = "9999",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular25pt7b,
            .datum = ML_DATUM,
            .fontColor = CFK_COLOR019,
            .backgroundColor = CFK_GREY3,
        };
    txttotal.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "9999",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular20pt7b,
            .datum = ML_DATUM,
            .fontColor = CFK_COLOR013,
            .backgroundColor = CFK_GREY3,
        };
    txtdanificado.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "00:00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &G7_Segment7_S520pt7b,
            .datum = ML_DATUM,
            .fontColor = CFK_COLOR043,
            .backgroundColor = CFK_GREY3,
        };
    txttempo.setup(configLabel2);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR007,
            .colorOff = CFK_GREY3,
            .initialState = true
        };
    ledred.setup(configLed0);
    LedConfig configLed1 = {
            .radius = 16,
            .colorOn = CFK_COLOR031,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    ledgreen.setup(configLed1);
    myDisplay.setLed(arrayLed,qtdLed);


    TextButtonConfig configTextButton0 = {
            .text = "INICIAR",
            .callback = btnligar_cb,
            .fontFamily = &RobotoBold20pt7b,
            .width = 217,
            .height = 106,
            .radius = 10,
            .backgroundColor = CFK_COLOR031,
            .textColor = CFK_BLACK
        };

    btnligar.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "PARAR",
            .callback = btnparar_cb,
            .fontFamily = &RobotoBold25pt7b,
            .width = 280,
            .height = 104,
            .radius = 10,
            .backgroundColor = CFK_COLOR007,
            .textColor = CFK_WHITE
        };

    btnparar.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


    ToggleButtonConfig configToggle0 = {
            .callback = toggle1_cb,
            .width = 100,
            .height = 50,
            .pressedColor = CFK_COLOR031
        };
    toggle1.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    VerticalBarConfig configVBar0 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1.setup(configVBar0);
    VerticalBarConfig configVBar1 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy.setup(configVBar1);
    VerticalBarConfig configVBar2 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy2.setup(configVBar2);
    VerticalBarConfig configVBar3 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy3.setup(configVBar3);
    myDisplay.setVBar(arrayVbar,qtdVBar);


    ImageFromPixelsConfig configImage0 = {
            .pixels = iwebcampngPixels,
            .maskAlpha = iwebcampngMask,
            .cb = iwebcampng_cb,
            .angle = 0,
            .width = iwebcampngW,
            .height = iwebcampngH,
            .backgroundColor = CFK_GREY3
        };
    iwebcampng.setupFromPixels(configImage0);
    myDisplay.setImage(arrayImagem,qtdImagem);


    CameraMipiConfig configCameraMipi0 = {
            .width = 353,
            .height = 223,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


}

// This function is a callback of this element slidervelocidad.
// You dont need call it. Make sure it is as short and quick as possible.
void slidervelocidad_cb(){
    hslider0_val = slidervelocidad.getValue();
    gauge.setValue(hslider0_val);
}
// This function is a callback of this element btnligar.
// You dont need call it. Make sure it is as short and quick as possible.
void btnligar_cb(){

    if(switch0_val){
        esteiraLigada = true;
        tempoTotal = 0;
        totalPecas = 0;
        totalPecasDanificadas = 0;
        tempoFuncionamento = 0;
        setStatusLed(true);
    }

    Serial.print("Clicked on: ");Serial.println("btnligar_cb");
}
// This function is a callback of this element btnparar.
// You dont need call it. Make sure it is as short and quick as possible.
void btnparar_cb(){
    esteiraLigada = false;
    setStatusLed(false);
    Serial.print("Clicked on: ");Serial.println("btnparar_cb");
}
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){

    switch0_val = toggle1.getStatus();
}
// This function is a callback of this element iwebcampng.
// You dont need call it. Make sure it is as short and quick as possible.
void iwebcampng_cb(){
    cameraMipi.openConfig();
    // Image Webcampng clicked
    Serial.println("Image Webcampng clicked");
}
