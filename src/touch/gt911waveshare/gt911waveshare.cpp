#include "gt911waveshare.h"
#include "../../../user_setup.h"

#if defined(TOUCH_GT911_WAVESHARE)

#include <Wire.h>
#include "../share_i2c.h"

#if defined(USE_CAMERA_CSI)
#define Wire TouchI2c
#endif

namespace {
constexpr uint8_t kAddressPrimary = 0x5D;
constexpr uint8_t kAddressBackup = 0x14;
constexpr uint16_t kProductIdRegister = 0x8140;
constexpr uint16_t kCommandRegister = 0x8040;
constexpr uint16_t kConfigRegister = 0x8047;
constexpr uint16_t kConfigFreshRegister = 0x8100;
constexpr uint16_t kPointStatusRegister = 0x814E;
constexpr uint16_t kConfigBytes = 184;
constexpr uint8_t kConfigChecksumIndex = 184;
constexpr uint8_t kMaxPoints = 5;
constexpr uint8_t kChunkSize = 64;
constexpr uint32_t kProbeDelayMs = 200;
constexpr uint32_t kConfigApplyDelayMs = 100;
}

bool Gt911Waveshare::begin(int sda, int scl, uint16_t panelWidth, uint16_t panelHeight) {
    ready = false;
    address = 0;
    clearPoints();

#if defined(USE_CAMERA_CSI)
    if (!Wire.begin(sda, scl)) {
        return false;
    }
#else
    Wire.begin(sda, scl);
#endif

    delay(kProbeDelayMs);
    if (probe(kAddressPrimary)) {
        address = kAddressPrimary;
    } else if (probe(kAddressBackup)) {
        address = kAddressBackup;
    } else {
        Serial.println("GT911 Waveshare nao encontrado em 0x5D nem 0x14");
        return false;
    }

    if (!confirmIdentity()) {
        address = 0;
        return false;
    }

    prepareConfig(panelWidth, panelHeight);
    writeRegister(kCommandRegister, 0x00);
    writeRegister(kPointStatusRegister, 0);
    ready = true;
    return true;
}

void Gt911Waveshare::read() {
    clearPoints();
    if (!ready) {
        return;
    }

    uint8_t buffer[1 + (kMaxPoints * 8)] = {};
    if (!readRegister(kPointStatusRegister, buffer, 1)) {
        return;
    }

    const uint8_t status = buffer[0];
    if ((status & 0x80) == 0) {
        return;
    }

    const uint8_t count = status & 0x0F;
    if (count == 0 || count > kMaxPoints) {
        writeRegister(kPointStatusRegister, 0);
        return;
    }

    if (!readRegister(kPointStatusRegister + 1, &buffer[1], count * 8)) {
        writeRegister(kPointStatusRegister, 0);
        return;
    }
    writeRegister(kPointStatusRegister, 0);

    touches = count;
    isTouched = true;
    for (uint8_t index = 0; index < count; index++) {
        const uint8_t offset = index * 8;
        points[index].x = (uint16_t)((buffer[offset + 3] << 8) | buffer[offset + 2]);
        points[index].y = (uint16_t)((buffer[offset + 5] << 8) | buffer[offset + 4]);
        points[index].size = (uint16_t)((buffer[offset + 7] << 8) | buffer[offset + 6]);
    }
}

bool Gt911Waveshare::probe(uint8_t candidate) {
    Wire.beginTransmission(candidate);
    return Wire.endTransmission() == 0;
}

bool Gt911Waveshare::readRegister(uint16_t reg, uint8_t *data, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(highByte(reg));
    Wire.write(lowByte(reg));
    if (Wire.endTransmission(false) != 0) {
        return false;
    }
    if (Wire.requestFrom(address, (uint8_t)length) != length) {
        return false;
    }
    for (uint8_t index = 0; index < length; index++) {
        const int value = Wire.read();
        if (value < 0) {
            return false;
        }
        data[index] = (uint8_t)value;
    }
    return true;
}

bool Gt911Waveshare::writeRegister(uint16_t reg, uint8_t value) {
    Wire.beginTransmission(address);
    Wire.write(highByte(reg));
    Wire.write(lowByte(reg));
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool Gt911Waveshare::confirmIdentity() {
    uint8_t productId[4] = {};
    if (!readRegister(kProductIdRegister, productId, sizeof(productId))) {
        return false;
    }

    Serial.printf(
        "GT911 Waveshare em 0x%02X, id %02X %02X %02X %02X\n",
        address,
        productId[0],
        productId[1],
        productId[2],
        productId[3]);
    return true;
}

bool Gt911Waveshare::readBlock(uint16_t reg, uint8_t *data, uint16_t length) {
    uint16_t offset = 0;
    while (offset < length) {
        const uint8_t chunk = (length - offset) > kChunkSize ? kChunkSize : (uint8_t)(length - offset);
        if (!readRegister(reg + offset, data + offset, chunk)) {
            return false;
        }
        offset += chunk;
    }
    return true;
}

bool Gt911Waveshare::writeBlock(uint16_t reg, const uint8_t *data, uint8_t length) {
    Wire.beginTransmission(address);
    Wire.write(highByte(reg));
    Wire.write(lowByte(reg));
    for (uint8_t index = 0; index < length; index++) {
        Wire.write(data[index]);
    }
    return Wire.endTransmission() == 0;
}

void Gt911Waveshare::prepareConfig(uint16_t panelWidth, uint16_t panelHeight) {
    uint8_t config[kConfigBytes + 1] = {};
    if (!readBlock(kConfigRegister, config, kConfigBytes + 1)) {
        Serial.println("GT911 Waveshare: falha ao ler a configuracao");
        return;
    }

    const uint16_t storedWidth = (uint16_t)(config[1] | (config[2] << 8));
    const uint16_t storedHeight = (uint16_t)(config[3] | (config[4] << 8));
    Serial.printf(
        "GT911 Waveshare config %u, resolucao %ux%u, toques %u\n",
        config[0],
        storedWidth,
        storedHeight,
        config[5]);

    if (storedWidth != 0 && storedHeight != 0) {
        return;
    }
    if (panelWidth == 0 || panelHeight == 0) {
        return;
    }

    config[1] = (uint8_t)(panelWidth & 0xFF);
    config[2] = (uint8_t)(panelWidth >> 8);
    config[3] = (uint8_t)(panelHeight & 0xFF);
    config[4] = (uint8_t)(panelHeight >> 8);
    if (config[5] == 0) {
        config[5] = 1;
    }

    uint8_t sum = 0;
    for (uint16_t index = 0; index < kConfigBytes; index++) {
        sum += config[index];
    }
    config[kConfigChecksumIndex] = (uint8_t)((~sum) + 1);

    uint16_t offset = 0;
    while (offset < kConfigBytes + 1) {
        const uint8_t chunk = (kConfigBytes + 1 - offset) > kChunkSize ? kChunkSize : (uint8_t)(kConfigBytes + 1 - offset);
        if (!writeBlock(kConfigRegister + offset, config + offset, chunk)) {
            Serial.println("GT911 Waveshare: falha ao gravar a configuracao");
            return;
        }
        offset += chunk;
    }
    writeRegister(kConfigFreshRegister, 1);
    delay(kConfigApplyDelayMs);
    Serial.printf("GT911 Waveshare config atualizada para %ux%u\n", panelWidth, panelHeight);
}

void Gt911Waveshare::clearPoints() {
    isTouched = false;
    touches = 0;
    for (uint8_t index = 0; index < kMaxPoints; index++) {
        points[index] = {};
    }
}

#endif
