// ESP32S3 - 8048043




// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define TC_SCL	20
#define TC_SDA	19
#define TC_INT	-1
#define TC_RST	38
#define TOUCH_ROTATION	ROTATION_NORMAL
#define TOUCH_MAP_X0	0
#define TOUCH_MAP_X1	800
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	480
#define TOUCH_SWAP_XY	false
#define TOUCH_INVERT_X	false
#define TOUCH_INVERT_Y	false

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void statusmotor_cb();
void tbvelocidade_cb();
void savepng_cb();
void savepng1_cb();
void recuperarDados();
void salvarStatusMotor();
void salvarVelocidade();
void limparLog();
void mostrarLog(const char* mensagem);

#include "Savepng.h"
// Include for plugins of chip 1
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>
#include <Preferences.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
#define DISP_LED 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
Label txtlog(360, 430, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&txtlog};
NumberBox tbvelocidade(274, 184, 0);
const uint8_t qtdNumberbox = 1;
NumberBox *arrayNumberbox[qtdNumberbox] = {&tbvelocidade};
float nb0_val = 0; // Global variable that stores the value of the widget tbvelocidade
ToggleButton statusmotor(340, 330, 0);
const uint8_t qtdToggleBtn = 1;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&statusmotor};
bool  switch0_val = false; // Global variable that stores the value of the widget statusmotor
Image savepng(454, 179, 0);
Image savepng1(450, 310, 0);
const uint8_t qtdImagem = 2;
Image *arrayImagem[qtdImagem] = {&savepng, &savepng1};

Preferences config;
const char* nomeProjeto = "myproject";

int tempoClearLog = 5000;
int currentTempoLog= 0;
bool logComMensagem = false;

void recuperarDados(){
    config.begin(nomeProjeto, true);//readonly true
    nb0_val = config.getFloat("tbvelocidade", 0);
    switch0_val = config.getBool("statusmotor", false);
    config.end();

    tbvelocidade.setValue(nb0_val);
    statusmotor.setStatus(switch0_val);
    mostrarLog("Valores lidos!");
}

void salvarStatusMotor(){
    config.begin(nomeProjeto, false);
    config.putBool("statusmotor", statusmotor.getStatus());
    config.end();
    mostrarLog("Status do motor salvo!");
}

void salvarVelocidade(){
    config.begin(nomeProjeto, false);
    config.putFloat("tbvelocidade", tbvelocidade.getValue());
    config.end();
    mostrarLog("Velocidade salva!");
}

void mostrarLog(const char* mensagem){
    txtlog.setText(mensagem);
    currentTempoLog = millis() + tempoClearLog;
    logComMensagem = true;
}   

void limparLog(){
    txtlog.setText("");
    logComMensagem = false;
}

void setup(){
    Serial.begin(115200);
    rgbpanel = new Arduino_ESP32RGBPanel(
    40, 41, 39, 42,
    45, 48, 47, 21, 14,
    5, 6, 7, 15, 16, 4,
    8, 3, 46, 9, 1,
    0, 8, 4, 8,
    0, 8, 4, 8,
    1, 16000000, false,
    0, 0, 0);
    tft = new Arduino_RGB_Display(800, 480, rgbpanel, 0 , true);
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
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw

    recuperarDados();
}

void loop(){
    // Use the variable nb0_val to check value/status of widget tbvelocidade
    // Use the variable switch0_val to check value/status of widget statusmotor
    delay(1000);

    if(currentTempoLog < millis() && logComMensagem){
        limparLog();
    }
}

void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(250, 260, 261, 122, 10, CFK_GREY5);
    tft->drawRoundRect(250, 260, 261, 122, 10, CFK_COLOR01);
    tft->fillRoundRect(250, 125, 261, 122, 10, CFK_GREY5);
    tft->drawRoundRect(250, 125, 261, 122, 10, CFK_COLOR01);
    myDisplay.printText("PREFERENCES", 245, 19, TL_DATUM, CFK_COLOR01, CFK_GREY3, &RobotoBold20pt7b);
    //This screen has a/an caixaNumero
    myDisplay.printText("Velocidade", 299, 151, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular10pt7b);
    //This screen has a/an imagem
    //This screen has a/an toggleButton
    myDisplay.printText("Motor", 350, 277, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular10pt7b);
    //This screen has a/an imagem
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR04,
            .backgroundColor = CFK_GREY3
        };
    txtlog.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    Numpad::m_backgroundColor = CFK_GREY3;
    Numpad::m_letterColor = CFK_BLACK;
    Numpad::m_keyColor = CFK_GREY13;

    NumberBoxConfig configNumberBox0 = {
            .funcPtr = screen0,
            .callback = tbvelocidade_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = nb0_val,
            .width = 150,
            .height = 25,
            .letterColor = CFK_COLOR06,
            .backgroundColor = CFK_GREY5
        };
    tbvelocidade.setup(configNumberBox0);
    myDisplay.setNumberbox(arrayNumberbox,qtdNumberbox);

    ToggleButtonConfig configToggle0 = {
            .callback = statusmotor_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR15
        };
    statusmotor.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    ImageFromPixelsConfig configImage0 = {
            .pixels = SavepngPixels,
            .maskAlpha = SavepngMask,
            .cb = savepng_cb,
            .angle = 0,
            .width = SavepngW,
            .height = SavepngH
        };
    savepng.setup(configImage0);
    ImageFromPixelsConfig configImage1 = {
            .pixels = SavepngPixels,
            .maskAlpha = SavepngMask,
            .cb = savepng1_cb,
            .angle = 0,
            .width = SavepngW,
            .height = SavepngH
        };
    savepng1.setup(configImage1);
    myDisplay.setImage(arrayImagem,qtdImagem);

}

// This function is a callback of this element tbvelocidade.
// You dont need call it. Make sure it is as short and quick as possible.
void tbvelocidade_cb(){
    nb0_val = tbvelocidade.getValue();
    
}
// This function is a callback of this element statusmotor.
// You dont need call it. Make sure it is as short and quick as possible.
void statusmotor_cb(){
    switch0_val = statusmotor.getStatus();
    
}

void savepng_cb(){
    // Image Savepng clicked
    Serial.println("Image Savepng clicked");
    salvarVelocidade();
}
// This function is a callback of this element savepng1.
// You dont need call it. Make sure it is as short and quick as possible.
void savepng1_cb(){
    // Image Savepng clicked
    Serial.println("Image Savepng clicked");
    salvarStatusMotor();
}