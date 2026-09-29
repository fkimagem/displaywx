/**
 * Waveshare 7-DSI-TOUCH-A (ILI9881C 720x1280) na ESP32-P4-Pico.
 *
 * O controlador auxiliar 0x45 (alimentacao e backlight) usa o mesmo I2C do GT911.
 * startTouchGT911Waveshare reutiliza esse barramento, sonda 0x5D e 0x14
 * e nao grava a configuracao do chip.
 *
 * user_setup.h: DISP_DEFAULT + TOUCH_GT911_WAVESHARE
 * Serial 115200.
 * Placa: https://www.waveshare.com/wiki/ESP32-P4-Pico
 */
#include "Arduino.h"
#include <Arduino_GFX_Library.h>
#include <displayfk.h>

#if !defined(DISP_DEFAULT) || !defined(TOUCH_GT911_WAVESHARE)
#error "This example requires DISP_DEFAULT and TOUCH_GT911_WAVESHARE in user_setup.h"
#endif

#if !defined(CONFIG_IDF_TARGET_ESP32P4)
#error "The selected target SoC is not supported"
#endif

const uint8_t rotationScreen = 0;

Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayFK myDisplay;

void screen0();

void drawTest()
{
    const int panelWidth = tft->width();
    const int panelHeight = tft->height();

    tft->fillScreen(RGB565_BLACK);
    tft->fillRect(panelWidth / 4, panelHeight / 4, panelWidth / 2, panelHeight / 2, RGB565_RED);
    tft->fillRect(0, 0, 100, 100, RGB565_GREEN);
    tft->fillRect(panelWidth - 100, panelHeight - 100, 100, 100, RGB565_BLUE);
    tft->setTextColor(RGB565_WHITE);
    tft->setTextSize(1);
    tft->setCursor(30, 130);
    tft->println("ESP32-P4-Pico");
    tft->setCursor(30, 170);
    tft->println("7-DSI-TOUCH-A");
    tft->setCursor(30, 210);
    tft->println("ILI9881C");
}

void screen0()
{
    drawTest();
}

void setup()
{
    Serial.begin(115200);
    delay(2000);
    Serial.println("ESP32-P4-Pico Waveshare 7-DSI-TOUCH-A");

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

    if (!tft->begin()) {
        Serial.println("ERRO: tft->begin() falhou");
        return;
    }

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
    myDisplay.loadScreen(screen0);
    myDisplay.createTask(false, 3);
    delay(2000);
    Serial.println("Display e GT911 iniciados");
}

void loop()
{
}
