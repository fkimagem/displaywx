#ifndef DISPLAYFK_GT911_WAVESHARE_H
#define DISPLAYFK_GT911_WAVESHARE_H

#include <Arduino.h>

struct Gt911WavesharePoint {
    uint16_t x;
    uint16_t y;
    uint16_t size;
};

class Gt911Waveshare {
public:
    bool begin(int sda, int scl, uint16_t panelWidth, uint16_t panelHeight);
    void read();

    bool isTouched = false;
    uint8_t touches = 0;
    Gt911WavesharePoint points[5] = {};

private:
    bool probe(uint8_t candidate);
    bool readRegister(uint16_t reg, uint8_t *data, uint8_t length);
    bool writeRegister(uint16_t reg, uint8_t value);
    bool confirmIdentity();
    bool readBlock(uint16_t reg, uint8_t *data, uint16_t length);
    bool writeBlock(uint16_t reg, const uint8_t *data, uint8_t length);
    void prepareConfig(uint16_t panelWidth, uint16_t panelHeight);
    void clearPoints();

    uint8_t address = 0;
    bool ready = false;
};

#endif
