#pragma once

#include "../widgets/widgetsetup.h"

#if defined(DFK_SD)

#include "sdpolicy.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/// Configuração de montagem do cartão. Os pinos pertencem ao sketch.
/// Com initializeSPI verdadeiro, a instância SPI ainda não pode ter sido iniciada:
/// SPI.begin() em um barramento ativo ignora os pinos novos. A DisplayWX não chama SPI.end().
struct SDConfig {
    int8_t cs = -1;
    int8_t sck = -1;
    int8_t miso = -1;
    int8_t mosi = -1;
    SPIClass *spi = &SPI;
    uint32_t frequencyHz = displaywx::storage::kSdDefaultFrequencyHz;
    bool initializeSPI = true;
};

namespace displaywx {
namespace storage {

class SdGuard {
public:
    static constexpr uint32_t kWaitForever = 0xFFFFFFFFu;

    explicit SdGuard(uint32_t timeoutMs = kSdLockTimeoutMs);
    ~SdGuard();

    SdGuard(const SdGuard &) = delete;
    SdGuard &operator=(const SdGuard &) = delete;

    bool ownsLock() const;

private:
    bool m_locked;
};

class SdSession {
public:
    static SdSession &shared();

    bool mount(const SDConfig &config, const void *owner);
    bool unmount(const void *requester);
    bool isMounted() const;
    const void *owner() const;
    fs::FS *filesystem() const;

private:
    friend class SdGuard;

    SdSession();
    bool lock(uint32_t timeoutMs);
    void unlock();
    bool pinsAcceptable(const SDConfig &config) const;
    bool sameConfig(const SDConfig &config) const;
    bool prepareBus(const SDConfig &config);
    void publishCard();
    void concealCard();

    SemaphoreHandle_t m_mutex;
    const void *m_owner;
    SDConfig m_config;
    bool m_mounted;
    bool m_busPrepared;
    SPIClass *m_preparedSpi;
    int8_t m_preparedSck;
    int8_t m_preparedMiso;
    int8_t m_preparedMosi;
    int8_t m_preparedCs;
};

bool readText(fs::FS &filesystem, const char *path, String &out, size_t maxBytes);
bool readBinary(fs::FS &filesystem, const char *path, uint8_t *buffer, size_t capacity, size_t &bytesRead, size_t offset);
bool writeBytes(fs::FS &filesystem, const char *path, const uint8_t *data, size_t length, bool append);
bool existsRegularFile(fs::FS &filesystem, const char *path);
bool createOneDirectory(fs::FS &filesystem, const char *path);

void legacyReadToSerial(fs::FS *filesystem, const char *path);
void legacyAppendLine(fs::FS *filesystem, const char *path, const char *message);
void legacyWriteText(fs::FS *filesystem, const char *path, const char *message);
void legacyCreateDir(fs::FS *filesystem, const char *path);
void legacyList(fs::FS *filesystem, const char *dirname, uint8_t levels);

} // namespace storage
} // namespace displaywx

#endif
