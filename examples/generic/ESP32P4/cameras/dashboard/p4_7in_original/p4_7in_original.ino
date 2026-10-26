// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <ESP_Video.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: EK79007 - DSI
    * Touch: GT911
    * Camera: SC2336
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 1024;
const int DISPLAY_H = 600;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 1024;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 600;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = true;
const bool TOUCH_INVERT_Y = true;
const int CAM_SDA = 7;
const int CAM_SCL = 8;
const int CAM_FREQUENCY_SCCB = 400000;
const int CAM_INT = -1;
const int CAM_RST = -1;
const int CAM_WIDTH = 1920;
const int CAM_HEIGHT = 1080;
const int CAM_I2C_PORT = 0;
const bool CAM_MIRROR_X = true;
const bool CAM_MIRROR_Y = true;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void toggle1_cb();
void toggle2_cb();
void toggle3_cb();
void esvaziar1_cb();
void esvaziar2_cb();
void esvaziar3_cb();

#define DISP_LED 26
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
TextButton esvaziar1(810, 95, 0);
TextButton esvaziar2(810, 185, 0);
TextButton esvaziar3(810, 275, 0);
const uint8_t qtdTextButton = 3;
TextButton *arrayTextButton[qtdTextButton] = {&esvaziar1, &esvaziar2, &esvaziar3};
ToggleButton toggle1(170, 135, 0);
ToggleButton toggle2(170, 210, 0);
ToggleButton toggle3(170, 275, 0);
const uint8_t qtdToggleBtn = 3;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1, &toggle2, &toggle3};
bool  switch0_val = false; // Global variable that stores the value of the widget toggle1
bool  switch1_val = false; // Global variable that stores the value of the widget toggle2
bool  switch2_val = false; // Global variable that stores the value of the widget toggle3
CircularBar reservatorio1(275, 460, 0);
CircularBar reservatorio2(510, 460, 0);
CircularBar reservatorio3(765, 460, 0);
const uint8_t qtdCircBar = 3;
CircularBar *arrayCircularbar[qtdCircBar] = {&reservatorio1, &reservatorio2, &reservatorio3};
CameraMipi cameraMipi(295, 80, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

int totalTanque1 = 0;
int totalTanque2 = 0;
int totalTanque3 = 0;

uint32_t intervaloEncherTanque = 500;
uint32_t intervaloEsvaziarTanque = 600;

uint32_t timerTanque1 = 0;
uint32_t timerTanque2 = 0;
uint32_t timerTanque3 = 0;

enum class EstadoTanque : uint8_t {
    VAZIO = 0,
    CHEIO = 1,
    ENCHENDO = 2,
    ESVAZIANDO = 3,
    PARADO = 4
};

EstadoTanque estadoTanque1 = EstadoTanque::PARADO;
EstadoTanque estadoTanque2 = EstadoTanque::PARADO;
EstadoTanque estadoTanque3 = EstadoTanque::PARADO;


void loopTanque(EstadoTanque &estado, uint32_t &timer, int &total, uint32_t intervaloEncher, uint32_t intervaloEsvaziar, CircularBar &circularBar){
    
    switch(estado){
        case EstadoTanque::VAZIO:
            break;
        case EstadoTanque::CHEIO:
            break;
        case EstadoTanque::ENCHENDO:
        {
            if(millis() - timer >= intervaloEncher){
                total++;
                circularBar.setValue(total);
                timer = millis();
                if(total >= 100){
                    estado = EstadoTanque::CHEIO;
                }
            }
        }
            break;
        case EstadoTanque::ESVAZIANDO:{
            if(millis() - timer >= intervaloEsvaziar){
                total--;
                circularBar.setValue(total);
                timer = millis();
                if(total <= 0){
                    estado = EstadoTanque::VAZIO;
                }
            }
        }
            break;
        case EstadoTanque::PARADO:
            break;
    }
}

void loopTanque1(){
    loopTanque(estadoTanque1, timerTanque1, totalTanque1, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio1);
}

void loopTanque2(){
    loopTanque(estadoTanque2, timerTanque2, totalTanque2, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio2);
}

void loopTanque3(){
    loopTanque(estadoTanque3, timerTanque3, totalTanque3, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio3);
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
    10, 160,160 ,
    1, 23, 12,
    52000000);
    tft = new Arduino_DSI_Display(
    1024 /* width default */, 600 /* height default */, bus, rotationScreen, true,
    DISP_RST, ek79007_init_operations, sizeof(ek79007_init_operations) / sizeof(lcd_init_cmd_t));
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
    myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

    myDisplay.startCustomDraw();
    loopTanque1();
    loopTanque2();
    loopTanque3();
    myDisplay.finishCustomDraw();
    delay(50);
}

