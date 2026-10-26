/*

--- English ---

This project is an example of using two TextButton widgets to send start and stop commands over
UART Serial2. Each TextButton displays a label ("Start" or "Stop") and when the user touches it,
the DisplayWX library calls the associated callback. The program uses these callbacks to send
formatted strings over Serial2 (">command:start" and ">command:stop"), so an external device
connected to the UART can receive the commands. Each callback also prints to Serial for debug.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Serial2 is
configured on pins 16 and 17 at 115200 baud. On screen, two TextButtons are shown with the text
"Start" and "Stop", each with rounded corners and a distinct background color.

The TextButton widget is a rectangular button with customizable text. btnstart is configured with
callback btnstart_cb, which sends ">command:start" via Serial2; btnstop is configured with
btnstop_cb, which sends ">command:stop". All action is triggered by touch and handled in the
callbacks; the main loop is empty. The screen is drawn in screen0 with a white background and
drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de uso de dois widgets TextButton para enviar comandos de início e parada
pela UART Serial2. Cada TextButton exibe um rótulo ("Start" ou "Stop") e quando o usuário toca, a
biblioteca DisplayWX chama o callback associado. O programa usa essas callbacks para enviar
strings formatadas pela Serial2 (">command:start" e ">command:stop"), permitindo que um
dispositivo externo conectado à UART receba os comandos. Cada callback também imprime no Serial
para debug.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. A Serial2 é
configurada nos pinos 16 e 17 a 115200 baud. Na tela são exibidos dois TextButtons com o texto
"Start" e "Stop", cada um com cantos arredondados e cor de fundo definida.

O widget TextButton é um botão retangular com texto personalizável. O btnstart está configurado
com o callback btnstart_cb, que envia ">command:start" pela Serial2; o btnstop está configurado com
btnstop_cb, que envia ">command:stop". Toda a ação é disparada pelo toque e tratada nas callbacks;
o loop principal fica vazio. A tela é desenhada em screen0 com fundo branco e drawWidgetsOnScreen(0).

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
void btnstart_cb();
void btnstop_cb();

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
TextButton btnstart(175, 100, 0);
TextButton btnstop(175, 175, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btnstart, &btnstop};

const int pinTx = 16;
const int pinRx = 17;
const int baudRate = 115200;



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


    Serial2.begin(baudRate, SERIAL_8N1, pinRx, pinTx);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an textButton
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    TextButtonConfig configTextButton0 = {
            .text = "Start",
            .callback = btnstart_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 100,
            .height = 40,
            .radius = 10,
            .backgroundColor = CFK_COLOR30,
            .textColor = CFK_WHITE
        };

    btnstart.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Stop",
            .callback = btnstop_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 100,
            .height = 40,
            .radius = 10,
            .backgroundColor = CFK_COLOR30,
            .textColor = CFK_WHITE
        };

    btnstop.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

// This function is a callback of this element btnstart.
// You dont need call it. Make sure it is as short and quick as possible.
void btnstart_cb(){

    Serial.print("Clicked on: ");Serial.println("btnstart_cb");
    Serial2.printf(">command:start\n");
}
// This function is a callback of this element btnstop.
// You dont need call it. Make sure it is as short and quick as possible.
void btnstop_cb(){

    Serial.print("Clicked on: ");Serial.println("btnstop_cb");
    Serial2.printf(">command:stop\n");
}
