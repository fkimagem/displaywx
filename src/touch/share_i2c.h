#ifndef DISPLAYFK_TOUCH_SHARE_I2C_H
#define DISPLAYFK_TOUCH_SHARE_I2C_H

#include "../../user_setup.h"
#include "../check_camera.h"

#if defined(HAS_CAMERA_MIPI)
#include <Arduino.h>
#include "driver/i2c_master.h"
#include <freertos/semphr.h>

class TouchI2cMaster
{
public:
    TouchI2cMaster();
    bool begin();
    bool begin(int sda, int scl, uint32_t frequency = 0);
    void beginTransmission(uint8_t address);
    size_t write(uint8_t data);
    size_t write(const uint8_t *data, size_t quantity);
    uint8_t endTransmission(bool sendStop);
    uint8_t endTransmission();
    size_t requestFrom(uint8_t address, size_t len, bool sendStop);
    size_t requestFrom(uint8_t address, size_t len);
    int available();
    int read();
    bool lock();
    void unlock();
    bool usesPins(int8_t sda, int8_t scl) const;

private:
    struct DeviceSlot {
        uint8_t address;
        bool used;
        i2c_master_dev_handle_t handle;
    };

    bool acquireBus(int8_t sda, int8_t scl);
    i2c_master_dev_handle_t deviceFor(uint8_t address);
    uint8_t statusFromEsp(esp_err_t err);

    static constexpr size_t kDeviceSlots = 4;

    i2c_master_bus_handle_t busHandle = nullptr;
    DeviceSlot deviceSlots[kDeviceSlots] = {};
    size_t deviceEvict = 0;
    uint8_t txAddress = 0;
    uint8_t txBuffer[128];
    size_t txLength = 0;
    bool repeatedStart = false;
    bool holdingBus = false;
    uint8_t rxBuffer[128];
    size_t rxIndex = 0;
    size_t rxLength = 0;
    uint32_t sclSpeedHz = 400000;
    int8_t sdaPin = -1;
    int8_t sclPin = -1;
    SemaphoreHandle_t busMutex = nullptr;
    StaticSemaphore_t busMutexBuffer = {};
};

extern TouchI2cMaster TouchI2c;
#endif

#endif
