# Linhas novas para o gerador de código

Referência dos sketches (não alterar os exemplos):

- `examples/generic/ESP32P4/camera_4in`
- `examples/generic/ESP32P4/camera_10in`
- `examples/generic/ESP32P4/pico_camera_display`
- `examples/generic/ESP32P4/p4_eye`

Alvo: ESP32-P4, ESP-IDF >= 5.4. A câmera só existe com um `CAM_*` em `user_setup.h`. `src/check_camera.h` define `HAS_CAMERA_MIPI` a partir desse sensor. O gerador não emite `#define HAS_CAMERA_MIPI` nem o nome antigo `USE_CAMERA_CSI`.

Só um `CAM_*` e só um `TOUCH_*` ficam ativos. O `startTouch*` do sketch tem que ser o mesmo do `#define`: as funções são `#elif` em `displaywx.h`. Sem o define certo, a chamada não compila. `HAS_TOUCH` também nasce sozinho em `src/check_touch.h`.

`DFK_CAMERA_MIPI` já está em `src/widgets/widgetsetup.h`. O sketch não define isso.

## Ordem que o gerador tem que respeitar

`setup()` sobe a câmera e só depois chama `startDisplay()`. Se `startCamera` falhar, o sketch dá `return` e não cria o painel.

Dentro de `startDisplay()`, depois de `tft->begin()`:

1. `myDisplay.getCamera().startPpa();`
2. backlight (GPIO ou `waveshareDsiAuxBacklight`)
3. `setDrawObject`
4. cantos, eixos e `startTouch*` (quando a placa tem touch)
5. `loadWidgets()` — aqui entra `setCameraMipi`
6. `loadScreen`
7. `createTask(false, 3);`

`loop()` fica vazio, exceto no P4-EYE (botões de zoom).

## Globais comuns (os quatro modelos)

```cpp
#define ORIENTATION 0          // 0..3, igual Arduino_GFX
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

DisplayWX myDisplay;

CameraMipi cam01(x, y, screen);   // um objeto por widget
const uint8_t qtdCameraMipi = 1;  // tamanho do array
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cam01};
```

No `loadWidgets()` de cada widget:

```cpp
CameraMipiConfig config = { .width = W, .height = H, .showZoomLevel = true };
cam01.setup(config);
cam01.setZoomLevel(1);
myDisplay.setCameraMipi(arrayCameraMipi, qtdCameraMipi);
```

`setZoomLevel` aceita 1, 2 ou 4. `showZoomLevel` só desenha o rótulo 1x/2x/3x no canto do widget.

Includes que os quatro usam: `Arduino.h`, `Arduino_GFX_Library.h`, `displaywx.h`.

## user_setup.h por modelo

Um bloco por placa. Os outros sensores e touchs ficam comentados.

### camera_4in — ST7701 480x800, OV02C10, GT911

```cpp
#define TOUCH_GT911
#define CAM_OV02C10
//#define TOUCH_GSL3680
//#define TOUCH_GT911_WAVESHARE
//#define CAM_OV5647
//#define CAM_SC2336
//#define CAM_OV2710
```

### camera_10in — JD9365 800x1280, OV02C10, GSL3680

```cpp
#define TOUCH_GSL3680
#define CAM_OV02C10
//#define TOUCH_GT911
//#define TOUCH_GT911_WAVESHARE
//#define CAM_OV5647
//#define CAM_SC2336
//#define CAM_OV2710
```

### pico_camera_display — Waveshare 7" ILI9881C 720x1280, OV5647, GT911 Waveshare

`DISP_DEFAULT` é obrigatório: as constantes `ILI9881C_WAVESHARE_7_*` só existem com esse define.

```cpp
#define DISP_DEFAULT
#define TOUCH_GT911_WAVESHARE
#define CAM_OV5647
//#define TOUCH_GT911
//#define TOUCH_GSL3680
//#define CAM_OV02C10
//#define CAM_SC2336
//#define CAM_OV2710
```

### p4_eye — ST7789 240x240 SPI, OV2710, sem touch

Não emitir `TOUCH_*`. Sem touch, `HAS_TOUCH` não existe e `startTouch*` não entra no sketch.

```cpp
#define CAM_OV2710
//#define CAM_OV02C10
//#define CAM_OV5647
//#define CAM_SC2336
//#define TOUCH_GT911
//#define TOUCH_GT911_WAVESHARE
//#define TOUCH_GSL3680
```

## Guardas no sketch

### camera_4in e camera_10in

