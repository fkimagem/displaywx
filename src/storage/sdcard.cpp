#include "sdcard.h"

#if defined(DFK_SD)

#include "../displaywx.h"

#include <esp32-hal-gpio.h>
#include <esp_log.h>

#include <string.h>

namespace displaywx {
namespace storage {
namespace {

const char *kTag = "SdCard";

bool pinInShiftRange(int pin)
{
    return pin >= 0 && pin <= 63;
}

bool pinCanOutput(int pin)
{
    return pinInShiftRange(pin) && digitalPinCanOutput(pin);
}

bool pinCanInput(int pin)
{
    return pinInShiftRange(pin) && digitalPinIsValid(pin);
}

bool readExact(fs::File &file, uint8_t *buffer, size_t expected, size_t &filled)
{
    filled = 0;
    while (filled < expected)
    {
        const size_t got = file.read(buffer + filled, expected - filled);
        if (got == 0)
        {
            return false;
        }
        filled += got;
    }
    return true;
}

bool writeExact(fs::File &file, const uint8_t *data, size_t length)
{
    size_t written = 0;
    while (written < length)
    {
        const size_t got = file.write(data + written, length - written);
        if (got == 0)
        {
            return false;
        }
        written += got;
    }
    return true;
}

bool rejectDirectory(fs::FS &filesystem, const char *path, const char *operation)
{
    fs::File existing = filesystem.open(path, FILE_READ);
    if (!existing)
    {
        return false;
    }
    const bool directory = existing.isDirectory();
    existing.close();
    if (directory)
    {
        ESP_LOGE(kTag, "%s rejected directory %s", operation, path);
    }
    return directory;
}

bool joinChildPath(const char *parent, const char *name, char *out, size_t outSize)
{
    if (parent == nullptr || name == nullptr || out == nullptr || outSize == 0)
    {
        return false;
    }
    if (name[0] == '/')
    {
        const size_t length = strlen(name);
        if (length + 1 > outSize)
        {
            return false;
        }
        memcpy(out, name, length + 1);
        return true;
    }

    const size_t parentLength = strlen(parent);
    const bool parentHasSlash = parentLength > 0 && parent[parentLength - 1] == '/';
    const size_t nameLength = strlen(name);
    const size_t needed = parentLength + (parentHasSlash ? 0u : 1u) + nameLength + 1u;
    if (needed > outSize)
    {
        return false;
    }
    memcpy(out, parent, parentLength);
    size_t cursor = parentLength;
    if (!parentHasSlash)
    {
        out[cursor++] = '/';
    }
    memcpy(out + cursor, name, nameLength + 1);
    return true;
}

} // namespace

SdGuard::SdGuard(uint32_t timeoutMs) : m_locked(SdSession::shared().lock(timeoutMs)) {}

SdGuard::~SdGuard()
{
    if (m_locked)
    {
        SdSession::shared().unlock();
    }
}

bool SdGuard::ownsLock() const
{
    return m_locked;
}

SdSession &SdSession::shared()
{
    static SdSession session;
    return session;
}

SdSession::SdSession()
    : m_mutex(xSemaphoreCreateMutex()), m_owner(nullptr), m_config(), m_mounted(false), m_busPrepared(false), m_preparedSpi(nullptr), m_preparedSck(-1), m_preparedMiso(-1), m_preparedMosi(-1), m_preparedCs(-1)
{
    if (m_mutex == nullptr)
    {
        ESP_LOGE(kTag, "SD mutex creation failed");
    }
}

bool SdSession::lock(uint32_t timeoutMs)
{
    if (m_mutex == nullptr)
    {
        ESP_LOGE(kTag, "SD mutex unavailable");
        return false;
    }
    const TickType_t ticks = (timeoutMs == SdGuard::kWaitForever) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);
    return xSemaphoreTake(m_mutex, ticks) == pdPASS;
}

void SdSession::unlock()
{
    xSemaphoreGive(m_mutex);
}

bool SdSession::pinsAcceptable(const SDConfig &config) const
{
    if (!pinCanOutput(config.cs))
    {
        ESP_LOGE(kTag, "Invalid SD CS pin: %d", config.cs);
        return false;
    }
    if (!config.initializeSPI)
    {
        return true;
    }
    if (!pinCanOutput(config.sck) || !pinCanInput(config.miso) || !pinCanOutput(config.mosi))
    {
        ESP_LOGE(kTag, "Invalid SD SPI pins sck=%d miso=%d mosi=%d", config.sck, config.miso, config.mosi);
        return false;
    }

    const int8_t pins[] = {config.cs, config.sck, config.miso, config.mosi};
    for (size_t first = 0; first < 4; ++first)
    {
        for (size_t second = first + 1; second < 4; ++second)
        {
            if (pins[first] == pins[second])
            {
                ESP_LOGE(kTag, "SD pins must be distinct");
                return false;
            }
        }
    }
    return true;
}

bool SdSession::sameConfig(const SDConfig &config) const
{
    return m_config.cs == config.cs && m_config.sck == config.sck && m_config.miso == config.miso && m_config.mosi == config.mosi && m_config.spi == config.spi && m_config.frequencyHz == config.frequencyHz && m_config.initializeSPI == config.initializeSPI;
}

bool SdSession::prepareBus(const SDConfig &config)
{
    SPIClass *spi = config.spi;
    if (config.initializeSPI)
    {
        const bool started = spi->bus() != nullptr;
        const bool preparedHere = m_busPrepared && m_preparedSpi == spi && m_preparedSck == config.sck && m_preparedMiso == config.miso && m_preparedMosi == config.mosi && m_preparedCs == config.cs;
        if (started && !preparedHere)
        {
            ESP_LOGE(kTag, "SPI already started. Refusing to remap; use initializeSPI=false or a free SPIClass.");
            return false;
        }
        if (!started)
        {
            if (!spi->begin(config.sck, config.miso, config.mosi, config.cs))
            {
                ESP_LOGE(kTag, "SPI.begin failed for SD");
                return false;
            }
            m_busPrepared = true;
            m_preparedSpi = spi;
            m_preparedSck = config.sck;
            m_preparedMiso = config.miso;
            m_preparedMosi = config.mosi;
            m_preparedCs = config.cs;
        }
    }
    else if (spi->bus() == nullptr)
    {
        ESP_LOGE(kTag, "Shared SPI is not started");
        return false;
    }

    pinMode(static_cast<uint8_t>(config.cs), OUTPUT);
    digitalWrite(static_cast<uint8_t>(config.cs), HIGH);
    return true;
}

void SdSession::publishCard()
{
    WidgetBase::mySD = &SD;
}

void SdSession::concealCard()
{
    WidgetBase::mySD = nullptr;
}

bool SdSession::mount(const SDConfig &config, const void *owner)
{
    if (config.spi == nullptr)
    {
        ESP_LOGE(kTag, "SD SPI instance is null");
        return false;
    }
    if (!frequencyInPolicy(config.frequencyHz))
    {
        ESP_LOGE(kTag, "SD frequency %u Hz rejected. Allowed range is %u to %u Hz.", static_cast<unsigned>(config.frequencyHz), static_cast<unsigned>(kSdMinFrequencyHz), static_cast<unsigned>(kSdMaxFrequencyHz));
        return false;
    }
    if (!pinsAcceptable(config))
    {
        return false;
    }

    if (m_mounted)
    {
        if (m_owner != owner)
        {
            ESP_LOGE(kTag, "SD session belongs to another DisplayWX");
            return false;
        }
        if (sameConfig(config))
        {
            return true;
        }
        ESP_LOGE(kTag, "SD already mounted with another configuration. Call endSD() before startSD().");
        return false;
    }

    if (SD.cardType() != CARD_NONE)
    {
        ESP_LOGE(kTag, "SD is already mounted outside DisplayWX. Keep a single owner.");
        return false;
    }
    if (!prepareBus(config))
    {
        return false;
    }

    const bool began = SD.begin(static_cast<uint8_t>(config.cs), *config.spi, config.frequencyHz, "/sd", kSdMaxOpenFiles, false);
    if (!began || SD.cardType() == CARD_NONE)
    {
        ESP_LOGE(kTag, "SD mount failed");
        SD.end();
        concealCard();
        m_mounted = false;
        m_owner = nullptr;
        return false;
    }

    m_config = config;
    m_owner = owner;
    m_mounted = true;
    publishCard();
    ESP_LOGI(kTag, "SD card mounted at %u Hz", static_cast<unsigned>(config.frequencyHz));
    return true;
}

bool SdSession::unmount(const void *requester)
{
    if (!m_mounted)
    {
        concealCard();
        m_owner = nullptr;
        m_config = SDConfig{};
        return true;
    }
    if (m_owner != requester)
    {
        ESP_LOGE(kTag, "This DisplayWX does not own the SD session");
        return false;
    }

    concealCard();
    SD.end();
    m_mounted = false;
    m_owner = nullptr;
    m_config = SDConfig{};
    return true;
}

bool SdSession::isMounted() const
{
    return m_mounted;
}

const void *SdSession::owner() const
{
    return m_owner;
}

fs::FS *SdSession::filesystem() const
{
    if (!m_mounted)
    {
        return nullptr;
    }
    return &SD;
}

bool readText(fs::FS &filesystem, const char *path, String &out, size_t maxBytes)
{
    out = "";
    if (!isCardFilePath(path))
    {
        ESP_LOGE(kTag, "readSDFile invalid path");
        return false;
    }
    if (rejectDirectory(filesystem, path, "readSDFile"))
    {
        return false;
    }

    fs::File file = filesystem.open(path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(kTag, "readSDFile failed to open %s", path);
        return false;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "readSDFile path is a directory: %s", path);
        file.close();
        return false;
    }