void screen0(){

    tft->fillScreen(CFK_COLOR046);
    WidgetBase::backgroundColor = CFK_COLOR046;
    tft->fillRoundRect(280, 65, 481, 295, 20, CFK_COLOR046);
    tft->drawRoundRect(280, 65, 481, 295, 20, CFK_COLOR044);
    myDisplay.printText("Monitoramento de tanques", 525, 14, TC_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular20pt7b);
    tft->fillRoundRect(10, 65, 256, 295, 20, CFK_COLOR046);
    tft->drawRoundRect(10, 65, 256, 295, 20, CFK_COLOR044);
    tft->fillRoundRect(776, 65, 230, 295, 20, CFK_COLOR046);
    tft->drawRoundRect(776, 65, 230, 295, 20, CFK_COLOR044);
    tft->fillRoundRect(10, 376, 995, 209, 20, CFK_COLOR046);
    tft->drawRoundRect(10, 376, 995, 209, 20, CFK_COLOR044);
    //This screen has a/an cameraMipi
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    myDisplay.printText("Reservatorio 1", 205, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    myDisplay.printText("Reservatorio 2", 440, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    myDisplay.printText("Reservatorio 3", 700, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    myDisplay.printText("Reservatorio 1", 25, 142, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    myDisplay.printText("Reservatorio 2", 25, 217, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    myDisplay.printText("Reservatorio 3", 25, 287, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    myDisplay.printText("Encher", 105, 87, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    TextButtonConfig configTextButton0 = {
            .text = "Esvaziar 1",
            .callback = esvaziar1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 151,
            .height = 58,
            .radius = 10,
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    esvaziar1.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Esvaziar 2",
            .callback = esvaziar2_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 151,
            .height = 58,
            .radius = 10,
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    esvaziar2.setup(configTextButton1);
    TextButtonConfig configTextButton2 = {
            .text = "Esvaziar 3",
            .callback = esvaziar3_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 151,
            .height = 58,
            .radius = 10,
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    esvaziar3.setup(configTextButton2);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


    ToggleButtonConfig configToggle0 = {
            .callback = toggle1_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle1.setup(configToggle0);
    ToggleButtonConfig configToggle1 = {
            .callback = toggle2_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle2.setup(configToggle1);
    ToggleButtonConfig configToggle2 = {
            .callback = toggle3_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle3.setup(configToggle2);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            .thickness = 30,
            .showValue = true,
            .inverted = false
        };
    reservatorio1.setup(configCirculaBar0);
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            .thickness = 30,
            .showValue = true,
            .inverted = false
        };
    reservatorio2.setup(configCirculaBar1);
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            .thickness = 30,
            .showValue = true,
            .inverted = false
        };
    reservatorio3.setup(configCirculaBar2);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);


    CameraMipiConfig configCameraMipi0 = {
            .width = 452,
            .height = 264,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


    reservatorio1.setValue(0);
    reservatorio2.setValue(0);
    reservatorio3.setValue(0);


}

// This function is a callback of this element esvaziar1.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar1_cb(){
    toggle1.setStatus(false, false);
    estadoTanque1 = EstadoTanque::ESVAZIANDO;
    Serial.print("Clicked on: ");Serial.println("esvaziar1_cb");
}
// This function is a callback of this element esvaziar2.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar2_cb(){
    toggle2.setStatus(false, false);
    estadoTanque2 = EstadoTanque::ESVAZIANDO;
    Serial.print("Clicked on: ");Serial.println("esvaziar2_cb");
}
// This function is a callback of this element esvaziar3.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar3_cb(){
    toggle3.setStatus(false, false);
    estadoTanque3 = EstadoTanque::ESVAZIANDO;
    Serial.print("Clicked on: ");Serial.println("esvaziar3_cb");
}
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){
    
    switch0_val = toggle1.getStatus();
    estadoTanque1 = switch0_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
// This function is a callback of this element toggle2.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle2_cb(){

    switch1_val = toggle2.getStatus();
    estadoTanque2 = switch1_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
// This function is a callback of this element toggle3.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle3_cb(){

    switch2_val = toggle3.getStatus();
    estadoTanque3 = switch2_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
