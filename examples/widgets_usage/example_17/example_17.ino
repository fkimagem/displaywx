/*

--- English ---

This project is a WiFi connection screen: two TextBox widgets for SSID and password, a "Connect" TextButton,
and a Label that shows "Connected" or "Disconnected". The TextBox callbacks (tbssid_cb, tbpass_cb) store
the current text with getValue() into tb0_val and tb1_val. When the user taps "Connect", the callback
(textbutton1_cb) should set connectWiFi to true so that the loop runs WiFi.begin(tb0_val, tb1_val) and
waits up to timeOutConnectWiFi for the connection. The loop then checks WiFi.status(), sets
isWiFiConnected, and updates the Label (text3) with setText("Connected") or setText("Disconnected") inside
startCustomDraw and finishCustomDraw. The screen title is "Connect WiFi"; labels "SSID" and "Pass" appear
beside the text boxes. For the connection to start, the Connect button callback must set connectWiFi = true
(if it only prints to Serial, the connection will never be triggered).

The TextBox widget is a text input field; when the user touches it, the library opens an on-screen
keyboard (WKeyboard). After the user finishes editing, the callback is called and the program can read
the current text with getValue(). Here the two TextBoxes store SSID and password in tb0_val and tb1_val.
The TextButton widget is a button with a text label; when the user taps it, its callback runs (here it
should set connectWiFi = true to trigger WiFi.begin in the loop). The Label widget displays text; the
program updates it in the loop with setText() to show "Connected" or "Disconnected" according to
WiFi.status().

Display: ST7796, FT6336U, ESP32S3. Uses the WiFi library; loop ends with delay(2000).

--- Português ---

Este projeto é uma tela de conexão WiFi: dois widgets TextBox para SSID e senha, um TextButton "Connect"
e um Label que exibe "Connected" ou "Disconnected". Os callbacks dos TextBox (tbssid_cb, tbpass_cb)
armazenam o texto atual com getValue() em tb0_val e tb1_val. Quando o usuário toca em "Connect", o
callback (textbutton1_cb) deve definir connectWiFi como true para que o loop execute WiFi.begin(tb0_val,
tb1_val) e espere até timeOutConnectWiFi pela conexão. O loop verifica WiFi.status(), define
isWiFiConnected e atualiza o Label (text3) com setText("Connected") ou setText("Disconnected") entre
startCustomDraw e finishCustomDraw. O título da tela é "Connect WiFi"; as etiquetas "SSID" e "Pass"
aparecem ao lado das caixas de texto. Para a conexão iniciar, o callback do botão Connect deve definir
connectWiFi = true (se apenas imprimir na Serial, a conexão nunca será disparada).

O widget TextBox é um campo de entrada de texto; quando o usuário toca nele, a biblioteca abre um
teclado na tela (WKeyboard). Após o usuário terminar de editar, o callback é chamado e o programa
pode ler o texto atual com getValue(). Aqui os dois TextBoxes armazenam SSID e senha em tb0_val e
tb1_val. O widget TextButton é um botão com rótulo de texto; quando o usuário toca, seu callback é
executado (aqui deve definir connectWiFi = true para disparar WiFi.begin no loop). O widget Label
exibe texto; o programa o atualiza no loop com setText() para mostrar "Connected" ou "Disconnected"
conforme WiFi.status().

Display: ST7796, FT6336U, ESP32S3. Usa a biblioteca WiFi; o loop termina com delay(2000).

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
void tbssid_cb();
void tbpass_cb();
void textbutton1_cb();

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
Label text3(190, 255, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&text3};
TextBox tbssid(175, 90, 0);
TextBox tbpass(175, 140, 0);
const uint8_t qtdTextbox = 2;
TextBox *arrayTextbox[qtdTextbox] = {&tbssid, &tbpass};
const char* tb0_val = ""; // Global variable that stores the value of the widget tbssid
const char* tb1_val = ""; // Global variable that stores the value of the widget tbpass
TextButton textbutton1(175, 190, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};


bool connectWiFi = false;
uint32_t timeOutConnectWiFi = 10000;
bool isWiFiConnected = false;

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
}

void loop(){

    if(connectWiFi){
        connectWiFi = false;
        WiFi.begin(tb0_val, tb1_val);
        uint32_t startTime = millis();
        while(WiFi.status() != WL_CONNECTED && millis() - startTime < timeOutConnectWiFi){
            delay(1000);
        }
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println("WiFi connected");
        isWiFiConnected = true;
    }else{
        Serial.println("WiFi connection failed");
        isWiFiConnected = false;
    }

    myDisplay.startCustomDraw();
    text3.setText(isWiFiConnected ? "Connected" : "Disconnected");
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("SSID", 120, 92, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an caixaTexto
    //This screen has a/an caixaTexto
    myDisplay.printText("Pass", 120, 142, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an textButton
    //This screen has a/an label
    myDisplay.printText("Connect WiFi", 175, 22, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "---",
            .prefix = "Status: ",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
    text3.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    WKeyboard::m_backgroundColor = CFK_GREY3;
    WKeyboard::m_letterColor = CFK_BLACK;
    WKeyboard::m_keyColor = CFK_GREY13;


    TextBoxConfig configTextBox0 = {
            .startValue = "SSDI",
            .funcPtr = screen0,
            .cb = tbssid_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
    tbssid.setup(configTextBox0);
    TextBoxConfig configTextBox1 = {
            .startValue = "PASS",
            .funcPtr = screen0,
            .cb = tbpass_cb,
            .font = &RobotoRegular10pt7b,
            .width = 141,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE
        };
    tbpass.setup(configTextBox1);
    myDisplay.setTextbox(arrayTextbox,qtdTextbox);


    TextButtonConfig configTextButton0 = {
            .text = "Connect",
            .callback = textbutton1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 110,
            .height = 40,
            .radius = 10,
            .backgroundColor = CFK_COLOR17,
            .textColor = CFK_BLACK
        };

    textbutton1.setup(configTextButton0);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

// This function is a callback of this element tbssid.
// You dont need call it. Make sure it is as short and quick as possible.
void tbssid_cb(){

    tb0_val = tbssid.getValue();
}
// This function is a callback of this element tbpass.
// You dont need call it. Make sure it is as short and quick as possible.
void tbpass_cb(){

    tb1_val = tbpass.getValue();
}
// This function is a callback of this element textbutton1.
// You dont need call it. Make sure it is as short and quick as possible.
void textbutton1_cb(){
    connectWiFi = true;
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
}