    const size_t fileSize = file.size();
    if (fileSize > maxBytes)
    {
        ESP_LOGE(kTag, "readSDFile %s exceeds %u bytes", path, static_cast<unsigned>(maxBytes));
        file.close();
        return false;
    }
    if (fileSize == 0)
    {
        file.close();
        return true;
    }
    if (!out.reserve(fileSize + 1u))
    {
        ESP_LOGE(kTag, "readSDFile allocation failed for %s", path);
        file.close();
        return false;
    }

    uint8_t chunk[kSdIoChunkBytes];
    size_t remaining = fileSize;
    while (remaining > 0)
    {
        const size_t want = remaining > sizeof(chunk) ? sizeof(chunk) : remaining;
        size_t filled = 0;
        if (!readExact(file, chunk, want, filled))
        {
            ESP_LOGE(kTag, "readSDFile short read %s", path);
            out = "";
            file.close();
            return false;
        }
        if (!out.concat(chunk, static_cast<unsigned int>(filled)))
        {
            ESP_LOGE(kTag, "readSDFile copy failed for %s", path);
            out = "";
            file.close();
            return false;
        }
        remaining -= filled;
    }

    file.close();
    if (out.length() != fileSize)
    {
        ESP_LOGE(kTag, "readSDFile length mismatch for %s", path);
        out = "";
        return false;
    }
    return true;
}

