#include "cameracontroller.h"
#include "../../touch/share_i2c.h"

#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)


#include "esp_log.h"
#if defined(CONFIG_IDF_TARGET_ESP32P4)
#include <driver/rtc_io.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#endif

static const char *TAG = "CameraController";

#if defined(CAM_OV02C10)
extern "C" void displaywx_link_ov02c10_hook(void);
extern "C" void displaywx_link_ov02c10_ipa(void);
#endif
#if defined(CAM_OV5647)
extern "C" void displaywx_link_ov5647_hook(void);
extern "C" void displaywx_link_ov5647_ipa(void);
#include "../ov5647/ov5647/ov5647.h"
#endif
#if defined(CAM_SC2336)
extern "C" void displaywx_link_sc2336_hook(void);
extern "C" void displaywx_link_sc2336_ipa(void);
#endif
#if defined(CAM_OV2710)
extern "C" void displaywx_link_ov2710_hook(void);
extern "C" void displaywx_link_ov2710_ipa(void);
#endif

static void pullCameraDriverIntoLink(void)
{
#if defined(CAM_OV02C10)
    displaywx_link_ov02c10_hook();
    displaywx_link_ov02c10_ipa();
#elif defined(CAM_OV5647)
    displaywx_link_ov5647_hook();
    displaywx_link_ov5647_ipa();
#elif defined(CAM_SC2336)
    displaywx_link_sc2336_hook();
    displaywx_link_sc2336_ipa();
#elif defined(CAM_OV2710)
    displaywx_link_ov2710_hook();
    displaywx_link_ov2710_ipa();
#endif
}

CameraController::CameraController() : m_buffer(nullptr), m_ppaSrm(nullptr), m_xclk(nullptr), m_cameraBusy(nullptr), m_sensorWidth(0), m_sensorHeight(0), m_ready(false)
{
    m_cameraBusy = xSemaphoreCreateMutexStatic(&m_cameraBusyBuffer);
}

bool CameraController::sensorUsesRaw10() const
{
    if (m_sensorWidth == 1920 && m_sensorHeight == 1080) {
        return true;
    }
    return m_sensorWidth == 1280 && m_sensorHeight == 960;
}

bool CameraController::selectSensorSize(uint16_t width, uint16_t height)
{
#if defined(CAM_OV5647)
    if (width == 0 && height == 0) {
        return true;
    }
    if (ov5647_select_mipi_size(width, height) != ESP_OK) {
        Serial.printf("failed to select OV5647 MIPI size %ux%u\n", width, height);
        ESP_LOGE(TAG, "failed to select OV5647 MIPI size %ux%u", width, height);
        return false;
    }
    m_sensorWidth = width;
    m_sensorHeight = height;
    Serial.printf("OV5647 MIPI size selected: %ux%u\n", width, height);
    ESP_LOGI(TAG, "OV5647 MIPI size selected: %ux%u", width, height);
    return true;
#else
    if (width != 0 || height != 0) {
        ESP_LOGW(TAG, "sensor size %ux%u ignored for %s", width, height, sensorName());
    }
    return true;
#endif
}

const char *CameraController::sensorName() const
{
#if defined(CAM_OV02C10)
    return "OV02C10";
#elif defined(CAM_OV5647)
    return "OV5647";
#elif defined(CAM_SC2336)
    return "SC2336";
#elif defined(CAM_OV2710)
    return "OV2710";
#else
    return "unknown";
#endif
}

esp_video_format_t CameraController::fallbackFormat() const
{
#if defined(CAM_OV02C10)
    return ESP_VIDEO_FORMAT_RAW10;
#elif defined(CAM_OV5647)
    if (sensorUsesRaw10()) {
        return ESP_VIDEO_FORMAT_RAW10;
    }
    return ESP_VIDEO_FORMAT_RAW8;
#elif defined(CAM_SC2336)
    return ESP_VIDEO_FORMAT_RAW8;
#elif defined(CAM_OV2710)
    return ESP_VIDEO_FORMAT_RAW8;
#else
    return ESP_VIDEO_FORMAT_RAW10;
#endif
}

const char *CameraController::fallbackFormatName() const
{
#if defined(CAM_OV02C10)
    return "RAW10";
#elif defined(CAM_OV5647)
    if (sensorUsesRaw10()) {
        return "RAW10";
    }
    return "RAW8";
#elif defined(CAM_SC2336)
    return "RAW8";
#elif defined(CAM_OV2710)
    return "RAW8";
#else
    return "RAW10";
#endif
}

bool CameraController::startWithFormat(esp_video_format_t format, const char *formatName)
{
    if (!m_captureDev.setFormat(format)) {
        ESP_LOGE(TAG, "failed to set format %s", formatName);
        return false;
    }

    if (!m_captureDev.startCapture()) {
        ESP_LOGE(TAG, "failed to start capture with %s", formatName);
        return false;
    }

    ESP_LOGI(TAG, "Arduino %s capture started (%s)", sensorName(), formatName);
    Serial.printf("Arduino %s capture started (%s)\n", sensorName(), formatName);
    return true;
}

