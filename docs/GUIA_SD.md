# Cartão SD e a imagem icone.fki

O arquivo `/icone.fki` fica na raiz do cartão. O widget `Image` lê esse arquivo depois de `startSD()`. A tela, o toque e o cartão só dividem um fio quando o exemplo diz isso.

No `user_setup.h` fica um display e um toque. Dois toques ao mesmo tempo não compilam.

A carga da imagem acontece no `setup()`, antes de `createTask()`. O widget guarda os pixels. O desenho não reabre o cartão.

```cpp
Image icone(8, 8, 0);                 // Canto da imagem na tela. Troque se quiser outro lugar.
Image *listaIcone[] = {&icone};

bool carregarIcone()
{
    ImageFromFileConfig config;
    config.path = "/icone.fki";       // Caminho na raiz do cartão.
    config.cb = nullptr;              // Sem ação de toque nesta imagem.
    config.source = SourceFile::SD;   // Lê o cartão já montado.
    config.backgroundColor = CFK_WHITE;
    if (!icone.setupFromFileSafe(config))
    {
        return false;
    }
    myDisplay.setImage(listaIcone, 1); // Entrega o widget à biblioteca.
    icone.forceUpdate();
    icone.draw();                      // Mostra os 32x32 agora.
    return true;
}
```

## DSI + GT911 + SD

`user_setup.h`: `DISP_DEFAULT` e `TOUCH_GT911`.

A tela MIPI DSI e o GT911 usam outros barramentos. O cartão fica num SPI só dele. Os tempos abaixo são os da Guition 7 pol. JD9165. SDA 7, SCL 8 e reset 22 são os dessa placa. Os quatro pinos do cartão precisam ser GPIO livres dessa mesma placa.

```cpp
const int TOUCH_SDA = 7;
const int TOUCH_SCL = 8;
const int TOUCH_INT = -1;
const int TOUCH_RST = 22;
const int DISP_RST = 27;
const int SD_CS = 3;                  // Substitua os quatro GPIO. A Guition não publica pino de SD.
const int SD_SCK = 4;
const int SD_MISO = 5;
const int SD_MOSI = 6;
SPIClass spi_sd(FSPI);

void setup()
{
    bus = new Arduino_ESP32DSIPanel(40, 160, 160, 10, 23, 12, 48000000);
    tft = new Arduino_DSI_Display(1024, 600, bus, 0, true, DISP_RST, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchGT911(1024, 600, 0, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);

    SDConfig config;
    config.cs = SD_CS;
    config.sck = SD_SCK;
    config.miso = SD_MISO;
    config.mosi = SD_MOSI;
    config.spi = &spi_sd;             // SPI próprio. Não é o barramento da tela.
    config.initializeSPI = true;      // A biblioteca inicia este SPI.
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```

## RGB + XPT2046 + SD no mesmo SPI

`user_setup.h`: `DISP_DEFAULT` e `TOUCH_XPT2046`.

A tela RGB 4827S043 não usa SPI. O toque e o cartão usam o mesmo SPI. O CS do toque é 38. O CS do cartão é 10. SCK 12, MOSI 11 e MISO 13 são os pinos desse toque.

```cpp
const int TOUCH_CS = 38;
const int TOUCH_SCK = 12;
const int TOUCH_MOSI = 11;
const int TOUCH_MISO = 13;
const int SD_CS = 10;
SPIClass spi_shared(FSPI);

void deixarCsInativo()
{
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH);     // Toque fora do barramento.
    digitalWrite(SD_CS, HIGH);        // Cartão fora do barramento.
}

void setup()
{
    deixarCsInativo();
    spi_shared.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI);
    bus = new Arduino_ESP32RGBPanel(
        40, 41, 39, 42,
        45, 48, 47, 21, 14,
        5, 6, 7, 15, 16, 4,
        8, 3, 46, 9, 1,
        0, 1, 1, 43,
        0, 3, 1, 12,
        1, 10000000, false,
        0, 0, 0);
    tft = new Arduino_RGB_Display(480, 272, bus, 0, true);
    tft->begin();
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchXPT2046(480, 272, 0, TOUCH_CS, &spi_shared, tft, 2500000);
    deixarCsInativo();

    SDConfig config;
    config.cs = SD_CS;
    config.spi = &spi_shared;         // O mesmo SPI do XPT2046.
    config.initializeSPI = false;     // O begin acima já mapeou os pinos.
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```

## ILI9488 + XPT2046 + SD no mesmo SPI

`user_setup.h`: `DISP_DEFAULT` e `TOUCH_XPT2046`.

Tela, toque e cartão usam SCK 18, MISO 19 e MOSI 23. Cada um tem o próprio CS: tela 5, toque 21, cartão 22. DC 2 e reset 4 são da tela. Esses números vêm do exemplo ILI9488 com XPT2046.

```cpp
const int DISP_SCK = 18;
const int DISP_MISO = 19;
const int DISP_MOSI = 23;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const int TOUCH_CS = 21;
const int SD_CS = 22;
SPIClass spi_shared(HSPI);

void deixarCsInativo()
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
    deixarCsInativo();
    spi_shared.begin(DISP_SCK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, 0, false);
    tft->begin(27000000);
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchXPT2046(320, 480, 0, TOUCH_CS, &spi_shared, tft, 2500000, 27000000, DISP_CS);
    deixarCsInativo();

    SDConfig config;
    config.cs = SD_CS;
    config.spi = &spi_shared;
    config.initializeSPI = false;
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```

## ILI9488 + XPT2046 no mesmo SPI e SD em outro SPI

