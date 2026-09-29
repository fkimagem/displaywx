#include "../displayfk.h"
#include <Arduino.h>

#if defined(USE_CAMERA_CSI)
#include "../touch/share_i2c.h"
#else
#include <Wire.h>
#endif

namespace {

constexpr uint8_t kPanelAddress = 0x45;
constexpr uint8_t kBacklightRegister = 0x96;
constexpr uint32_t kStepDelayMs = 100;

struct RegisterWrite {
    uint8_t reg;
    uint8_t value;
};

constexpr RegisterWrite kPowerSequence[] = {
    {0x95, 0x11},
    {0x95, 0x17},
    {0x96, 0x00},
    {0x96, 0xFF},
};

bool openSharedBus(int sda, int scl)
{
#if defined(USE_CAMERA_CSI)
    return TouchI2c.begin(sda, scl);
#else
    Wire.begin(sda, scl);
    return true;
#endif
}

bool writeRegister(uint8_t reg, uint8_t value)
{
#if defined(USE_CAMERA_CSI)
    TouchI2c.beginTransmission(kPanelAddress);
    TouchI2c.write(reg);
    TouchI2c.write(value);
    return TouchI2c.endTransmission() == 0;
#else
    Wire.beginTransmission(kPanelAddress);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
#endif
}

bool applyPowerSequence()
{
    size_t index = 0;
    while (index < (sizeof(kPowerSequence) / sizeof(kPowerSequence[0]))) {
        if (!writeRegister(kPowerSequence[index].reg, kPowerSequence[index].value)) {
            return false;
        }
        delay(kStepDelayMs);
        index++;
    }
    return true;
}

}  // namespace

bool DisplayFK::waveshareDsiAuxBacklight(uint8_t brightness)
{
    return writeRegister(kBacklightRegister, brightness);
}

bool DisplayFK::waveshareDsiAuxBegin(int sda, int scl)
{
    if (!openSharedBus(sda, scl)) {
        return false;
    }
    if (!applyPowerSequence()) {
        return false;
    }
    return waveshareDsiAuxBacklight(255);
}
