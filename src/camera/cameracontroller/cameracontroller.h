#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "../../../user_setup.h"
#include "../../check_camera.h"
#include "../../widgets/widgetsetup.h"

#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)

#include <Arduino.h>
#include <ESP_Video.h>
#include <freertos/semphr.h>
#if defined(CONFIG_IDF_TARGET_ESP32P4)
#include <driver/ppa.h>
#include "esp_cam_sensor_xclk.h"
#else
typedef struct ppa_client_t *ppa_client_handle_t;
typedef void *esp_cam_sensor_xclk_handle_t;
#endif

class CameraController
{
public:
    CameraController();

    bool begin(int sccbPort, int8_t sclPin, int8_t sdaPin, uint32_t i2cFreq = 0, int8_t resetPin = -1, int8_t pwdnPin = -1, uint16_t sensorWidth = 0, uint16_t sensorHeight = 0);
    bool enableBoardPower(int8_t pin, bool activeHigh = true, uint32_t settleMs = 20);
    bool startXclk(int8_t pin, uint32_t freqHz);
    bool startPpa();
    bool captureFrame();
    void releaseFrame();
    bool isReady() const;
    bool isOpened() const;
    bool isCaptureStarted() const;
    ESPVideoBufferClass *buffer();
    ppa_client_handle_t ppaClient() const;

private:
    void releaseAfterFailedStart();
    void markReady();
    bool startWithFormat(esp_video_format_t format, const char *formatName);
    bool selectSensorSize(uint16_t width, uint16_t height);
    bool sensorUsesRaw10() const;
    const char *sensorName() const;
    esp_video_format_t fallbackFormat() const;
    const char *fallbackFormatName() const;

    ESPVideoClass m_video;
    ESPVideoCaptureDevClass m_captureDev;
    ESPVideoBufferClass m_frame;
    ESPVideoBufferClass *m_buffer;
    ppa_client_handle_t m_ppaSrm;
    esp_cam_sensor_xclk_handle_t m_xclk;
    SemaphoreHandle_t m_cameraBusy;
    StaticSemaphore_t m_cameraBusyBuffer;
    uint16_t m_sensorWidth;
    uint16_t m_sensorHeight;
    bool m_ready;
    static const size_t kCaptureBufferCount = 2;
};

#else

#pragma message("Camera controller not enabled")


#endif

#endif