```cpp
#if !defined(HAS_CAMERA_MIPI) || !defined(CAM_OV02C10)
#error "This example requires HAS_CAMERA_MIPI and CAM_OV02C10 in user_setup.h"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
#if CONFIG_IDF_TARGET_ESP32P4
// defines SCCB + setup() + loop()
#else
#error "The selected target SoC is not supported"
#endif
#else
// setup() só imprime que o IDF < 5.4 não é suportado
#endif
```

### pico_camera_display

Estas guardas ficam no topo, antes dos globais:

```cpp
#if !defined(DISP_DEFAULT) || !defined(TOUCH_GT911_WAVESHARE)
#error "This example requires DISP_DEFAULT and TOUCH_GT911_WAVESHARE in user_setup.h"
#endif

#if !defined(HAS_CAMERA_MIPI) || !defined(CAM_OV5647)
#error "This example requires CAM_OV5647 in user_setup.h"
#endif

#if !defined(CONFIG_IDF_TARGET_ESP32P4)
#error "The selected target SoC is not supported"
#endif
```

O `setup()` da câmera ainda fica dentro de `ESP_IDF_VERSION >= 5.4.0`.

### p4_eye

```cpp
#if !defined(HAS_CAMERA_MIPI) || !defined(CAM_OV2710)
#error "This example requires HAS_CAMERA_MIPI and CAM_OV2710 in user_setup.h"
#endif
```

Mais o mesmo bloco de IDF 5.4 e `CONFIG_IDF_TARGET_ESP32P4` dos painéis de 4" e 10".

## camera_4in

Painel DSI ST7701, nativo 480x800. Orientação de exemplo: 1 (paisagem). Touch GT911 no mesmo I2C da câmera (GPIO 7/8). INT e RST do touch não estão ligados.

Include extra: `#include <SPI.h>` e `#include <ESP_Video.h>`.

```cpp
#define ORIENTATION 1
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

const int PANEL_NATIVE_W = 480;
const int PANEL_NATIVE_H = 800;
#if (ORIENTATION % 2) == 0
const int DISPLAY_W = PANEL_NATIVE_W;
const int DISPLAY_H = PANEL_NATIVE_H;
#else
const int DISPLAY_W = PANEL_NATIVE_H;
const int DISPLAY_H = PANEL_NATIVE_W;
#endif
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int DISP_RST = 5;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = ORIENTATION;

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
```

SCCB (dentro do `#if` do P4):

```cpp
#define EXAMPLE_MIPI_CSI_SCCB_I2C_PORT    0
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN 8
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN 7
```

`setup()` — três argumentos. Frequência, reset, pwdn, largura e altura ficam no default (`0`, `-1`, `-1`, `0`, `0`):

```cpp
void setup() {
    Serial.begin(115200);
    myDisplay.setRotationCamera(ORIENTATION, CAMERA_MIRROR_X, CAMERA_MIRROR_Y);
    if (!myDisplay.startCamera(EXAMPLE_MIPI_CSI_SCCB_I2C_PORT, EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN, EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN)) {
        Serial.println("failed to start CSI camera");
        return;
    }
    startDisplay();
}

void loop() {
}
```

Painel e touch, dentro de `startDisplay()`:

```cpp
bus = new Arduino_ESP32DSIPanel(
    12, 42, 42,
    2, 8, 166,
    34000000);
tft = new Arduino_DSI_Display(
    480, 800, bus, rotationScreen, true,
    DISP_RST, st7701_init_operations, sizeof(st7701_init_operations) / sizeof(lcd_init_cmd_t));
tft->begin();
myDisplay.getCamera().startPpa();
pinMode(DISP_LED, OUTPUT);
digitalWrite(DISP_LED, HIGH);
myDisplay.setDrawObject(tft);
myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
myDisplay.setSwapAxis(TOUCH_SWAP_XY);
myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
myDisplay.enableTouchLog();
```

Widgets de exemplo: `cam01(10, 10, 0)` 100x100 e `cam02(200, 200, 0)` 200x200, zoom 1, `showZoomLevel = true`.

## camera_10in

Painel DSI JD9365, nativo 800x1280. Orientação de exemplo: 1. Touch GSL3680. INT e RST existem.

Includes iguais ao 4": `SPI.h`, `ESP_Video.h`, `Arduino_GFX_Library.h`, `displaywx.h`.

