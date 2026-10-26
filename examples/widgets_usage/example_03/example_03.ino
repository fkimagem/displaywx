/*

--- English ---

This project is an example of using two RectButton widgets to send commands over UART Serial2. Each
RectButton is a rectangular button that changes appearance when pressed; when the user touches and
the button state changes (pressed or released), the DisplayWX library calls the callback associated
with that widget. The program uses these callbacks to send formatted strings over Serial2, so an
external device (e.g. a module or another board) can be controlled according to each button state on
screen.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. Serial2 is
configured on pins 16 (RX) and 17 (TX) at 115200 baud. On screen, two RectButtons are shown with
the labels "Button 01" and "Button 02" beside them, each with a distinct color when pressed.

The RectButton widget is a rectangular button with rounded corners that toggles between released
and pressed on each touch. Each of the two RectButtons (rectbutton1 and rectbutton1copy) is
configured in loadWidgets with a pressed color and its own callback. When the user touches a button
and the state changes, only that button's callback runs: it reads the new state with getStatus(),
prints to Serial for debug and sends over Serial2 a string in the form ">commandA:0" or ">commandA:1"
(for the first button) and ">commandB:0" or ">commandB:1" (for the second), so a device connected to
the UART can interpret the command. All action is triggered by touch on the buttons and handled in
the callbacks.

The main loop is empty. The screen is drawn in screen0: white background, the two text labels and
drawWidgetsOnScreen(0) to draw the two RectButtons on screen 0.

--- Português ---

Este projeto é um exemplo de uso de dois widgets RectButton para envio de comandos pela UART
Serial2. Cada RectButton é um botão retangular que muda de aparência ao ser pressionado; quando o
usuário toca e o estado do botão muda (pressionado ou solto), a biblioteca DisplayWX chama a
função de callback associada a esse widget. O programa usa essas callbacks para enviar strings
formatadas pela Serial2, permitindo comandar um dispositivo externo (por exemplo um módulo ou outra
placa) conforme o estado de cada botão na tela.

O projeto utiliza um display ST7796 colorido, um touch capacitivo FT6336U e um ESP32S3. A Serial2 é
configurada nos pinos 16 (RX) e 17 (TX) a 115200 baud. Na tela são exibidos dois RectButtons com os
rótulos "Button 01" e "Button 02" ao lado, cada um com cor distinta quando pressionado.

O widget RectButton é um botão retangular com bordas arredondadas que alterna entre estado solto e
pressionado a cada toque. Cada um dos dois RectButtons (rectbutton1 e rectbutton1copy) foi
configurado em loadWidgets com uma cor quando pressionado e com um callback próprio. Quando o
usuário toca em um botão e o estado muda, apenas o callback daquele botão é executado: ele lê o
novo estado com getStatus(), envia o valor ao Serial para debug e envia pela Serial2 uma string no
formato ">commandA:0" ou ">commandA:1" (para o primeiro botão) e ">commandB:0" ou ">commandB:1" (para
o segundo), permitindo que um dispositivo conectado à UART interprete o comando. Toda a ação é
disparada pelo toque nos botões e tratada nas callbacks.

O loop do programa fica vazio. A tela é desenhada em screen0: fundo branco, os dois rótulos de texto
e a chamada drawWidgetsOnScreen(0) para desenhar os dois RectButtons na tela 0.

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
void rectbutton1_cb();
void rectbutton1copy_cb();

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
RectButton rectbutton1(90, 95, 0);
RectButton rectbutton1copy(90, 155, 0);
const uint8_t qtdRectBtn = 2;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton1, &rectbutton1copy};

const int rxSerial2 = 16;
const int txSerial2 = 17;
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

    Serial2.begin(baudRate, SERIAL_8N1, rxSerial2, txSerial2);
}

void loop(){

}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Button 01", 160, 107, TL_DATUM, CFK_COLOR20, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Button 02", 160, 167, TL_DATUM, CFK_COLOR23, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    RectButtonConfig configRectButton0 = {
            .callback = rectbutton1_cb,
            .width = 54,
            .height = 40,
            .pressedColor = CFK_COLOR20
        };

rectbutton1.setup(configRectButton0);
    RectButtonConfig configRectButton1 = {
            .callback = rectbutton1copy_cb,
            .width = 54,
            .height = 40,
            .pressedColor = CFK_COLOR23
        };

rectbutton1copy.setup(configRectButton1);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);


}

// This function is a callback of this element rectbutton1.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1_cb(){

    bool myValue = rectbutton1.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
    Serial2.printf(">commandA:%d\n", myValue);
}
// This function is a callback of this element rectbutton1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1copy_cb(){

    bool myValue = rectbutton1copy.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
    Serial2.printf(">commandB:%d\n", myValue);
}