void CameraController::releaseAfterFailedStart()
{
    m_ready = false;
    m_buffer = nullptr;
    if (m_captureDev.isOpened()) {
        m_captureDev.end();
    }
    m_video.end();
}

void CameraController::markReady()
{
    m_ready = true;
    Serial.printf("%s detected, CSI video device ready\n", sensorName());
    ESP_LOGI(TAG, "%s detected, CSI video device ready", sensorName());
}

bool CameraController::begin(int sccbPort, int8_t sclPin, int8_t sdaPin, uint32_t i2cFreq, int8_t resetPin, int8_t pwdnPin, uint16_t sensorWidth, uint16_t sensorHeight)
{
    m_ready = false;
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
    ESP_LOGE(TAG, "ESP_IDF_VERSION < 5.4.0, CSI camera is not supported");
    return false;
#elif !defined(CONFIG_IDF_TARGET_ESP32P4)
    ESP_LOGE(TAG, "The selected target SoC is not supported");
    return false;
#else
    struct SharedBusLock {
        bool held;
        SharedBusLock(int8_t sda, int8_t scl) : held(false)
        {
            if (TouchI2c.usesPins(sda, scl)) {
                held = TouchI2c.lock();
            }
        }
        ~SharedBusLock()
        {
            if (held) {
                TouchI2c.unlock();
            }
        }
    };

    const bool sharesTouchBus = TouchI2c.usesPins(sdaPin, sclPin);
    SharedBusLock busLock(sdaPin, sclPin);
    if (sharesTouchBus && !busLock.held) {
        ESP_LOGE(TAG, "shared I2C lock timed out");
        return false;
    }

    m_buffer = nullptr;
    pullCameraDriverIntoLink();
    if (!selectSensorSize(sensorWidth, sensorHeight)) {
        return false;
    }

    ESPVideoCamConfigClass camConfig;
    if (i2cFreq == 0 && resetPin < 0 && pwdnPin < 0)
    {
        camConfig.begin(static_cast<i2c_port_num_t>(sccbPort), sclPin, sdaPin);
    }
    else
    {
        uint32_t freq = (i2cFreq == 0) ? 400000 : i2cFreq;
        camConfig.begin(static_cast<i2c_port_num_t>(sccbPort), sclPin, sdaPin, freq, resetPin, pwdnPin);
    }

    ESPVideoCSIConfigClass csiConfig;
    csiConfig.begin(camConfig);

    if (!m_video.begin(csiConfig)) {
        Serial.printf("failed to detect %s / init CSI camera\n", sensorName());
        ESP_LOGE(TAG, "failed to detect %s / init CSI camera", sensorName());
        releaseAfterFailedStart();
        return false;
    }

    if (!m_captureDev.begin(ESP_VIDEO_MIPI_CSI_DEVICE_NAME, kCaptureBufferCount)) {
        Serial.println("failed to open capture device");
        ESP_LOGE(TAG, "failed to open capture device");
        releaseAfterFailedStart();
        return false;
    }
    Serial.println("capture device opened");
    ESP_LOGI(TAG, "capture device opened");

    if (startWithFormat(ESP_VIDEO_FORMAT_RGB565, "RGB565")) {
        markReady();
        return true;
    }

    if (startWithFormat(fallbackFormat(), fallbackFormatName())) {
        markReady();
        return true;
    }

    Serial.printf("%s capture did not start, releasing SCCB pins\n", sensorName());
    ESP_LOGE(TAG, "%s capture did not start, releasing SCCB pins", sensorName());
    releaseAfterFailedStart();
    return false;
#endif
}

bool CameraController::enableBoardPower(int8_t pin, bool activeHigh, uint32_t settleMs)
{
#if !defined(CONFIG_IDF_TARGET_ESP32P4)
    ESP_LOGE(TAG, "enableBoardPower is only available on ESP32-P4");
    return false;
#else
    if (pin < 0)
    {
        ESP_LOGE(TAG, "invalid board power pin");
        return false;
    }

    gpio_num_t gpio = static_cast<gpio_num_t>(pin);
    uint32_t level = activeHigh ? 1 : 0;

    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_ON);
    if (rtc_gpio_init(gpio) != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to init RTC GPIO %d for board power", pin);
        return false;
    }
    rtc_gpio_set_direction(gpio, RTC_GPIO_MODE_OUTPUT_ONLY);
    rtc_gpio_pulldown_dis(gpio);
    rtc_gpio_pullup_dis(gpio);
    rtc_gpio_hold_dis(gpio);
    rtc_gpio_set_level(gpio, level);
    rtc_gpio_hold_en(gpio);
    delay(settleMs);
    Serial.printf("board power GPIO %d set %s\n", pin, activeHigh ? "HIGH" : "LOW");
    ESP_LOGI(TAG, "board power GPIO %d set %s", pin, activeHigh ? "HIGH" : "LOW");
    return true;
