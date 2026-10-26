#define P4_WIFI6
//#define SCAN_REDES 1


typedef struct
{
    int32_t channel;
    int16_t idx;
    int32_t rssi;
    String ssid;
} Rede_t;

#define MAX_REDES 10
#define QUEUE_SIZE 10  // Tamanho da fila de strings
#define LINE_HEIGHT 30

Rede_t redes[MAX_REDES];
int lastCountRedes = 0;

// Fila de strings para armazenar dados recebidos via Serial1
String messageQueue[QUEUE_SIZE];
int queueCount = 0;  // Número atual de itens na fila
bool queueUpdated = false;  // Flag para indicar que a fila foi atualizada

// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include "WiFi.h"
// #define DFK_SD
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
const int DISP_MOSI = 23;
const int DISP_MISO = -1;
const int DISP_SCLK = 26;
const int DISP_CS = 20;
const int DISP_DC = 22;
const int DISP_RST = 21;
#if defined(P4_WIFI6)
const int TOUCH_SCL = 31;
const int TOUCH_SDA = 30;
#else
const int TOUCH_SCL = 18;
const int TOUCH_SDA = 17;
#endif

const int TOUCH_INT = 47;
const int TOUCH_RST = 33;

#define RX1_PIN 49
#define TX1_PIN 50

// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, true);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();

    // Init Display
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    // loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0);  // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}
void readSerial(){
    static String receivedMessage = "";  // Buffer para acumular caracteres
    
    while (Serial1.available())
    {
        int byteRecebido = Serial1.read();
        Serial.write(byteRecebido);
        
        // Se não for quebra de linha, adiciona ao buffer
        if (byteRecebido != '\n') {
            receivedMessage += (char)byteRecebido;
        } else {
            // Encontrou quebra de linha, processa a mensagem
            receivedMessage.trim();  // Remove espaços em branco no início e fim
            
            // Se a mensagem tem caracteres impressos, adiciona à fila
            if (receivedMessage.length() > 0) {
                // Se a fila está cheia, remove o primeiro item e realoca
                if (queueCount >= QUEUE_SIZE) {
                    // Remove o primeiro item e move todos para cima
                    for (int i = 0; i < QUEUE_SIZE - 1; i++) {
                        messageQueue[i] = messageQueue[i + 1];
                    }
                    queueCount = QUEUE_SIZE - 1;
                }
                
                // Adiciona o novo item no final
                messageQueue[queueCount] = receivedMessage;
                queueCount++;
                queueUpdated = true;  // Marca que a fila foi atualizada
            }
            
            receivedMessage = "";  // Limpa o buffer para a próxima mensagem
        }
    }

    // Se houver dados digitados no monitor serial, envia para a Serial1
    while (Serial.available())
    {
        int byteDigitado = Serial.read();
        Serial1.write(byteDigitado);
    }
}
void printMessages(){
// Verifica se a fila foi atualizada
    if (queueUpdated) {
        myDisplay.startTransaction();
        int fixX = 5;
        int startY = 5;
        tft->fillScreen(CFK_WHITE);
        Serial.println("=== FILA DE MENSAGENS ===");
        for (int i = 0; i < queueCount; i++) {
            Serial.print("[" + String(i) + "] ");
            Serial.println(messageQueue[i]);
            //myDisplay.printText("Custom text", 5, 7, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
            // Prin line start x = 5, y = 5, increment  y by variable LINE_HEIGHT
            myDisplay.printText(messageQueue[i].c_str(), fixX, startY, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
            startY += LINE_HEIGHT;

        }
        myDisplay.finishTransaction();
        Serial.println("========================");
        queueUpdated = false;  // Reseta a flag
    }
}
void loop()
{
    readSerial();
    printMessages();
    
    
    delay(2);
}

void scan()
{
    #ifdef SCAN_REDES
#if defined(ESP32)
    int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */, true /* passive */, 500 /* max_ms_per_chan */);
#else
    int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */);
#endif

    int minRSSI = -80;
    lastCountRedes = 0;

    for (int i = 0; i < n && i < MAX_REDES; i++)
    {

        redes[lastCountRedes].channel = WiFi.channel(i);
        redes[lastCountRedes].idx = redes[lastCountRedes].channel - 1;
        redes[lastCountRedes].rssi = WiFi.RSSI(i);
        redes[lastCountRedes].ssid = WiFi.SSID(i);

        if (redes[lastCountRedes].rssi < minRSSI)
            continue;

        Serial.printf("%i - Channel: %02i\t", i, redes[lastCountRedes].channel);
        Serial.printf("Idx: %i\t", redes[lastCountRedes].idx);
        Serial.printf("RSSI: %i\t", redes[lastCountRedes].rssi);
        Serial.printf("SSID: %s\n", redes[lastCountRedes].ssid.c_str());

        lastCountRedes++;
    }
    Serial.println("------------------");
    myDisplay.refreshScreen();
    #endif
}

void screen0()
{
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    myDisplay.printText("Aguardando comandos", 5, 5, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    /*tft->fillCircle(25, 25, 19, CFK_COLOR01);
    tft->drawCircle(25, 25, 19, CFK_COLOR01);
    tft->fillCircle(40, 35, 19, CFK_COLOR11);
    tft->drawCircle(40, 35, 19, CFK_COLOR01);
    tft->fillCircle(55, 45, 19, CFK_COLOR22);
    tft->drawCircle(55, 45, 19, CFK_COLOR01);*/

    

    #ifdef SCAN_REDES

    int raioMin = 10;
    int raioMax = 150;
    int quantidade = 5;
    int passo = (raioMax - raioMin) / quantidade;
    for (auto i = 0; i <= quantidade; i++)
    {
        int raio = raioMin + passo * i;
        tft->drawCircle(240, 160, raio, CFK_COLOR01);
    }
    
    for (auto i = 0; i < lastCountRedes; i++)
    {
        Rede_t item = redes[i];
        int rndAngle = random(0, 360);
        int prop = map(item.rssi, -10, -80, raioMin, raioMax);
        float anguloRad = rndAngle * PI / 180.0;

        int calcX = cos(anguloRad) * prop;
        int calcY = sin(anguloRad) * prop;

        int posX = 240 + calcX;
        int posY = 160 + calcY;

        tft->fillCircle(posX, posY, 2, CFK_COLOR22);
        myDisplay.printText(item.ssid.c_str(), posX, posY, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular5pt7b);
    }
    #endif

    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets()
{
}