```cpp
#define ORIENTATION 1
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

const int PANEL_NATIVE_W = 800;
const int PANEL_NATIVE_H = 1280;
#if (ORIENTATION % 2) == 0
const int DISPLAY_W = PANEL_NATIVE_W;
const int DISPLAY_H = PANEL_NATIVE_H;
#else
const int DISPLAY_W = PANEL_NATIVE_H;
const int DISPLAY_H = PANEL_NATIVE_W;
#endif
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;
const uint8_t rotationScreen = ORIENTATION;

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
```

SCCB igual ao 4" (porta 0, SCL 8, SDA 7). `setup()` e `startCamera` de três argumentos, iguais ao 4". `loop()` vazio.

Painel e touch:

```cpp
bus = new Arduino_ESP32DSIPanel(
    20, 20, 40,
    4, 8, 20,
    60000000);
tft = new Arduino_DSI_Display(
    800, 1280, bus, rotationScreen, true,
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
tft->begin();
myDisplay.getCamera().startPpa();
pinMode(DISP_LED, OUTPUT);
digitalWrite(DISP_LED, HIGH);
myDisplay.setDrawObject(tft);
myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
myDisplay.setSwapAxis(TOUCH_SWAP_XY);
myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
myDisplay.enableTouchLog();
```

Widgets de exemplo: `cam01(50, 50, 0)` 200x100 e `cam02(500, 50, 0)` 300x300, zoom 1, `showZoomLevel = true`.

## pico_camera_display

Waveshare 7-DSI-TOUCH-A na ESP32-P4-Pico. O auxiliar do painel (0x45), o GT911 e o SCCB da OV5647 compartilham GPIO 7 e 8. O touch Waveshare não recebe INT nem RST.

Não incluir `SPI.h` nem `ESP_Video.h`.

```cpp
#define ORIENTATION 0
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

const uint8_t rotationScreen = ORIENTATION;
const uint16_t kOv5647Width = 1920;
const uint16_t kOv5647Height = 1080;
const uint32_t kSccbFreqHz = 400000;
const int8_t kUnusedSensorGpio = -1;

Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
```

Timings e pinos já vêm de `src/displays/ili9881c_waveshare_7in.h` quando `DISP_DEFAULT` está ligado: 720x1280, clock 80 MHz, 1000 Mbps/lane, RST -1, SDA 7, SCL 8.

SCCB:

```cpp
#define EXAMPLE_MIPI_CSI_SCCB_I2C_PORT 0
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN 8
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN 7
```

`setup()` passa frequência, reset, pwdn e o tamanho do sensor. Sem largura e altura a OV5647 não sobe no modo usado por este exemplo:

```cpp
void setup()
{
    Serial.begin(115200);
    delay(2000);
    myDisplay.setRotationCamera(ORIENTATION, CAMERA_MIRROR_X, CAMERA_MIRROR_Y);
    if (!myDisplay.startCamera(
            EXAMPLE_MIPI_CSI_SCCB_I2C_PORT,
            EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN,
            EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN,
            kSccbFreqHz,
            kUnusedSensorGpio,
            kUnusedSensorGpio,
            kOv5647Width,
            kOv5647Height)) {
        Serial.println("failed to start CSI camera");
        return;
    }
    startDisplay();
}

void loop()
{
}
```

Painel, backlight do auxiliar e touch. `waveshareDsiAuxBegin` vem antes de criar o barramento DSI. O backlight vem depois do `startPpa`:

```cpp
if (!myDisplay.waveshareDsiAuxBegin(ILI9881C_WAVESHARE_7_SDA, ILI9881C_WAVESHARE_7_SCL)) {
    Serial.println("AVISO: controlador auxiliar do painel falhou");
}

bus = new Arduino_ESP32DSIPanel(
    ILI9881C_WAVESHARE_7_HSYNC_PULSE,
    ILI9881C_WAVESHARE_7_HSYNC_BACK,
    ILI9881C_WAVESHARE_7_HSYNC_FRONT,
    ILI9881C_WAVESHARE_7_VSYNC_PULSE,
    ILI9881C_WAVESHARE_7_VSYNC_BACK,
    ILI9881C_WAVESHARE_7_VSYNC_FRONT,
    ILI9881C_WAVESHARE_7_PIXEL_CLOCK_HZ,
    ILI9881C_WAVESHARE_7_LANE_MBPS);
tft = new Arduino_DSI_Display(
    ILI9881C_WAVESHARE_7_WIDTH,
    ILI9881C_WAVESHARE_7_HEIGHT,
    bus,
    rotationScreen,
    true,
    ILI9881C_WAVESHARE_7_RST,
    ili9881c_waveshare_7_init_operations,
    sizeof(ili9881c_waveshare_7_init_operations) / sizeof(lcd_init_cmd_t));
tft->begin();
myDisplay.getCamera().startPpa();
myDisplay.waveshareDsiAuxBacklight(255);
myDisplay.setDrawObject(tft);
myDisplay.setTouchCorners(0, ILI9881C_WAVESHARE_7_WIDTH, 0, ILI9881C_WAVESHARE_7_HEIGHT);
myDisplay.setInvertAxis(false, false);
myDisplay.setSwapAxis(false);
myDisplay.startTouchGT911Waveshare(
    ILI9881C_WAVESHARE_7_WIDTH,
    ILI9881C_WAVESHARE_7_HEIGHT,
    rotationScreen,
    ILI9881C_WAVESHARE_7_SDA,
    ILI9881C_WAVESHARE_7_SCL);
myDisplay.enableTouchLog();
```