`user_setup.h`: `DISP_DEFAULT` e `TOUCH_XPT2046`.

A tela e o toque dividem SCK 18, MISO 19 e MOSI 23. O CS da tela é 5. O CS do toque é 21. DC 2 e reset 4 são da tela. Esses números vêm do exemplo ILI9488 com XPT2046.

O cartão fica noutro controlador SPI. O CS do cartão é 22, o mesmo da seção anterior. SCK 14, MISO 27 e MOSI 13 são um ponto de partida. Os quatro GPIO não repetem os pinos da tela nem o CS do toque. Confira os GPIO da sua placa antes de gravar.

No ESP32 clássico a tela usa HSPI e o cartão usa VSPI. No ESP32-S3 e no ESP32-P4 a tela usa FSPI e o cartão usa HSPI. O bloco abaixo escolhe esse par. O objeto do cartão ainda não passou por `begin()`. A biblioteca mapeia os quatro pinos.

A chamada do toque recebe o objeto SPI da tela.

```cpp
const int DISP_SCK = 18;
const int DISP_MISO = 19;
const int DISP_MOSI = 23;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const int TOUCH_CS = 21;
const int SD_CS = 22;
const int SD_SCK = 14;                // Substitua SCK, MISO e MOSI se a placa usar outros GPIO.
const int SD_MISO = 27;
const int SD_MOSI = 13;
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32P4)
SPIClass spi_tela(FSPI);              // Tela e toque neste controlador.
SPIClass spi_sd(HSPI);                // Cartão no outro controlador.
#else
SPIClass spi_tela(HSPI);              // ESP32 clássico. Tela e toque neste controlador.
SPIClass spi_sd(VSPI);                // ESP32 clássico. Cartão no outro controlador.
#endif

void deixarCsInativo()
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
    deixarCsInativo();
    spi_tela.begin(DISP_SCK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCK, DISP_MOSI, DISP_MISO, &spi_tela);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, 0, false);
    tft->begin(27000000);
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchXPT2046(320, 480, 0, TOUCH_CS, &spi_tela, tft, 2500000, 27000000, DISP_CS);
    deixarCsInativo();

    SDConfig config;
    config.cs = SD_CS;
    config.sck = SD_SCK;
    config.miso = SD_MISO;
    config.mosi = SD_MOSI;
    config.spi = &spi_sd;             // SPI próprio. Não é o barramento da tela.
    config.initializeSPI = true;      // A biblioteca inicia este SPI.
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```

## Tela I2C + SD em outros pinos

`user_setup.h`: `DISP_SSD1306`. Sem linha de toque.

O OLED SSD1306 usa I2C nos pinos 14 e 13, endereço `0x3C`. O cartão usa outro SPI. SCK 18, MISO 19, MOSI 23 e CS 5 são os pinos SPI do exemplo ILI9488 e não batem com o I2C desse OLED.

O arquivo `/icone.fki` precisa estar no formato da tela escolhida no `user_setup.h`. No SSD1306 o widget espera o formato monocromático, não o RGB565 da tela colorida.

```cpp
const int DISP_SDA = 14;
const int DISP_SCL = 13;
const int SD_CS = 5;
const int SD_SCK = 18;
const int SD_MISO = 19;
const int SD_MOSI = 23;
SPIClass spi_sd(FSPI);

void setup()
{
    Wire.begin(DISP_SDA, DISP_SCL);
    tft = new Adafruit_SSD1306(128, 32, &Wire, -1);
    tft->begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
    myDisplay.setDrawObject(tft);

    SDConfig config;
    config.cs = SD_CS;
    config.sck = SD_SCK;
    config.miso = SD_MISO;
    config.mosi = SD_MOSI;
    config.spi = &spi_sd;             // SPI separado do I2C da tela.
    config.initializeSPI = true;
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```

## ILI9488 + GT911 + SD no mesmo SPI

`user_setup.h`: `DISP_DEFAULT` e `TOUCH_GT911`.

A tela e o cartão dividem o SPI. O GT911 fica na I2C e não entra nesse SPI. SCK 18, MISO 19, MOSI 23, CS da tela 5, DC 2 e reset 4 vêm do exemplo ILI9488. SDA 7, SCL 8 e reset 22 vêm do GT911 da Guition. O CS do cartão é 10, o mesmo usado na 4827, e não repete nenhum desses pinos. Confira os GPIO da sua placa antes de gravar.

```cpp
const int DISP_SCK = 18;
const int DISP_MISO = 19;
const int DISP_MOSI = 23;
const int DISP_CS = 5;
const int DISP_DC = 2;
const int DISP_RST = 4;
const int TOUCH_SDA = 7;
const int TOUCH_SCL = 8;
const int TOUCH_INT = -1;
const int TOUCH_RST = 22;
const int SD_CS = 10;
SPIClass spi_shared(HSPI);

void deixarCsInativo()
{
    pinMode(DISP_CS, OUTPUT);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(DISP_CS, HIGH);
    digitalWrite(SD_CS, HIGH);
}

void setup()
{
    deixarCsInativo();
    spi_shared.begin(DISP_SCK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, 0, false);
    tft->begin(27000000);
    myDisplay.setDrawObject(tft);
    myDisplay.startTouchGT911(320, 480, 0, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    deixarCsInativo();

    SDConfig config;
    config.cs = SD_CS;
    config.spi = &spi_shared;         // Mesmo SPI da tela. O GT911 não usa este SPI.
    config.initializeSPI = false;
    config.frequencyHz = 4000000;
    if (myDisplay.startSD(config))
    {
        carregarIcone();
    }
    myDisplay.createTask(false, 3);
}
```
