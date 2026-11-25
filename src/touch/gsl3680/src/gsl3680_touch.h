#ifndef _GSL3680_TOUCH_H
#define _GSL3680_TOUCH_H


#include <stdio.h>
#include <stdint.h>
#include "esp_lcd_touch.h"

class GSL3680_touch
{
public:
    GSL3680_touch(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin = -1, int8_t int_pin = -1);

    ~GSL3680_touch();
    GSL3680_touch(const GSL3680_touch&) = delete;
    GSL3680_touch& operator=(const GSL3680_touch&) = delete;
    bool begin();
    void end();
    bool getTouch(uint16_t *x, uint16_t *y);
    void set_rotation(uint8_t r);

private:
    int8_t _sda, _scl, _rst, _int;
    esp_lcd_touch_handle_t tp = nullptr;
    esp_lcd_panel_io_handle_t tp_io_handle = nullptr;
    bool m_createdBus = false;
};

#endif