bool readBinary(fs::FS &filesystem, const char *path, uint8_t *buffer, size_t capacity, size_t &bytesRead, size_t offset)
{
    bytesRead = 0;
    if (!isCardFilePath(path))
    {
        ESP_LOGE(kTag, "readSDFile invalid path");
        return false;
    }
    if (offset > static_cast<size_t>(UINT32_MAX))
    {
        ESP_LOGE(kTag, "readSDFile offset does not fit File::seek");
        return false;
    }
    if (capacity > 0 && buffer == nullptr)
    {
        ESP_LOGE(kTag, "readSDFile buffer is null");
        return false;
    }
    if (rejectDirectory(filesystem, path, "readSDFile"))
    {
        return false;
    }

    fs::File file = filesystem.open(path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(kTag, "readSDFile failed to open %s", path);
        return false;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "readSDFile path is a directory: %s", path);
        file.close();
        return false;
    }

    const size_t fileSize = file.size();
    if (offset > fileSize)
    {
        ESP_LOGE(kTag, "readSDFile offset past end of %s", path);
        file.close();
        return false;
    }

    const size_t available = fileSize - offset;
    const size_t toRead = capacity < available ? capacity : available;
    if (toRead == 0)
    {
        file.close();
        return true;
    }
    if (!file.seek(static_cast<uint32_t>(offset)))
    {
        ESP_LOGE(kTag, "readSDFile seek failed for %s", path);
        file.close();
        return false;
    }

    const bool complete = readExact(file, buffer, toRead, bytesRead);
    file.close();
    if (!complete)
    {
        ESP_LOGE(kTag, "readSDFile short read %s", path);
    }
    return complete;
}

bool writeBytes(fs::FS &filesystem, const char *path, const uint8_t *data, size_t length, bool append)
{
    const char *operation = append ? "appendSDFile" : "writeSDFile";
    if (!isCardFilePath(path))
    {
        ESP_LOGE(kTag, "%s invalid path", operation);
        return false;
    }
    if (length > 0 && data == nullptr)
    {
        ESP_LOGE(kTag, "%s data is null", operation);
        return false;
    }
    if (rejectDirectory(filesystem, path, operation))
    {
        return false;
    }

    fs::File file = filesystem.open(path, append ? FILE_APPEND : FILE_WRITE);
    if (!file)
    {
        ESP_LOGE(kTag, "%s failed to open %s", operation, path);
        return false;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "%s path is a directory: %s", operation, path);
        file.close();
        return false;
    }

    const bool written = writeExact(file, data, length);
    file.flush();
    file.close();
    if (!written)
    {
        ESP_LOGE(kTag, "%s incomplete write %s", operation, path);
    }
    return written;
}

bool existsRegularFile(fs::FS &filesystem, const char *path)
{
    if (!isCardFilePath(path))
    {
        return false;
    }
    fs::File file = filesystem.open(path, FILE_READ);
    if (!file)
    {
        return false;
    }
    const bool regular = !file.isDirectory();
    file.close();
    return regular;
}

