#include "cst820.h"
#include "../share_i2c.h"
#if defined(HAS_CAMERA_MIPI)
#define Wire TouchI2c
#endif

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
{
    _sda = sda_pin;
    _scl = scl_pin;
    _rst = rst_pin;
    _int = int_pin;
}

void CST820::begin(void)
{
    // Initialize I2C
    bool started = false;
    if (_sda != -1 && _scl != -1)
    {
        started = Wire.begin(_sda, _scl);
    }
    else
    {
        started = Wire.begin();
    }
#if defined(HAS_CAMERA_MIPI)
    if (!started) {
        return;
    }
#else
    (void)started;
#endif

    // Int Pin Configuration
    if (_int != -1)
    {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH); //高电平
        delay(1);
        digitalWrite(_int, LOW); //低电平
        delay(1);
    }

    // Reset Pin Configuration
    if (_rst != -1)
    {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300);
    }

    // Initialize Touch
    i2c_write(0xFE, 0XFF); //禁止自动进入低功耗模式。
}

bool CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture)
{
    int finger = i2c_read(0x02);
    if (finger < 0) {
        return false;
    }

    int gestureRaw = i2c_read(0x01);
    *gesture = None;
    if (gestureRaw == SlideUp || gestureRaw == SlideDown) {
        *gesture = (uint8_t)gestureRaw;
    }

    uint8_t data[4] = {};
    if (!i2c_read_continuous(0x03, data, 4)) {
        return false;
    }
    *x = ((data[0] & 0x0f) << 8) | data[1];
    *y = ((data[2] & 0x0f) << 8) | data[3];
    return finger != 0;
}

int CST820::i2c_read(uint8_t addr)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    if (Wire.endTransmission(false) != 0) {
        return -1;
    }
    if (Wire.requestFrom(I2C_ADDR_CST820, 1) != 1) {
        return -1;
    }
    int value = Wire.read();
    if (value < 0) {
        return -1;
    }
    return value;
}

bool CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    if (Wire.endTransmission(true) != 0) {
        return false;
    }
    if (Wire.requestFrom(I2C_ADDR_CST820, length) != length) {
        return false;
    }
    for (uint32_t i = 0; i < length; i++) {
        int value = Wire.read();
        if (value < 0) {
            return false;
        }
        data[i] = (uint8_t)value;
    }
    return true;
}

void CST820::i2c_write(uint8_t addr, uint8_t data)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length)
{
  Wire.beginTransmission(I2C_ADDR_CST820);
  Wire.write(addr);
  for (int i = 0; i < length; i++) {
    Wire.write(*data++);
  }
  if ( Wire.endTransmission(true))return -1;
  return 0;
}