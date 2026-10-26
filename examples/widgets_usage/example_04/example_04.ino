/*

--- English ---

This project is an example of using four CircleButton widgets to control a stepper motor driver (e.g.
A4988 or DRV8825). Three buttons set the microstep pins M0, M1 and M2; a fourth button enables or
disables the driver and starts/stops the step pulse task. When the user touches a button and its
state changes, the DisplayWX library calls the associated callback; the program uses these callbacks
to drive the driver pins and to resume or suspend the FreeRTOS task that toggles the STEP pin.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Output pins 16, 17
and 18 drive M0, M1, M2; pin 19 is ENABLE; pins 20 and 21 are STEP and DIR. A task runs on core 0
generating step pulses; when the Enable button is pressed the task is resumed, when released it is
suspended. On screen, four CircleButtons are shown with labels "M0", "M1", "M2" and "Enable".

The CircleButton widget is a circular button that shows a visual change when pressed. Each of the
four buttons (btnm0, btnm1, btnm2, btnenable) is configured in loadWidgets with a callback. The
callbacks btnm0_cb, btnm1_cb and btnm2_cb read getStatus() and set the M0, M1, M2 pins; btnenable_cb
sets the ENABLE pin and calls xTaskResume or xTaskSuspend on the stepper task. All action is
triggered by touch and handled in the callbacks; the main loop is empty. Screen drawing is done in
screen0 with a white background and drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de uso de quatro widgets CircleButton para controlar um driver de motor de
passo (por exemplo A4988 ou DRV8825). Três botões definem os pinos de microstep M0, M1 e M2; um
quarto botão habilita ou desabilita o driver e inicia ou para a tarefa de pulsos de step. Quando o
usuário toca em um botão e o estado muda, a biblioteca DisplayWX chama o callback associado; o
programa usa essas callbacks para acionar os pinos do driver e para retomar ou suspender a tarefa
FreeRTOS que alterna o pino STEP.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. Os pinos de saída 16,
17 e 18 acionam M0, M1 e M2; o pino 19 é ENABLE; os pinos 20 e 21 são STEP e DIR. Uma tarefa roda
no core 0 gerando pulsos de step; quando o botão Enable é pressionado a tarefa é retomada, quando
solto é suspensa. Na tela são exibidos quatro CircleButtons com os rótulos "M0", "M1", "M2" e "Enable".

O widget CircleButton é um botão circular que muda de aparência ao ser pressionado. Cada um dos
quatro botões (btnm0, btnm1, btnm2, btnenable) foi configurado em loadWidgets com um callback. As
callbacks btnm0_cb, btnm1_cb e btnm2_cb leem getStatus() e definem os pinos M0, M1 e M2;
btnenable_cb define o pino ENABLE e chama xTaskResume ou xTaskSuspend na tarefa do stepper. Toda a
ação é disparada pelo toque e tratada nas callbacks; o loop principal fica vazio. O desenho da tela
é feito em screen0 com fundo branco e drawWidgetsOnScreen(0).

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
void btnm0_cb();
void btnm1_cb();
void btnm2_cb();
void btnenable_cb();

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
CircleButton btnm0(146, 115, 0);
CircleButton btnm1(231, 115, 0);
CircleButton btnm2(316, 115, 0);
CircleButton btnenable(145, 215, 0);
const uint8_t qtdCircleBtn = 4;
CircleButton *arrayCirclebtn[qtdCircleBtn] = {&btnm0, &btnm1, &btnm2, &btnenable};


const int pinM0 = 16;
const int pinM1 = 17;
const int pinM2 = 18;
const int pinEnable = 19;
const int pinStep = 20;
const int pinDir = 21;
const int delayStepUs = 10;

TaskHandle_t taskStepperHandle;

void taskStepper(){
    while(true){
        digitalWrite(pinStep, HIGH);
        delayMicroseconds(10);
        digitalWrite(pinStep, LOW);
        delayMicroseconds(delayStepUs);
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

    pinMode(pinM0, OUTPUT);
    pinMode(pinM1, OUTPUT);
    pinMode(pinM2, OUTPUT);
    pinMode(pinEnable, OUTPUT);
    pinMode(pinStep, OUTPUT);
    pinMode(pinDir, OUTPUT);

    digitalWrite(pinEnable, HIGH);
    digitalWrite(pinDir, LOW);
    digitalWrite(pinM0, LOW);
    digitalWrite(pinM1, LOW);
    digitalWrite(pinM2, LOW);

    xTaskCreatePinnedToCore(taskStepper, "taskStepper", 10000, NULL, 1, &taskStepperHandle, 0);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an circleButton
    //This screen has a/an circleButton
    //This screen has a/an circleButton
    myDisplay.printText("M0", 131, 52, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("M1", 221, 52, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("M2", 306, 52, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an circleButton
    myDisplay.printText("Enable", 115, 262, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    CircleButtonConfig configCircleButton0 = {
            .callback = btnm0_cb,
            .radius = 31,
            .pressedColor = CFK_COLOR08
        };

    btnm0.setup(configCircleButton0);
    CircleButtonConfig configCircleButton1 = {
            .callback = btnm1_cb,
            .radius = 31,
            .pressedColor = CFK_COLOR08
        };

    btnm1.setup(configCircleButton1);
    CircleButtonConfig configCircleButton2 = {
            .callback = btnm2_cb,
            .radius = 31,
            .pressedColor = CFK_COLOR08
        };

    btnm2.setup(configCircleButton2);
    CircleButtonConfig configCircleButton3 = {
            .callback = btnenable_cb,
            .radius = 31,
            .pressedColor = CFK_COLOR05
        };

    btnenable.setup(configCircleButton3);
    myDisplay.setCircleButton(arrayCirclebtn,qtdCircleBtn);


}

// This function is a callback of this element btnm0.
// You dont need call it. Make sure it is as short and quick as possible.
void btnm0_cb(){

    bool myValue = btnm0.getStatus();
    Serial.print("New value for circlebutton is: ");Serial.println(myValue);
    digitalWrite(pinM0, myValue ? HIGH : LOW);
}
// This function is a callback of this element btnm1.
// You dont need call it. Make sure it is as short and quick as possible.
void btnm1_cb(){

    bool myValue = btnm1.getStatus();
    Serial.print("New value for circlebutton is: ");Serial.println(myValue);
    digitalWrite(pinM1, myValue ? HIGH : LOW);
}
// This function is a callback of this element btnm2.
// You dont need call it. Make sure it is as short and quick as possible.
void btnm2_cb(){

    bool myValue = btnm2.getStatus();
    Serial.print("New value for circlebutton is: ");Serial.println(myValue);
    digitalWrite(pinM2, myValue ? HIGH : LOW);
}
// This function is a callback of this element btnenable.
// You dont need call it. Make sure it is as short and quick as possible.
void btnenable_cb(){

    bool myValue = btnenable.getStatus();
    Serial.print("New value for circlebutton is: ");Serial.println(myValue);
    digitalWrite(pinEnable, myValue ? HIGH : LOW);
    if(myValue){
        xTaskResume(taskStepperHandle);
    }else{
        xTaskSuspend(taskStepperHandle);
    }
}
