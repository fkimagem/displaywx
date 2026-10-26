/*

O projeto é um exemplo de como usar o widget Checkbox, vertical bar e label.
O widget Checkbox é um widget que permite selecionar uma opção.
O widget vertical bar é um widget que permite visualizar um valor numérico em uma barra vertical.
O widget label é um widget que permite exibir um texto.

O projeto é composto por um display ST7796, um touch FT6336U e um ESP32S3.

O display ST7796 é um display colorido de 320x480 pixels.
O touch FT6336U é um touch capacitivo de 320x480 pixels.

Foi adicionado um led e um potenciômetro para testar o projeto.
O led é ligado quando o checkbox está marcado.
O valor do potenciômetro é mapeado para um valor entre 0 e 100 e exibido na barra vertical.

Quando o checkbox tem o estado alterado, o callback checkbox1_cb é chamado e o led é ligado ou desligado.
O loop é responsavel por ler o valor do potenciômetro e mapear para um valor entre 0 e 100 e exibir na barra vertical.

Usamos a startCustomDraw e finishCustomDraw para agrupar mudanças no widgets, ou seja. Quando startCustomDraw o desenho no display é bloqueado pelo classe DisplayWX, dessa maneira podemos modificar todos os widgets que queremos de uma unica vez e quando chamamos o finishCustomDraw o desenho é liberado e o display é atualizado.


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
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
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
CheckBox checkbox1(115, 90, 0);
const uint8_t qtdCheckbox = 1;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox1};
Label labelvbar12(130, 265, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&labelvbar12};
VBar vertbar1(175, 125, 0);
const uint8_t qtdVBar = 1;
VBar *arrayVbar[qtdVBar] = {&vertbar1};

const int pinLed = 16;
const int pinPotenciometer = 17;

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

    pinMode(pinLed, OUTPUT);
    pinMode(pinPotenciometer, INPUT);
}

void loop(){

    myDisplay.startCustomDraw();
    int value = analogRead(pinPotenciometer);
    int mappedValue = map(value, 0, 4095, 0, 100);
    vertbar1.setValue(mappedValue); //Use this command to change widget value.
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an checkbox
    myDisplay.printText("Custom text", 145, 92, TL_DATUM, CFK_COLOR30, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an rectRange
    //This screen has a/an label
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    CheckBoxConfig configCheckbox0 = {
            .callback = checkbox1_cb,
            .weight = CheckBoxWeight::MEDIUM,
            .size = 21,
            .checkedColor = CFK_COLOR30,
            .uncheckedColor = CFK_GREY10
        };
    checkbox1.setup(configCheckbox0);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);


    LabelConfig configLabel0 = {
            .text = "label_vbar 1",
            .prefix = "",
            .suffix = "%",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR30,
            .backgroundColor = CFK_WHITE
        };
    labelvbar12.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    VerticalBarConfig configVBar0 = {
            .subtitle = &labelvbar12,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 21,
            .height = 130,
            .filledColor = CFK_COLOR30
        };
    vertbar1.setup(configVBar0);
    myDisplay.setVBar(arrayVbar,qtdVBar);


}

// This function is a callback of this element checkbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox1_cb(){

    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);

    digitalWrite(pinLed, myValue ? HIGH : LOW);
}
