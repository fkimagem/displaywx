#include <SPI.h>
#include <displaywx.h>

// Configuracao concreta ja usada por examples/basic_test/spi/esp32/Arduino1x__ESP32__ILI9488_XPT2046.
// O CS do cartao desta ligacao e diferente do CS da tela e do touch. Nao e um pino universal da biblioteca.
#if defined(DISP_DEFAULT) && defined(TOUCH_XPT2046)
#include <Arduino_GFX_Library.h>

const int DISPLAY_W = 240;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_FREQUENCY = 2500000;
const int DISP_MOSI = 23;
const int DISP_MISO = 19;
const int DISP_SCLK = 18;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const int TOUCH_CS = 21;
const int SD_CS = 22;
const uint8_t rotationScreen = 0;
const bool isIPS = false;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif

Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;

void deixarSelecoesInativas()
{
    pinMode(DISP_CS, OUTPUT);
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(DISP_CS, HIGH);
    digitalWrite(TOUCH_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
}

void setup()
{
    Serial.begin(115200);
    deixarSelecoesInativas();
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);

    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, isIPS);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_CS, &spi_shared, tft, TOUCH_FREQUENCY, DISP_FREQUENCY, DISP_CS);

    deixarSelecoesInativas();
    SDConfig config;
    config.cs = SD_CS;
    config.spi = &spi_shared;
    config.initializeSPI = false;
    config.frequencyHz = 4000000;
    if (!myDisplay.startSD(config))
    {
        Serial.println("SD indisponivel; tela e touch continuam ativos.");
    }
    myDisplay.createTask(false, 3);
}

#else

DisplayWX myDisplay;

void setup()
{
    Serial.begin(115200);
    Serial.println("Este exemplo compartilha o SPI do ILI9488 com XPT2046. A user_setup atual usa outro display ou toque.");
}

#endif

void loop() {}
