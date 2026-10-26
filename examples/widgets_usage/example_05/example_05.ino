/*

--- English ---

This project is an example of using one RectButton and three Led widgets to show motor enable state
and end/start sensor status. The RectButton toggles the motor enable output; when the user touches
it and the state changes, the DisplayWX library calls btncontrolmotor_cb, which updates the enable
pin and the global motor state. The three LEDs are updated in the loop: led1 shows whether the motor
is enabled (same as the button state), led2 shows the end sensor (pin 17) and led3 shows the start
sensor (pin 18). startCustomDraw and finishCustomDraw group the LED updates so the display is
refreshed once per loop cycle.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Pin 16 drives the
motor enable output; pins 17 and 18 are inputs for the end and start sensors. On screen, one
RectButton is shown with the label "Control Motor", and three LEDs with the labels "Motor enabled",
"End sensor" and "Start sensor".

The RectButton widget toggles between pressed and released on each touch; its callback reads
getStatus(), prints to Serial and sets the motor enable pin and the motor state variable. The Led
widgets are visual only (no touch callback); their state is set in the loop with setState() from the
sensor readings and the motor enable state. Screen drawing is done in screen0 with a white
background and drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de uso de um RectButton e de três widgets Led para exibir o estado de
habilitação do motor e o status dos sensores de fim e início. O RectButton alterna a saída de
habilitação do motor; quando o usuário toca e o estado muda, a biblioteca DisplayWX chama
btncontrolmotor_cb, que atualiza o pino de enable e o estado global do motor. Os três LEDs são
atualizados no loop: led1 indica se o motor está habilitado (mesmo estado do botão), led2 indica o
sensor de fim (pino 17) e led3 o sensor de início (pino 18). startCustomDraw e finishCustomDraw
agrupam as atualizações dos LEDs para que o display seja atualizado uma vez a cada ciclo do loop.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. O pino 16 aciona a
saída de habilitação do motor; os pinos 17 e 18 são entradas dos sensores de fim e de início. Na
tela são exibidos um RectButton com o rótulo "Control Motor" e três LEDs com os rótulos "Motor
enabled", "End sensor" e "Start sensor".

O widget RectButton alterna entre pressionado e solto a cada toque; seu callback lê getStatus(),
imprime no Serial e define o pino de enable do motor e a variável de estado do motor. Os widgets Led
são apenas visuais (sem callback de toque); seu estado é definido no loop com setState() a partir das
leituras dos sensores e do estado de habilitação do motor. O desenho da tela é feito em screen0 com
fundo branco e drawWidgetsOnScreen(0).

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
void btncontrolmotor_cb();

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
Led led1(150, 110, 0);
Led led2(150, 170, 0);
Led led3(150, 220, 0);
const uint8_t qtdLed = 3;
Led *arrayLed[qtdLed] = {&led1, &led2, &led3};
RectButton btncontrolmotor(120, 35, 0);
const uint8_t qtdRectBtn = 1;
RectButton *arrayRectbtn[qtdRectBtn] = {&btncontrolmotor};


const int pinMotorEnable = 16;
const int pinEndSensor = 17;
const int pinStartSensor = 18;
const bool motorEnabledState = false;


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


    pinMode(pinMotorEnable, OUTPUT);
    pinMode(pinEndSensor, INPUT);
    pinMode(pinStartSensor, INPUT);
    digitalWrite(pinMotorEnable, motorEnabledState ? HIGH : LOW);
}

void loop(){

    bool endSensor = digitalRead(pinEndSensor);
    bool startSensor = digitalRead(pinStartSensor);
    myDisplay.startCustomDraw();
    led1.setState(motorEnabledState);
    led2.setState(endSensor);
    led3.setState(startSensor);
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an led
    myDisplay.printText("Motor enabled", 175, 102, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("End sensor", 165, 157, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Start sensor", 165, 207, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an rectButton
    myDisplay.printText("Control Motor", 185, 47, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR12,
            .colorOff = CFK_GREY3,
            .initialState = true
        };
    led1.setup(configLed0);
    LedConfig configLed1 = {
            .radius = 8,
            .colorOn = CFK_COLOR01,
            .colorOff = CFK_GREY3,
            .initialState = true
        };
    led2.setup(configLed1);
    LedConfig configLed2 = {
            .radius = 8,
            .colorOn = CFK_COLOR01,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    led3.setup(configLed2);
    myDisplay.setLed(arrayLed,qtdLed);


    RectButtonConfig configRectButton0 = {
            .callback = btncontrolmotor_cb,
            .width = 60,
            .height = 36,
            .pressedColor = CFK_COLOR16
        };

btncontrolmotor.setup(configRectButton0);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);


}

// This function is a callback of this element btncontrolmotor.
// You dont need call it. Make sure it is as short and quick as possible.
void btncontrolmotor_cb(){

    bool myValue = btncontrolmotor.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
    motorEnabledState = myValue;
    digitalWrite(pinMotorEnable, motorEnabledState ? HIGH : LOW);
}
