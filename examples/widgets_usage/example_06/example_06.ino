/*

--- English ---

This project is an example of using two RadioGroup widgets to choose the blink rate of two LEDs.
Each RadioGroup shows a set of radio buttons where only one can be selected at a time. When the user
touches a different option, the DisplayWX library calls the callback for that group; the program uses
these callbacks to update volatile variables (pulseLed1, pulseLed2) that define the delay between
blinks. Two FreeRTOS tasks (taskLed1 and taskLed2) run on core 0 and blink the LEDs on pins 16 and
17; the selected option in each group is intended to control the blink frequency for the
corresponding LED.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Pins 16 and 17 drive
the two LEDs. On screen, the first RadioGroup (grupo1) has three options labelled "Fast", "Slow" and
"Super Slow" for LED 1; the second (grupo2) has two options "Fast" and "Slow" for LED 2. A title
"Frequence pulse" and labels "LED 1" and "LED 2" identify the sections.

The RadioGroup widget displays a set of circular buttons with single selection. grupo1 has three
buttons (ids 1, 2, 3) and callback callbackOfGroup_grupo1, which reads getSelected() and sets
pulseLed1 to pulseFast, pulseSlow or pulseSuperSlow. grupo2 has two buttons (ids 1, 2) and callback
callbackOfGroup_grupo2, which sets pulseLed2 to pulseFast or pulseSlow. The main loop is empty; the
screen is drawn in screen0 with a white background and drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de uso de dois widgets RadioGroup para escolher a frequência de piscar de
dois LEDs. Cada RadioGroup exibe um conjunto de botões de rádio dos quais apenas um pode estar
selecionado por vez. Quando o usuário toca em uma opção diferente, a biblioteca DisplayWX chama o
callback daquele grupo; o programa usa essas callbacks para atualizar variáveis voláteis (pulseLed1,
pulseLed2) que definem o intervalo entre os piscar. Duas tarefas FreeRTOS (taskLed1 e taskLed2) rodam
no core 0 e piscam os LEDs nos pinos 16 e 17; a opção selecionada em cada grupo define a
frequência de piscar do LED correspondente.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. Os pinos 16 e 17
acionam os dois LEDs. Na tela, o primeiro RadioGroup (grupo1) tem três opções com os rótulos "Fast",
"Slow" e "Super Slow" para o LED 1; o segundo (grupo2) tem duas opções "Fast" e "Slow" para o LED 2.
Um título "Frequence pulse" e os rótulos "LED 1" e "LED 2" identificam as seções.

O widget RadioGroup exibe um conjunto de botões circulares com seleção única. O grupo1 tem três
botões (ids 1, 2, 3) e o callback callbackOfGroup_grupo1, que lê getSelected() e define pulseLed1
como pulseFast, pulseSlow ou pulseSuperSlow. O grupo2 tem dois botões (ids 1, 2) e o callback
callbackOfGroup_grupo2, que define pulseLed2 como pulseFast ou pulseSlow. O loop principal fica
vazio; a tela é desenhada em screen0 com fundo branco e drawWidgetsOnScreen(0).

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
void callbackOfGroup_grupo1();
void callbackOfGroup_grupo2();

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
RadioGroup grupo1(0);
RadioGroup grupo2(0);
const uint8_t qtdRadioGroup = 2;
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1, &grupo2};
radio_t radiosOfGroup_grupo1[3] = {{65, 130, CFK_COLOR30, 1},{65, 170, CFK_COLOR30, 2},{65, 215, CFK_COLOR30, 3}};
radio_t radiosOfGroup_grupo2[2] = {{290, 130, CFK_COLOR30, 1},{290, 175, CFK_COLOR30, 2}};

const int pinLed1 = 16;
const int pinLed2 = 17;

const int pulseFast = 100;
const int pulseSlow = 200;
const int pulseSuperSlow = 500;


volatile int pulseLed1 = pulseFast;
volatile int pulseLed2 = pulseFast;


TaskHandle_t taskLed1Handle;
TaskHandle_t taskLed2Handle;

void taskLed1(){
    pinMode(pinLed1, OUTPUT);
    while(true){
        digitalWrite(pinLed1, HIGH);
        delay(pulseFast);
        digitalWrite(pinLed1, LOW);
        delay(pulseFast);
    }
}

void taskLed2(){
    pinMode(pinLed2, OUTPUT);
    while(true){
        digitalWrite(pinLed2, HIGH);
        delay(pulseFast);
        digitalWrite(pinLed2, LOW);
        delay(pulseFast);
    }
}


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

    xTaskCreatePinnedToCore(taskLed1, "taskLed1", 10000, NULL, 1, &taskLed1Handle, 0);
    xTaskCreatePinnedToCore(taskLed2, "taskLed2", 10000, NULL, 1, &taskLed2Handle, 0);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Frequence pulse", 145, 27, TL_DATUM, CFK_COLOR04, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    myDisplay.printText("Fast", 85, 122, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Slow", 85, 162, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Super Slow", 85, 202, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("LED 1", 90, 82, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("LED 2", 305, 82, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    myDisplay.printText("Fast", 310, 122, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Slow", 310, 167, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    RadioGroupConfig configRadioGroup0 = {
            .buttons = radiosOfGroup_grupo1,
            .callback = callbackOfGroup_grupo1,
            .radius = 10,
            .group = 1,
            .amount = 3,
            .defaultClickedId = 1
        };
    grupo1.setup(configRadioGroup0);
    RadioGroupConfig configRadioGroup1 = {
            .buttons = radiosOfGroup_grupo2,
            .callback = callbackOfGroup_grupo2,
            .radius = 10,
            .group = 2,
            .amount = 2,
            .defaultClickedId = 1
        };
    grupo2.setup(configRadioGroup1);
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);


}

// This function is a callback of radio buttons of group 1.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo1(){

    uint16_t selectedOption = grupo1.getSelected();
    Serial.print("New value for radiogroup is: ");Serial.println(selectedOption);
    switch(selectedOption){
        case 1:
            pulseLed1 = pulseFast;
            break;
        case 2:
            pulseLed1 = pulseSlow;
            break;
        case 3:
            pulseLed1 = pulseSuperSlow;
            break;
        default:
            pulseLed1 = pulseFast;
            break;
    }
}
// This function is a callback of radio buttons of group 2.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo2(){

    uint16_t selectedOption = grupo2.getSelected();
    Serial.print("New value for radiogroup is: ");Serial.println(selectedOption);
    switch(selectedOption){
        case 1:
            pulseLed2 = pulseFast;
            break;
        case 2:
            pulseLed2 = pulseSlow;
            break;
        default:
            pulseLed2 = pulseFast;
            break;
    }
}