bool createOneDirectory(fs::FS &filesystem, const char *path)
{
    if (!isCardDirectoryPath(path))
    {
        ESP_LOGE(kTag, "createSDDir invalid path");
        return false;
    }

    fs::File existing = filesystem.open(path, FILE_READ);
    if (existing)
    {
        const bool directory = existing.isDirectory();
        existing.close();
        if (directory)
        {
            return true;
        }
        ESP_LOGE(kTag, "createSDDir path is a file: %s", path);
        return false;
    }
    if (!filesystem.mkdir(path))
    {
        ESP_LOGE(kTag, "createSDDir failed for %s", path);
        return false;
    }
    return true;
}

void legacyReadToSerial(fs::FS *filesystem, const char *path)
{
    if (filesystem == nullptr || !isCardFilePath(path))
    {
        ESP_LOGE(kTag, "readFile invalid arguments");
        return;
    }
    fs::File file = filesystem->open(path, FILE_READ);
    if (!file)
    {
        ESP_LOGE(kTag, "readFile failed to open %s", path);
        return;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "readFile path is a directory: %s", path);
        file.close();
        return;
    }
    uint8_t chunk[kSdIoChunkBytes];
    while (true)
    {
        const size_t got = file.read(chunk, sizeof(chunk));
        if (got == 0)
        {
            break;
        }
        Serial.write(chunk, got);
    }
    file.close();
}

void legacyAppendLine(fs::FS *filesystem, const char *path, const char *message)
{
    if (filesystem == nullptr || !isCardFilePath(path) || message == nullptr)
    {
        ESP_LOGE(kTag, "appendFile invalid arguments");
        return;
    }

    const size_t length = strlen(message);
    fs::File file = filesystem->open(path, FILE_APPEND);
    if (!file)
    {
        ESP_LOGE(kTag, "appendFile failed to open %s", path);
        return;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "appendFile path is a directory: %s", path);
        file.close();
        return;
    }
    if (file.size() > LOG_MAX_SIZE)
    {
        ESP_LOGE(kTag, "Log too large");
        file.close();
        return;
    }
    if (length <= 1)
    {
        ESP_LOGW(kTag, "Empty message isnt appended");
        file.close();
        return;
    }
    if (!file.println(message))
    {
        ESP_LOGE(kTag, "appendFile failed for %s", path);
    }
    file.flush();
    file.close();
}

void legacyWriteText(fs::FS *filesystem, const char *path, const char *message)
{
    if (filesystem == nullptr || !isCardFilePath(path) || message == nullptr)
    {
        ESP_LOGE(kTag, "writeFile invalid arguments");
        return;
    }
    fs::File file = filesystem->open(path, FILE_WRITE);
    if (!file)
    {
        ESP_LOGE(kTag, "writeFile failed to open %s", path);
        return;
    }
    if (file.isDirectory())
    {
        ESP_LOGE(kTag, "writeFile path is a directory: %s", path);
        file.close();
        return;
    }
    if (!file.print(message))
    {
        ESP_LOGE(kTag, "writeFile failed for %s", path);
    }
    file.flush();
    file.close();
}

void legacyCreateDir(fs::FS *filesystem, const char *path)
{
    if (filesystem == nullptr || !isCardDirectoryPath(path))
    {
        ESP_LOGE(kTag, "createDir invalid arguments");
        return;
    }
    if (!filesystem->mkdir(path))
    {
        ESP_LOGE(kTag, "createDir failed for %s", path);
    }
}

void legacyList(fs::FS *filesystem, const char *dirname, uint8_t levels)
{
    if (filesystem == nullptr || !isCardDirectoryPath(dirname))
    {
        ESP_LOGE(kTag, "listFiles invalid arguments");
        return;
    }

    fs::File root = filesystem->open(dirname, FILE_READ);
    if (!root)
    {
        ESP_LOGE(kTag, "listFiles failed to open %s", dirname);
        return;
    }
    if (!root.isDirectory())
    {
        ESP_LOGE(kTag, "listFiles path is not a directory: %s", dirname);
        root.close();
        return;
    }

    fs::File file = root.openNextFile();
    uint16_t fileIndex = 1;
    while (file)
    {
        if (file.isDirectory())
        {
            ESP_LOGD(kTag, "DIR: %s", file.name());
            if (levels > 0)
            {
                char child[256];
                if (!joinChildPath(dirname, file.name(), child, sizeof(child)))
                {
                    ESP_LOGE(kTag, "listFiles skipped child of %s", dirname);
                }
                else
                {
                    file.close();
                    legacyList(filesystem, child, static_cast<uint8_t>(levels - 1));
                    file = root.openNextFile();
                    continue;
                }
            }
        }
        else
        {
            ESP_LOGD(kTag, "%u FILE: %s SIZE: %u", fileIndex, file.name(), static_cast<unsigned>(file.size()));
            ++fileIndex;
        }
        file.close();
        file = root.openNextFile();
    }
    root.close();
}

} // namespace storage
} // namespace displaywx

#endif
