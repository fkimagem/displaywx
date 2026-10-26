/*

--- English ---

This project is an example of using three CheckBox widgets to control three relays. Each CheckBox
allows the user to check or uncheck an option on screen; when the user touches and changes the
checkbox state, the DisplayWX library calls the callback associated with that widget. The program
uses these callbacks to turn the corresponding relay on or off, keeping the relay state in sync with
the checked or unchecked state of the checkbox on screen.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Three digital
outputs (pins 16, 17 and 18) are configured to drive relays; each pin controls one relay
independently. On screen, three CheckBoxes are shown with the labels "Relay 1", "Relay 2" and
"Relay 3" beside them, each with a distinct color when checked for easy identification.

The CheckBox widget is a control that toggles between two states, checked and unchecked, on each
touch. Each of the three CheckBoxes (checkbox1, checkbox2, checkbox3) is configured in loadWidgets
with a distinct checked color and its own callback. When the user touches a checkbox and the state
changes, only that checkbox's callback runs: it reads the new state with getStatus(), prints to
Serial for debug and applies that state to the relay pin with digitalWrite (HIGH when checked, LOW
when unchecked). Relays are driven only by the callbacks; no logic is needed in the loop.

The main loop is empty because all action is triggered by touch on the CheckBoxes and handled in the
callbacks. The screen is drawn in screen0: white background, the three text labels and
drawWidgetsOnScreen(0) to draw the three CheckBoxes on screen 0.

--- Português ---

Este projeto é um exemplo de uso de três widgets CheckBox para comando de três relés. Cada CheckBox
permite marcar ou desmarcar uma opção na tela; quando o usuário toca e altera o estado do checkbox,
a biblioteca DisplayWX chama a função de callback associada a esse widget. O programa usa essas
callbacks para ligar ou desligar o relé correspondente, mantendo o estado do relé sincronizado com
o estado marcado ou desmarcado do checkbox na tela.

O projeto utiliza um display ST7796 colorido, um touch capacitivo FT6336U e um ESP32S3. Três saídas
digitais (pinos 16, 17 e 18) são configuradas para acionar relés; cada pino controla um relé de forma
independente. Na tela são exibidos três CheckBoxes com os rótulos "Relay 1", "Relay 2" e "Relay 3"
ao lado, cada um com cor distinta quando marcado para facilitar a identificação.

O widget CheckBox é um controle que alterna entre dois estados, marcado e desmarcado, a cada toque.
Cada um dos três CheckBoxes (checkbox1, checkbox2 e checkbox3) foi configurado em loadWidgets com
uma cor de seleção distinta e com um callback próprio. Quando o usuário toca em um checkbox e o
estado muda, apenas o callback daquele checkbox é executado: ele lê o novo estado com getStatus(),
envia o valor ao Serial para debug e aplica esse estado ao pino do relé correspondente com
digitalWrite (HIGH quando marcado, LOW quando desmarcado). Os relés são comandados exclusivamente
pelas callbacks, sem necessidade de lógica no loop.

O loop do programa fica vazio porque toda a ação é disparada pelo toque nos CheckBoxes e tratada
nas callbacks. A tela é desenhada em screen0: fundo branco, os três rótulos de texto e a chamada
drawWidgetsOnScreen(0) para desenhar os três CheckBoxes na tela 0.

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
void checkbox1_cb();
void checkbox2_cb();
void checkbox3_cb();

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
CheckBox checkbox1(90, 80, 0);
CheckBox checkbox2(90, 130, 0);
CheckBox checkbox3(90, 180, 0);
const uint8_t qtdCheckbox = 3;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox1, &checkbox2, &checkbox3};

const int pinRelay1 = 16;
const int pinRelay2 = 17;
const int pinRelay3 = 18;

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


    pinMode(pinRelay1, OUTPUT);
    pinMode(pinRelay2, OUTPUT);
    pinMode(pinRelay3, OUTPUT);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    myDisplay.printText("Relay 1", 120, 82, TL_DATUM, CFK_COLOR04, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Relay 2", 120, 132, TL_DATUM, CFK_COLOR20, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Relay 3", 120, 182, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    CheckBoxConfig configCheckbox0 = {
            .callback = checkbox1_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR04,
            .uncheckedColor = CFK_GREY10
        };
    checkbox1.setup(configCheckbox0);
    CheckBoxConfig configCheckbox1 = {
            .callback = checkbox2_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR20,
            .uncheckedColor = CFK_GREY10
        };
    checkbox2.setup(configCheckbox1);
    CheckBoxConfig configCheckbox2 = {
            .callback = checkbox3_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR30,
            .uncheckedColor = CFK_GREY10
        };
    checkbox3.setup(configCheckbox2);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);


}

// This function is a callback of this element checkbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox1_cb(){

    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay1, myValue ? HIGH : LOW);
}
// This function is a callback of this element checkbox2.
// You dont need call it. Make sure it is as short and quick as possible.

void checkbox2_cb(){

    bool myValue = checkbox2.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay2, myValue ? HIGH : LOW);
}
// This function is a callback of this element checkbox3.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox3_cb(){

    bool myValue = checkbox3.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
    digitalWrite(pinRelay3, myValue ? HIGH : LOW);
}
