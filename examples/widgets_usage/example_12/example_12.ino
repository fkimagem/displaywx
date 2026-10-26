/*

--- English ---

This project is an example of displaying WiFi signal strength (RSSI) using a GaugeSuper widget and a
Label. The ESP32 connects to a WiFi network in setup(); the loop checks if the connection is active,
reconnects if needed, then reads WiFi.RSSI() and updates both the gauge needle and the label inside
startCustomDraw and finishCustomDraw. The gauge is configured for the range -100 to 0 (typical RSSI
in dBm) with four coloured intervals; the Label shows the same value with prefix "RSSI: " and suffix
" dBm", so the user sees both a needle and a text like "RSSI: -45 dBm".

The GaugeSuper widget is a semicircular gauge that displays a numeric value with a needle and
optional coloured intervals (ranges). It has no touch callback; the program updates the value with
setValue() so the needle and colours reflect the current reading. The Label widget displays text or
numeric values on screen; it can show a value with optional prefix and suffix (e.g. "RSSI: -45 dBm").
When setTextInt() or setTextFloat() is called, the displayed string becomes prefix + value + suffix,
making units and captions clear without extra code.

The project uses an ST7796 display, an FT6336U capacitive touch and an ESP32S3. There are no touch
callbacks. Configure SSID and PASSWORD before uploading. Screen drawing is done in screen0 with the
title "WiFi force" and drawWidgetsOnScreen(0).

--- Português ---

Este projeto é um exemplo de exibição da força do sinal WiFi (RSSI) usando um widget GaugeSuper e um
Label. O ESP32 conecta à rede WiFi no setup(); o loop verifica se a conexão está ativa, reconecta
se necessário, lê WiFi.RSSI() e atualiza tanto a agulha do gauge quanto o label entre
startCustomDraw e finishCustomDraw. O gauge está configurado para a faixa -100 a 0 (RSSI típico em
dBm) com quatro intervalos coloridos; o Label exibe o mesmo valor com prefix "RSSI: " e suffix "
dBm", de modo que o usuário vê a agulha e um texto como "RSSI: -45 dBm".

O widget GaugeSuper é um gauge semicircular que exibe um valor numérico com uma agulha e intervalos
coloridos opcionais (faixas). Não possui callback de toque; o programa atualiza o valor com
setValue() para que a agulha e as cores reflitam a leitura atual. O widget Label exibe texto ou
valores numéricos na tela; pode mostrar um valor com prefix e suffix opcionais (ex.: "RSSI: -45 dBm").
Quando setTextInt() ou setTextFloat() é chamado, a string exibida fica prefix + valor + suffix,
deixando unidades e legendas claras sem código extra.

O projeto utiliza um display ST7796, um touch capacitivo FT6336U e um ESP32S3. Não há callbacks de
toque. Configure SSID e PASSWORD antes de enviar o sketch. O desenho da tela é feito em screen0 com
o título "WiFi force" e drawWidgetsOnScreen(0).

*/

#include <WiFi.h>

const char *SSID = "SEU_SSID";
const char *PASSWORD = "SUA_SENHA";

// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <SPI.h>
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
const uint8_t rotationScreen =
    1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

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
GaugeSuper widget1(245, 165, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&widget1};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {-100, -85, -70, -50};
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR01, CFK_COLOR04, CFK_COLOR06,
                                   CFK_COLOR12};
Label txtforce(190, 195, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&txtforce};

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI inicial (dBm): ");
  Serial.println(WiFi.RSSI());
}

void setup() {

  Serial.begin(115200);
  // Start SPI object for display
  spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
  bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO,
                          &spi_shared);
  tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
  tft->begin(DISP_FREQUENCY);
  myDisplay.setDrawObject(tft); // Reference to object to draw on screen
  // Setup touch
  myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0,
                            TOUCH_MAP_Y1);
  myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
  myDisplay.setSwapAxis(TOUCH_SWAP_XY);
  myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA,
                             TOUCH_SCL, TOUCH_INT, TOUCH_RST);
  // myDisplay.enableTouchLog();
  loadWidgets(); // This function is used to setup with widget individualy
  myDisplay.loadScreen(screen0);  // Use this line to change between screens
  myDisplay.createTask(false, 3); // Initialize the task to read touch and draw

  connectWiFi();
}

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Reconectando...");
    connectWiFi();
  } else {
    int rssi = WiFi.RSSI();

    myDisplay.startCustomDraw();
    widget1.setValue(rssi);    // Use this command to change widget value.
    txtforce.setTextInt(rssi); // Use this command to change widget value.
    myDisplay.finishCustomDraw();
  }
}

void screen0() {

  tft->fillScreen(CFK_WHITE);
  WidgetBase::backgroundColor = CFK_WHITE;
  // This screen has a/an ponteiroAnalog
  // This screen has a/an label
  myDisplay.printText("WiFi force", 195, 17, TL_DATUM, CFK_BLACK, CFK_WHITE,
                      &RobotoRegular10pt7b);
  myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets() {

  GaugeConfig configGauge0 = {.title = "Title",
                              .intervals = range0,
                              .colors = colors0,
                              .fontFamily = &RobotoRegular8pt7b,
                              .minValue = -100,
                              .maxValue = 0,
                              .width = 255,
                              .height = 96,
                              .borderColor = CFK_GREY3,
                              .textColor = CFK_BLACK,
                              .backgroundColor = CFK_WHITE,
                              .titleColor = CFK_BLACK,
                              .needleColor = CFK_RED,
                              .markersColor = CFK_BLACK,
                              .amountIntervals = qtdIntervalG0,
                              .showLabels = true};
  widget1.setup(configGauge0);
  myDisplay.setGauge(arrayGauge, qtdGauge);

  LabelConfig configLabel0 = {.text = "-----",
                              .prefix = "RSSI: ",
                              .suffix = " dBm",
                              .fontFamily = &RobotoRegular10pt7b,
                              .datum = TL_DATUM,
                              .fontColor = CFK_BLACK,
                              .backgroundColor = CFK_WHITE};
  txtforce.setup(configLabel0);
  myDisplay.setLabel(arrayLabel, qtdLabel);
}