Assinatura nova, sem INT e sem RST:

```cpp
void startTouchGT911Waveshare(uint16_t w, uint16_t h, uint8_t _rotation, int8_t pinSDA, int8_t pinSCL);
```

Widgets de exemplo: `cam01(100, 100, 0)` 200x100 zoom 1 e `cam02(100, 400, 0)` 300x300 zoom 2, `showZoomLevel = false`.

## p4_eye

ESP32-P4-EYE: OV2710 + ST7789 240x240 por SPI. Não há framebuffer DSI. Sem touch, sem `setTouchCorners` / `setInvertAxis` / `setSwapAxis` / `startTouch*` / `enableTouchLog`.

A OV2710 não tem cristal. Antes de `startCamera` o sketch liga a alimentação (GPIO 12, o mesmo enable da câmera e do LCD) e gera o XCLK (GPIO 11, 24 MHz). Sem os dois, o SCCB não responde.

Include extra, nesta ordem: `ESP_Video.h`, `SPI.h`, o limite de DMA, depois a GFX:

```cpp
#include <ESP_Video.h>
#include <SPI.h>
#ifndef ESP32SPIDMA_MAX_PIXELS_AT_ONCE
#define ESP32SPIDMA_MAX_PIXELS_AT_ONCE 4096
#endif
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
```

```cpp
#define ORIENTATION 0
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

const int PANEL_NATIVE_W = 240;
const int PANEL_NATIVE_H = 240;
const int DISPLAY_W = 240;
const int DISPLAY_H = 240;
const int DISP_RST = 15;
const int DISP_DC = 19;
const int DISP_CS = 18;
const int DISP_SCK = 17;
const int DISP_MOSI = 16;
const int DISP_MISO = -1;
const uint8_t rotationScreen = ORIENTATION;
const bool isIPS = true;

#define DISP_LED 20
#define DISP_LED_ON LOW
#define P4_EYE_PWR_EN 12
#define P4_EYE_CAM_XCLK 11
#define P4_EYE_CAM_RST 26
#define P4_EYE_XCLK_HZ 24000000
#define ST7789_SPI_HZ 40000000

Arduino_DataBus *bus = nullptr;
Arduino_ST7789 *tft = nullptr;
```

SCCB neste placa é GPIO 13/14, não 7/8:

```cpp
#define EXAMPLE_MIPI_CSI_SCCB_I2C_PORT    0
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN 13
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN 14
```

`setup()` — power, XCLK, depois `startCamera` com 400 kHz e reset no GPIO 26. Largura e altura ficam no default:

```cpp
void setup() {
    Serial.begin(115200);

    if (!myDisplay.getCamera().enableBoardPower(P4_EYE_PWR_EN)) {
        Serial.println("failed to enable P4-EYE camera/LCD power (GPIO 12)");
        return;
    }
    if (!myDisplay.getCamera().startXclk(P4_EYE_CAM_XCLK, P4_EYE_XCLK_HZ)) {
        return;
    }

    myDisplay.setRotationCamera(ORIENTATION, CAMERA_MIRROR_X, CAMERA_MIRROR_Y);
    if (!myDisplay.startCamera(EXAMPLE_MIPI_CSI_SCCB_I2C_PORT, EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN, EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN, 400000, P4_EYE_CAM_RST, -1)) {
        Serial.println("failed to start CSI camera");
        return;
    }

    startDisplay();
}
```

Painel SPI. Backlight em nível baixo:

