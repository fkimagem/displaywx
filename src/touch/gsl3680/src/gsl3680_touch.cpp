#define CONFIG_I2C_ENABLE_LEGACY_DRIVER 1

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "../../../extras/check_version.h"
#include "../../../../user_setup.h"
#include "../../../check_camera.h"

#if defined(ESP_ARDUINO_VERSION_OK)
#include "driver/i2c_master.h"  // Driver NOVO
#include "esp_lcd_touch.h"
#include "esp_lcd_gsl3680.h"
#if defined(HAS_CAMERA_MIPI)
#include "esp32-hal-periman.h"
#endif
#endif

#include "gsl3680_touch.h"


#include <Arduino.h>

#define CONFIG_LCD_HRES 800
#define CONFIG_LCD_VRES 1280

static const char *TAG = "GSL3680";

#if defined(ESP_ARDUINO_VERSION_OK) && defined(HAS_CAMERA_MIPI)
static bool pinIsI2cSda(int8_t pin)
{
    peripheral_bus_type_t type = perimanGetPinBusType((uint8_t)pin);
    if (type == ESP32_BUS_TYPE_I2C_MASTER_SDA) {
        return true;
    }
#if SOC_LCDCAM_CAM_SUPPORTED
    if (type == ESP32_BUS_TYPE_VIDEO_SCCB_SDA) {
        return true;
    }
#endif
    return false;
}

static bool pinIsI2cScl(int8_t pin)
{
    peripheral_bus_type_t type = perimanGetPinBusType((uint8_t)pin);
    if (type == ESP32_BUS_TYPE_I2C_MASTER_SCL) {
        return true;
    }
#if SOC_LCDCAM_CAM_SUPPORTED
    if (type == ESP32_BUS_TYPE_VIDEO_SCCB_SCL) {
        return true;
    }
#endif
    return false;
}

static bool pinIsFree(int8_t pin)
{
    return perimanGetPinBusType((uint8_t)pin) == ESP32_BUS_TYPE_INIT;
}

static bool portIsFree(i2c_port_num_t port)
{
    i2c_master_bus_handle_t unused = NULL;
    return i2c_master_get_bus_handle(port, &unused) != ESP_OK;
}

static bool tryReuseBusOnPins(int8_t sda, int8_t scl, i2c_master_bus_handle_t *handle)
{
    if (!pinIsI2cSda(sda)) {
        return false;
    }
    if (!pinIsI2cScl(scl)) {
        return false;
    }

    int8_t sdaPort = perimanGetPinBusNum((uint8_t)sda);
    int8_t sclPort = perimanGetPinBusNum((uint8_t)scl);
    if (sdaPort < 0) {
        return false;
    }
    if (sdaPort != sclPort) {
        return false;
    }

    esp_err_t err = i2c_master_get_bus_handle((i2c_port_num_t)sdaPort, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C port %d marked on pins but has no bus handle", sdaPort);
        return false;
    }

    ESP_LOGI(TAG, "reusing I2C port %d", sdaPort);
    return true;
}

static bool registerPinsOnPeriman(int8_t sda, int8_t scl, i2c_port_num_t port)
{
    void *bus = (void *)(port + 1);
    if (!perimanSetPinBus((uint8_t)sda, ESP32_BUS_TYPE_I2C_MASTER_SDA, bus, (int8_t)port, -1)) {
        ESP_LOGE(TAG, "Failed to register SDA pin on periman");
        return false;
    }
    if (!perimanSetPinBus((uint8_t)scl, ESP32_BUS_TYPE_I2C_MASTER_SCL, bus, (int8_t)port, -1)) {
        ESP_LOGE(TAG, "Failed to register SCL pin on periman");
        return false;
    }
    return true;
}

static bool createMasterBusOnPort(i2c_port_num_t port, int8_t sda, int8_t scl, i2c_master_bus_handle_t *handle)
{
    i2c_master_bus_config_t config = {};
    config.i2c_port = port;
    config.sda_io_num = (gpio_num_t)sda;
    config.scl_io_num = (gpio_num_t)scl;
    config.clk_source = I2C_CLK_SRC_DEFAULT;
    config.glitch_ignore_cnt = 7;
    config.flags.enable_internal_pullup = true;

    esp_err_t err = i2c_new_master_bus(&config, handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
        return false;
    }
    return true;
}

static bool tryCreateOnPort(i2c_port_num_t port, int8_t sda, int8_t scl, i2c_master_bus_handle_t *handle)
{
    if (!portIsFree(port)) {
        return false;
    }
    if (!createMasterBusOnPort(port, sda, scl, handle)) {
        return false;
    }
    if (!registerPinsOnPeriman(sda, scl, port)) {
        return false;
    }

    ESP_LOGI(TAG, "created I2C port %d", (int)port);
    return true;
}

