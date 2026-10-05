/**
 * Teste DMA puro: OV02C10 MIPI-CSI → PPA → framebuffer DSI (JD9365 10").
 *
 * Sem toque, sem widgets, sem texto, sem draw16bitRGBBitmap.
 * A captura V4L2 e o PPA (cópia 2D, rotação 0) são DMA; o DSI lê o framebuffer sozinho.
 * Sem rotação: menos tráfego de memória. Crop 800x1080 no FB nativo 800x1280.
 * Não há zero-copy: 1920x1080 da câmera não cabe no painel.
 *
 * user_setup.h: HAS_CAMERA_MIPI + CAM_OV02C10
 * Serial 115200 só no setup.
 */
#include "Arduino.h"
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>
#include <ESP_Video.h>
#include <esp_cache.h>
#include <driver/ppa.h>

const int PANEL_NATIVE_W = 800;
const int PANEL_NATIVE_H = 1280;
const int DISP_RST = 27;
const uint8_t rotationScreen = 0;
#define DISP_LED 23

Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayFK myDisplay;
ppa_client_handle_t ppa_srm = nullptr;

#if !defined(HAS_CAMERA_MIPI) || !defined(CAM_OV02C10)
#error "This example requires HAS_CAMERA_MIPI and CAM_OV02C10 in user_setup.h"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
#if CONFIG_IDF_TARGET_ESP32P4
#define EXAMPLE_MIPI_CSI_SCCB_I2C_PORT    0
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN 8
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN 7
#else
#error "The selected target SoC is not supported"
#endif

static bool blitCameraDmaFullScreen(const uint16_t *src, uint32_t cam_w, uint32_t cam_h)
{
  if (!tft || !ppa_srm || !src || cam_w == 0 || cam_h == 0) {
    return false;
  }

  uint16_t *fb = tft->getFramebuffer();
  if (!fb) {
    return false;
  }

  uint32_t crop_w = cam_w;
  uint32_t crop_h = cam_h;
  if (crop_w > (uint32_t)PANEL_NATIVE_W) {
    crop_w = PANEL_NATIVE_W;
  }
  if (crop_h > (uint32_t)PANEL_NATIVE_H) {
    crop_h = PANEL_NATIVE_H;
  }
  crop_w &= ~1u;
  crop_h &= ~1u;
  if (crop_w == 0 || crop_h == 0) {
    return false;
  }

  size_t fb_bytes = (size_t)PANEL_NATIVE_W * PANEL_NATIVE_H * sizeof(uint16_t);

  ppa_srm_oper_config_t op = {};
  op.in.buffer = src;
  op.in.pic_w = cam_w;
  op.in.pic_h = cam_h;
  op.in.block_w = crop_w;
  op.in.block_h = crop_h;
  op.in.srm_cm = PPA_SRM_COLOR_MODE_RGB565;
  op.out.buffer = fb;
  op.out.buffer_size = fb_bytes;
  op.out.pic_w = PANEL_NATIVE_W;
  op.out.pic_h = PANEL_NATIVE_H;
  op.out.srm_cm = PPA_SRM_COLOR_MODE_RGB565;
  op.rotation_angle = PPA_SRM_ROTATION_ANGLE_0;
  op.scale_x = 1.0f;
  op.scale_y = 1.0f;
  op.mode = PPA_TRANS_MODE_BLOCKING;

  if (ppa_do_scale_rotate_mirror(ppa_srm, &op) != ESP_OK) {
    return false;
  }

  size_t out_bytes = (size_t)crop_h * PANEL_NATIVE_W * sizeof(uint16_t);
  const uintptr_t cache_line = 128;
  uintptr_t start = (uintptr_t)fb & ~(cache_line - 1);
  uintptr_t end = ((uintptr_t)fb + out_bytes + cache_line - 1) & ~(cache_line - 1);
  esp_cache_msync((void *)start, (size_t)(end - start), ESP_CACHE_MSYNC_FLAG_DIR_C2M);
  return true;
}

static bool startPpa()
{
  ppa_client_config_t cfg = {};
  cfg.oper_type = PPA_OPERATION_SRM;
  cfg.max_pending_trans_num = 1;
  cfg.data_burst_length = PPA_DATA_BURST_LENGTH_128;
  if (ppa_register_client(&cfg, &ppa_srm) != ESP_OK) {
    ppa_srm = nullptr;
    Serial.println("failed to start PPA DMA client");
    return false;
  }
  Serial.println("PPA DMA client ready");
  return true;
}

static void startDisplay()
{
  bus = new Arduino_ESP32DSIPanel(
    20, 20, 40,
    4, 8, 20,
    60000000);
  tft = new Arduino_DSI_Display(
    800, 1280, bus, rotationScreen, true,
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
  tft->begin();
  startPpa();
#if defined(DISP_LED)
  pinMode(DISP_LED, OUTPUT);
  digitalWrite(DISP_LED, HIGH);
#endif
  Serial.println("Display started");
}

void setup()
{
  Serial.begin(115200);

  if (!myDisplay.getCamera().begin(EXAMPLE_MIPI_CSI_SCCB_I2C_PORT, EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN, EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN)) {
    Serial.println("failed to start CSI camera");
    return;
  }

  startDisplay();
}

void loop()
{
  CameraController &camera = myDisplay.getCamera();
  if (!camera.isOpened() || !camera.isCaptureStarted()) {
    delay(1000);
    return;
  }

  if (!camera.captureFrame()) {
    delay(10);
    return;
  }

  ESPVideoBufferClass *buffer = camera.buffer();
  if (!buffer) {
    delay(10);
    return;
  }

  blitCameraDmaFullScreen((const uint16_t *)buffer->data(), buffer->getWidth(), buffer->getHeight());
  camera.releaseFrame();
}

#else
void setup()
{
  Serial.begin(115200);
  Serial.println("ESP_IDF_VERSION < 5.4.0, this example is not supported");
}

void loop()
{
  delay(1000);
}
#endif