#endif
}

bool CameraController::startXclk(int8_t pin, uint32_t freqHz)
{
#if !defined(CONFIG_IDF_TARGET_ESP32P4)
    ESP_LOGE(TAG, "startXclk is only available on ESP32-P4");
    return false;
#else
    if (pin < 0 || freqHz == 0)
    {
        ESP_LOGE(TAG, "invalid XCLK pin or frequency");
        return false;
    }
    if (m_xclk)
    {
        return true;
    }

    esp_cam_sensor_xclk_config_t cfg = {};
    cfg.esp_clock_router_cfg.xclk_pin = static_cast<gpio_num_t>(pin);
    cfg.esp_clock_router_cfg.xclk_freq_hz = freqHz;
    if (esp_cam_sensor_xclk_allocate(ESP_CAM_SENSOR_XCLK_ESP_CLOCK_ROUTER, &m_xclk) != ESP_OK)
    {
        m_xclk = nullptr;
        Serial.println("failed to allocate camera XCLK");
        ESP_LOGE(TAG, "failed to allocate camera XCLK");
        return false;
    }
    if (esp_cam_sensor_xclk_start(m_xclk, &cfg) != ESP_OK)
    {
        Serial.printf("failed to start camera XCLK %lu Hz\n", (unsigned long)freqHz);
        ESP_LOGE(TAG, "failed to start camera XCLK");
        return false;
    }
    Serial.printf("camera XCLK %lu Hz on GPIO %d\n", (unsigned long)freqHz, pin);
    ESP_LOGI(TAG, "camera XCLK %lu Hz on GPIO %d", (unsigned long)freqHz, pin);
    return true;
#endif
}

bool CameraController::startPpa()
{
    if (!m_ready) {
        ESP_LOGW(TAG, "camera not ready, skipping PPA");
        return false;
    }
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
    ESP_LOGE(TAG, "ESP_IDF_VERSION < 5.4.0, PPA is not supported");
    return false;
#elif !defined(CONFIG_IDF_TARGET_ESP32P4)
    ESP_LOGE(TAG, "PPA DMA is only available on ESP32-P4");
    return false;
#else
    if (m_ppaSrm) {
        return true;
    }

    ppa_client_config_t cfg = {};
    cfg.oper_type = PPA_OPERATION_SRM;
    cfg.max_pending_trans_num = 1;
    cfg.data_burst_length = PPA_DATA_BURST_LENGTH_128;
    if (ppa_register_client(&cfg, &m_ppaSrm) != ESP_OK) {
        m_ppaSrm = nullptr;
        Serial.println("failed to start PPA DMA client");
        ESP_LOGE(TAG, "failed to start PPA DMA client");
        return false;
    }

    Serial.println("PPA DMA client ready");
    ESP_LOGI(TAG, "PPA DMA client ready");
    return true;
#endif
}

bool CameraController::captureFrame()
{
    if (!m_ready || m_cameraBusy == nullptr)
    {
        return false;
    }

    const bool alreadyHeld = xSemaphoreGetMutexHolder(m_cameraBusy) == xTaskGetCurrentTaskHandle();
    if (!alreadyHeld && xSemaphoreTake(m_cameraBusy, 0) != pdTRUE)
    {
        return false;
    }

    m_buffer = nullptr;
    m_frame.end();

    if (!m_captureDev.isOpened() || !m_captureDev.isCaptureStarted())
    {
        xSemaphoreGive(m_cameraBusy);
        return false;
    }

    m_frame = m_captureDev.captureBuffer();
    if (!m_frame.valid())
    {
        ESP_LOGE(TAG, "failed to capture buffer");
        xSemaphoreGive(m_cameraBusy);
        return false;
    }

    m_buffer = &m_frame;
    return true;
}

void CameraController::releaseFrame()
{
    if (m_cameraBusy == nullptr || xSemaphoreGetMutexHolder(m_cameraBusy) != xTaskGetCurrentTaskHandle())
    {
        return;
    }

    m_buffer = nullptr;
    m_frame.end();
    xSemaphoreGive(m_cameraBusy);
}

bool CameraController::isReady() const
{
    return m_ready;
}

bool CameraController::isOpened() const
{
    return m_captureDev.isOpened();
}

bool CameraController::isCaptureStarted() const
{
    return m_captureDev.isCaptureStarted();
}

ESPVideoBufferClass *CameraController::buffer()
{
    return m_buffer;
}

ppa_client_handle_t CameraController::ppaClient() const
{
    return m_ppaSrm;
}

#endif