```cpp
bus = new Arduino_ESP32SPIDMA(DISP_DC, DISP_CS, DISP_SCK, DISP_MOSI, DISP_MISO, SPI2_HOST + 1, false);
tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS, DISPLAY_W, DISPLAY_H, 0, 0, 0, 0);
tft->begin(ST7789_SPI_HZ);
myDisplay.getCamera().startPpa();
pinMode(DISP_LED, OUTPUT);
digitalWrite(DISP_LED, DISP_LED_ON);
myDisplay.setDrawObject(tft);
```

Widget de exemplo: um só, `cam01(0, 0, 0)`, 240x240, zoom 1, `showZoomLevel = true`.

O exemplo ainda lê três botões (GPIO 3 = zoom 1, GPIO 4 = zoom 2, GPIO 5 = zoom 4) em `loop()` via `cam01.setZoomLevel`. Isso é da placa EYE, não da subida da câmera. O gerador só emite esse `loop()` se o projeto tiver esses botões.

## Assinatura de startCamera

```cpp
bool startCamera(int sccbPort, int8_t sclPin, int8_t sdaPin,
                 uint32_t i2cFreq = 0, int8_t resetPin = -1, int8_t pwdnPin = -1,
                 uint16_t sensorWidth = 0, uint16_t sensorHeight = 0);
```

| Modelo | Chamada |
| --- | --- |
| camera_4in, camera_10in | `startCamera(porta, scl, sda)` |
| pico_camera_display | `startCamera(porta, scl, sda, 400000, -1, -1, 1920, 1080)` |
| p4_eye | `enableBoardPower(12)` e `startXclk(11, 24000000)` antes de `startCamera(porta, scl, sda, 400000, 26, -1)` |

`setRotationCamera(ORIENTATION, CAMERA_MIRROR_X, CAMERA_MIRROR_Y)` vem imediatamente antes de `startCamera` nos quatro.

## Touch — o que o gerador passa a emitir

| Modelo | Define em user_setup.h | Chamada | Pinos |
| --- | --- | --- | --- |
| camera_4in | `TOUCH_GT911` | `startTouchGT911(w, h, rot, sda, scl, int, rst)` | SDA 7, SCL 8, INT -1, RST -1. Cantos 0,800,0,480. Swap XY, inverte Y |
| camera_10in | `TOUCH_GSL3680` | `startTouchGSL3680(w, h, rot, sda, scl, int, rst)` | SDA 7, SCL 8, INT 21, RST 22. Cantos 0,800,0,1280. Sem swap e sem inversão |
| pico_camera_display | `TOUCH_GT911_WAVESHARE` | `startTouchGT911Waveshare(w, h, rot, sda, scl)` | SDA 7, SCL 8. Sem INT e sem RST. Cantos 0, 720, 0, 1280. Sem swap e sem inversão |
| p4_eye | nenhum | nenhuma | — |

Nos três com touch, a sequência antes da chamada é a mesma: `setTouchCorners`, `setInvertAxis`, `setSwapAxis`, o `startTouch*`, `enableTouchLog()`.

## Mapa rápido para o gerador

| | camera_4in | camera_10in | pico_camera_display | p4_eye |
| --- | --- | --- | --- | --- |
| Sensor | `CAM_OV02C10` | `CAM_OV02C10` | `CAM_OV5647` | `CAM_OV2710` |
| Touch define | `TOUCH_GT911` | `TOUCH_GSL3680` | `TOUCH_GT911_WAVESHARE` + `DISP_DEFAULT` | — |
| Barramento | DSI ST7701 480x800, 34 MHz | DSI JD9365 800x1280, 60 MHz | DSI ILI9881C 720x1280, 80 MHz, lane 1000 | SPI ST7789 240x240, 40 MHz |
| Reset do painel | GPIO 5 | GPIO 27 | -1 | GPIO 15 |
| Backlight | GPIO 23, HIGH | GPIO 23, HIGH | `waveshareDsiAuxBacklight(255)` | GPIO 20, LOW |
| SCCB | porta 0, SCL 8, SDA 7 | porta 0, SCL 8, SDA 7 | porta 0, SCL 8, SDA 7, 400 kHz, 1920x1080 | porta 0, SCL 13, SDA 14, 400 kHz, RST 26 |
| Extra antes da câmera | — | — | `delay(2000)` | `enableBoardPower(12)`, `startXclk(11, 24 MHz)` |
| Extra do painel | — | — | `waveshareDsiAuxBegin(7, 8)` antes do bus | `ESP32SPIDMA_MAX_PIXELS_AT_ONCE 4096` antes da GFX |