static bool createBusOnFreePort(int8_t sda, int8_t scl, i2c_master_bus_handle_t *handle)
{
    if (!pinIsFree(sda)) {
        return false;
    }
    if (!pinIsFree(scl)) {
        return false;
    }
    if (tryCreateOnPort(I2C_NUM_0, sda, scl, handle)) {
        return true;
    }
#if SOC_HP_I2C_NUM > 1
    if (tryCreateOnPort(I2C_NUM_1, sda, scl, handle)) {
        return true;
    }
#endif
    ESP_LOGE(TAG, "No free HP I2C port for GSL3680");
    return false;
}
#endif

#if defined(ESP_ARDUINO_VERSION_OK)
esp_lcd_touch_handle_t tp;
esp_lcd_panel_io_handle_t tp_io_handle;
i2c_master_bus_handle_t i2c_bus_handle;  // Handle do novo driver
#endif


uint16_t touch_strength[1];
uint8_t touch_cnt = 0;

GSL3680_touch::GSL3680_touch(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
{
    #if defined(ESP_ARDUINO_VERSION_OK)
    _sda = sda_pin;
    _scl = scl_pin;
    _rst = rst_pin;
    _int = int_pin;
    ESP_LOGD(TAG, "Constructor GSL3680_touch");
    Serial.println("Constructor GSL3680_touch");
    #else
    ESP_LOGE(TAG, "GSL3680_touch not supported. Constructor not called");
    #endif
}

void GSL3680_touch::begin()
{
    #if defined(ESP_ARDUINO_VERSION_OK)
    #if defined(HAS_CAMERA_MIPI)
    bool busReady = tryReuseBusOnPins(_sda, _scl, &i2c_bus_handle);
    if (!busReady) {
        busReady = createBusOnFreePort(_sda, _scl, &i2c_bus_handle);
    }
    if (!busReady) {
        ESP_LOGE(TAG, "GSL3680 I2C pins are busy; cannot share or create bus");
        return;
    }
    #else
    // Configuração do novo driver I2C Master
    i2c_master_bus_config_t i2c_bus_config = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = (gpio_num_t)_sda,
        .scl_io_num = (gpio_num_t)_scl,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle));
    #endif

    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GSL3680_CONFIG();
    tp_io_config.scl_speed_hz = 400000;
    ESP_LOGD(TAG, "Initialize touch IO (I2C)");
    // esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_NUM_1, &tp_io_config, &tp_io_handle);
    esp_lcd_new_panel_io_i2c(i2c_bus_handle, &tp_io_config, &tp_io_handle);

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = CONFIG_LCD_HRES,
        .y_max = CONFIG_LCD_VRES,
        .rst_gpio_num = (gpio_num_t)_rst,
        .int_gpio_num = (gpio_num_t)_int,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 1,
        },
    };

    ESP_LOGD(TAG, "Initialize touch controller gsl3680");
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gsl3680(tp_io_handle, &tp_cfg, &tp));
    #else
    ESP_LOGE(TAG, "GSL3680_touch not supported. begin() not called");
    #endif
}

bool GSL3680_touch::getTouch(uint16_t *x, uint16_t *y)
{
    #if defined(ESP_ARDUINO_VERSION_OK)
    if (tp == nullptr || x == nullptr || y == nullptr) {
        return false;
    }
    esp_lcd_touch_read_data(tp);
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, x, y, touch_strength, &touch_cnt, 1);

    return touchpad_pressed;
    #else
    ESP_LOGE(TAG, "GSL3680_touch not supported. getTouch() not called");
    return false;
    #endif
}

void GSL3680_touch::set_rotation(uint8_t r){
    #if defined(ESP_ARDUINO_VERSION_OK)
switch(r){
    case 0:
        esp_lcd_touch_set_swap_xy(tp, false);   
        esp_lcd_touch_set_mirror_x(tp, true);//false
        esp_lcd_touch_set_mirror_y(tp, false);//false
        break;
    case 1:
        esp_lcd_touch_set_swap_xy(tp, true);
        esp_lcd_touch_set_mirror_x(tp, false);
        esp_lcd_touch_set_mirror_y(tp, false);
        break;
    case 2:
        esp_lcd_touch_set_swap_xy(tp, false);   
        esp_lcd_touch_set_mirror_x(tp, false);
        esp_lcd_touch_set_mirror_y(tp, true);
        break;
    case 3:
        esp_lcd_touch_set_swap_xy(tp, true);   
        esp_lcd_touch_set_mirror_x(tp, true);
        esp_lcd_touch_set_mirror_y(tp, true);
        break;
    }
    #else
    ESP_LOGE(TAG, "GSL3680_touch not supported. set_rotation() not called");
    #endif
}