#include "gsl3680_touch.h"
#include "esp_lcd_gsl3680.h"
#include "../../share_i2c.h"
#include "../../../extras/check_version.h"
#include <Arduino.h>

GSL3680_touch::GSL3680_touch(int8_t sda, int8_t scl, int8_t rst, int8_t interrupt)
    : _sda(sda), _scl(scl), _rst(rst), _int(interrupt) {}

GSL3680_touch::~GSL3680_touch() { end(); }

bool GSL3680_touch::begin()
{
#if defined(TOUCH_GSL3680)
    if (tp) return true;
    if (!TouchI2c.lock()) return false;
    m_createdBus = TouchI2c.handle() == nullptr;
    if (!TouchI2c.begin(_sda, _scl, 400000)) {
        TouchI2c.unlock();
        m_createdBus = false;
        return false;
    }
    esp_lcd_panel_io_i2c_config_t io = ESP_LCD_TOUCH_IO_I2C_GSL3680_CONFIG();
    io.scl_speed_hz = 400000;
    esp_err_t result = esp_lcd_new_panel_io_i2c(TouchI2c.handle(), &io, &tp_io_handle);
    if (result == ESP_OK) {
        esp_lcd_touch_config_t config = {};
        config.x_max = 800;
        config.y_max = 1280;
        config.rst_gpio_num = static_cast<gpio_num_t>(_rst);
        config.int_gpio_num = static_cast<gpio_num_t>(_int);
        config.flags.mirror_y = 1;
        result = esp_lcd_touch_new_i2c_gsl3680(tp_io_handle, &config, &tp);
    }
    if (result != ESP_OK) end();
    TouchI2c.unlock();
    return result == ESP_OK;
#else
    return false;
#endif
}

void GSL3680_touch::end()
{
#if defined(TOUCH_GSL3680)
    // Driver lifetime is serialized with touch/camera transactions.
    if (!TouchI2c.lock(UINT32_MAX)) return;
    if (tp) { esp_lcd_touch_del(tp); tp = nullptr; }
    if (tp_io_handle) { esp_lcd_panel_io_del(tp_io_handle); tp_io_handle = nullptr; }
#if !defined(HAS_CAMERA_MIPI)
    if (m_createdBus) TouchI2c.end();
#endif
    // With a camera, DisplayWX releases the shared bus after both users stop.
    m_createdBus = false;
    TouchI2c.unlock();
#endif
}

bool GSL3680_touch::getTouch(uint16_t *x, uint16_t *y)
{
#if defined(TOUCH_GSL3680)
    if (!tp || !x || !y || !TouchI2c.lock()) return false;
    uint16_t strength = 0;
    uint8_t count = 0;
    const bool touched = esp_lcd_touch_read_data(tp) == ESP_OK &&
        esp_lcd_touch_get_coordinates(tp, x, y, &strength, &count, 1);
    TouchI2c.unlock();
    return touched;
#else
    return false;
#endif
}

void GSL3680_touch::set_rotation(uint8_t rotation)
{
#if defined(TOUCH_GSL3680)
    if (!TouchI2c.lock()) return;
    if (!tp) { TouchI2c.unlock(); return; }
    rotation &= 3;
    esp_lcd_touch_set_swap_xy(tp, (rotation & 1) != 0);
    esp_lcd_touch_set_mirror_x(tp, rotation == 0 || rotation == 3);
    esp_lcd_touch_set_mirror_y(tp, rotation >= 2);
    TouchI2c.unlock();
#else
    (void)rotation;
#endif
}
