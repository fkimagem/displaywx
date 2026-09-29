/**
 * Captura MIPI-CSI na Waveshare ESP32-P4-Pico com camera Raspberry Pi OV5647.
 *
 * O DisplayFK inicia a camera. O CameraController chama ov5647_select_mipi_size(1920, 1080)
 * antes de video.begin() e imprime no Serial o resultado de cada frame.
 *
 * Hardware Pico: SCCB GPIO 8/7; CSI 2-lane no conector FPC. XCLK/RST nao
 * vem do MCU (cristal de 24 MHz no modulo Raspberry).
 *
 * user_setup.h: USE_CAMERA_CSI + CAM_OV5647
 * Serial 115200.
 * Placa: https://www.waveshare.com/wiki/ESP32-P4-Pico
 */
#include "Arduino.h"
#include <displayfk.h>

#define ORIENTATION 0
#define CAMERA_MIRROR_X false
#define CAMERA_MIRROR_Y false

#if (ORIENTATION < 0) || (ORIENTATION > 3)
#error "ORIENTATION must be 0, 1, 2 or 3"
#endif

#if !defined(USE_CAMERA_CSI) || !defined(CAM_OV5647)
#error "This example requires USE_CAMERA_CSI and CAM_OV5647 in user_setup.h"
#endif

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)
#if CONFIG_IDF_TARGET_ESP32P4
#define EXAMPLE_MIPI_CSI_SCCB_I2C_PORT    0
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SCL_PIN 8
#define EXAMPLE_MIPI_CSI_SCCB_I2C_SDA_PIN 7
#else
#error "The selected target SoC is not supported"
#endif

DisplayFK myDisplay;
const uint16_t kOv5647Width = 1920;
const uint16_t kOv5647Height = 1080;
const uint32_t kSccbFreqHz = 400000;
const int8_t kUnusedSensorGpio = -1;

static void printFrame(ESPVideoBufferClass *buffer)
{
    Serial.printf(
        "frame capture ok: ptr=%p size=%lu format=%s width=%lu height=%lu\n",
        buffer->data(),
        (unsigned long)buffer->size(),
        buffer->formatName(),
        (unsigned long)buffer->getWidth(),
        (unsigned long)buffer->getHeight()
    );
}

void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32-P4-Pico OV5647: starting camera");

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
    Serial.println("camera init ok: stream running");
}

void loop()
{
    CameraController &camera = myDisplay.getCamera();
    if (!camera.isOpened() || !camera.isCaptureStarted()) {
        Serial.println("capture skipped: camera not opened or stream not started");
        delay(1000);
        return;
    }

    if (!camera.captureFrame()) {
        Serial.println("frame capture failed");
        delay(10);
        return;
    }

    ESPVideoBufferClass *buffer = camera.buffer();
    if (buffer == nullptr) {
        Serial.println("frame capture failed");
        delay(10);
        return;
    }

    printFrame(buffer);
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
