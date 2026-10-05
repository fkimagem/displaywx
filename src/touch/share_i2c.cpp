#include "share_i2c.h"

#if defined(HAS_CAMERA_MIPI)
#include "esp_log.h"
#include "esp32-hal-periman.h"
#include "soc/soc_caps.h"

static const char *TAG = "TouchI2C";
static const uint32_t kTimeoutMs = 50;
static const size_t kBufferSize = 128;

TouchI2cMaster::TouchI2cMaster()
{
    busMutex = xSemaphoreCreateMutexStatic(&busMutexBuffer);
}

bool TouchI2cMaster::lock()
{
    if (busMutex == nullptr) {
        return false;
    }
    if (xSemaphoreGetMutexHolder(busMutex) == xTaskGetCurrentTaskHandle()) {
        return true;
    }
    if (xSemaphoreTake(busMutex, pdMS_TO_TICKS(kTimeoutMs)) == pdTRUE) {
        return true;
    }
    ESP_LOGW(TAG, "I2C lock timed out");
    return false;
}

void TouchI2cMaster::unlock()
{
    if (busMutex == nullptr) {
        return;
    }
    if (xSemaphoreGetMutexHolder(busMutex) != xTaskGetCurrentTaskHandle()) {
        return;
    }
    xSemaphoreGive(busMutex);
}

bool TouchI2cMaster::usesPins(int8_t sda, int8_t scl) const
{
    return sdaPin >= 0 && sdaPin == sda && sclPin == scl;
}

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
    ESP_LOGE(TAG, "No free HP I2C port for touch");
    return false;
}

bool TouchI2cMaster::acquireBus(int8_t sda, int8_t scl)
{
    bool busReady = tryReuseBusOnPins(sda, scl, &busHandle);
    if (busReady) {
        return true;
    }

    busReady = createBusOnFreePort(sda, scl, &busHandle);
    if (busReady) {
        return true;
    }

    ESP_LOGE(TAG, "Touch I2C pins are busy; cannot share or create bus");
    return false;
}

bool TouchI2cMaster::begin()
{
    return begin(SDA, SCL, 0);
}

bool TouchI2cMaster::begin(int sda, int scl, uint32_t frequency)
{
    if (!lock()) {
        return false;
    }
    if (busHandle != nullptr) {
        unlock();
        return true;
    }
    if (frequency > 0) {
        sclSpeedHz = frequency;
    }
    const bool ready = acquireBus((int8_t)sda, (int8_t)scl);
    if (ready) {
        sdaPin = (int8_t)sda;
        sclPin = (int8_t)scl;
    }
    unlock();
    return ready;
}

i2c_master_dev_handle_t TouchI2cMaster::deviceFor(uint8_t address)
{
    if (busHandle == nullptr || address >= 128) {
        return nullptr;
    }

    for (size_t index = 0; index < kDeviceSlots; index++) {
        if (deviceSlots[index].used && deviceSlots[index].address == address) {
            return deviceSlots[index].handle;
        }
    }

    size_t slot = kDeviceSlots;
    for (size_t index = 0; index < kDeviceSlots; index++) {
        if (!deviceSlots[index].used) {
            slot = index;
            break;
        }
    }
    if (slot == kDeviceSlots) {
        slot = deviceEvict % kDeviceSlots;
        deviceEvict++;
        if (deviceSlots[slot].handle != nullptr) {
            i2c_master_bus_rm_device(deviceSlots[slot].handle);
            deviceSlots[slot].handle = nullptr;
            deviceSlots[slot].used = false;
        }
    }

    i2c_device_config_t config = {};
    config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    config.device_address = address;
    config.scl_speed_hz = sclSpeedHz;

    esp_err_t err = i2c_master_bus_add_device(busHandle, &config, &deviceSlots[slot].handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_master_bus_add_device failed: %s", esp_err_to_name(err));
        deviceSlots[slot].handle = nullptr;
        deviceSlots[slot].used = false;
        return nullptr;
    }
    deviceSlots[slot].address = address;
    deviceSlots[slot].used = true;
    return deviceSlots[slot].handle;
}

uint8_t TouchI2cMaster::statusFromEsp(esp_err_t err)
{
    if (err == ESP_OK) {
        return 0;
    }
    if (err == ESP_ERR_TIMEOUT) {
        return 5;
    }
    if (err == ESP_FAIL) {
        return 2;
    }
    if (err == ESP_ERR_NOT_FOUND) {
        return 2;
    }
    return 4;
}

void TouchI2cMaster::beginTransmission(uint8_t address)
{
    repeatedStart = false;
    txAddress = address;
    txLength = 0;
    if (holdingBus) {
        return;
    }
    holdingBus = lock();
}

size_t TouchI2cMaster::write(uint8_t data)
{
    if (txLength >= kBufferSize) {
        return 0;
    }
    txBuffer[txLength++] = data;
    return 1;
}

size_t TouchI2cMaster::write(const uint8_t *data, size_t quantity)
{
    size_t written = 0;
    while (written < quantity) {
        if (write(data[written]) == 0) {
            return written;
        }
        written++;
    }
    return written;
}

uint8_t TouchI2cMaster::endTransmission(bool sendStop)
{
    if (!holdingBus) {
        return 5;
    }
    if (!sendStop) {
        repeatedStart = true;
        return 0;
    }

    i2c_master_dev_handle_t device = deviceFor(txAddress);
    esp_err_t err = ESP_OK;
    if (device == nullptr) {
        err = ESP_FAIL;
    } else if (txLength == 0) {
        err = i2c_master_probe(busHandle, txAddress, kTimeoutMs);
    } else {
        err = i2c_master_transmit(device, txBuffer, txLength, kTimeoutMs);
    }

    holdingBus = false;
    repeatedStart = false;
    unlock();
    return statusFromEsp(err);
}

uint8_t TouchI2cMaster::endTransmission()
{
    return endTransmission(true);
}

size_t TouchI2cMaster::requestFrom(uint8_t address, size_t len, bool sendStop)
{
    (void)sendStop;
    if (len > kBufferSize) {
        len = kBufferSize;
    }
    if (!holdingBus && !lock()) {
        return 0;
    }
    holdingBus = true;

    rxIndex = 0;
    rxLength = 0;

    i2c_master_dev_handle_t device = deviceFor(address);
    esp_err_t err = ESP_OK;
    if (device == nullptr) {
        err = ESP_FAIL;
    } else if (repeatedStart) {
        repeatedStart = false;
        err = i2c_master_transmit_receive(device, txBuffer, txLength, rxBuffer, len, kTimeoutMs);
    } else {
        err = i2c_master_receive(device, rxBuffer, len, kTimeoutMs);
    }

    holdingBus = false;
    unlock();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
        return 0;
    }

    rxLength = len;
    return rxLength;
}

size_t TouchI2cMaster::requestFrom(uint8_t address, size_t len)
{
    return requestFrom(address, len, true);
}

int TouchI2cMaster::available()
{
    return (int)(rxLength - rxIndex);
}

int TouchI2cMaster::read()
{
    if (rxIndex >= rxLength) {
        return -1;
    }
    return rxBuffer[rxIndex++];
}

TouchI2cMaster TouchI2c;
#endif
