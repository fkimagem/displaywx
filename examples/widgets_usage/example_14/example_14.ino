/*

--- English ---

This project demonstrates an ADC alarm: the user sets a threshold with a SpinBox (0–4095, step 1), and
the loop reads an analog input (pin 16). When the reading is greater than or equal to the threshold,
a buzzer (pin 17) is turned on; otherwise it is off. The SpinBox callback (spinbox1_cb) reads the
value with getValue() and is intended to update the alarm threshold so the user can change it at
runtime. The screen shows the title "ADC Alarm" and the caption "ADC max value before alarm".

The SpinBox widget is a numeric input control with increment and decrement buttons; the user taps
the buttons to change the value within a configured min, max and step. When the value changes, the
library calls the widget's callback; the callback reads the new value with getValue() and can store
it (e.g. as the alarm threshold). The threshold variable must not be declared const if the callback
assigns a new value to it; otherwise the code will not compile.

The display uses ST7796 and FT6336U on ESP32S3.

--- Português ---

Este projeto demonstra um alarme por ADC: o usuário define um limiar com um SpinBox (0–4095, passo 1)
e o loop lê uma entrada analógica (pino 16). Quando a leitura é maior ou igual ao limiar, um buzzer
(pino 17) é ligado; caso contrário fica desligado. O callback do SpinBox (spinbox1_cb) lê o valor com
getValue() e tem a intenção de atualizar o limiar do alarme para que o usuário possa alterá-lo em
tempo de execução. A tela exibe o título "ADC Alarm" e a legenda "ADC max value before alarm".

O widget SpinBox é um controle de entrada numérica com botões de incremento e decremento; o usuário
toca nos botões para alterar o valor dentro do min, max e step configurados. Quando o valor muda, a
biblioteca chama o callback do widget; o callback lê o novo valor com getValue() e pode armazená-lo
(ex.: como limiar do alarme). A variável do limiar não deve ser declarada const se o callback atribuir
um novo valor a ela; caso contrário o código não compilará.

O display usa ST7796 e FT6336U no ESP32S3.

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
void spinbox1_cb();

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
SpinBox spinbox1(125, 125, 0);
const uint8_t qtdSpinbox = 1;
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox1};

const int pinAdc = 16;
int valueAdcAlarm = 1000;
const int pinBuzzer = 17;

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

    pinMode(pinBuzzer, OUTPUT);
    pinMode(pinAdc, INPUT);
}

void loop(){
    int valueAdc = analogRead(pinAdc);
    if(valueAdc >= valueAdcAlarm){
        digitalWrite(pinBuzzer, HIGH);
    }else{
        digitalWrite(pinBuzzer, LOW);
    }

    delay(1000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("ADC Alarm", 185, 32, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an spinbox
    myDisplay.printText("ADC mac value before alarm", 100, 92, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    SpinBoxConfig configSpinBox0 = {
            .callback = spinbox1_cb,
            .minValue = 0,
            .maxValue = 4095,
            .startValue = 50,
            .width = 216,
            .height = 62,
            .step = 1,
            .color = CFK_COLOR19,
            .textColor = CFK_BLACK
        };
    spinbox1.setup(configSpinBox0);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);


}

// This function is a callback of this element spinbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox1_cb(){

    int myValue = spinbox1.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
    valueAdcAlarm = myValue;
}
