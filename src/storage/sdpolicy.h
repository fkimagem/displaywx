#pragma once

#include <stddef.h>
#include <stdint.h>

namespace displaywx {
namespace storage {

constexpr uint32_t kSdMinFrequencyHz = 100000u;
constexpr uint32_t kSdMaxFrequencyHz = 20000000u;
constexpr uint32_t kSdDefaultFrequencyHz = 4000000u;
constexpr uint32_t kSdLockTimeoutMs = 1000u;
constexpr uint8_t kSdMaxOpenFiles = 5;
constexpr size_t kSdIoChunkBytes = 512u;

inline bool frequencyInPolicy(uint32_t frequencyHz)
{
    return frequencyHz >= kSdMinFrequencyHz && frequencyHz <= kSdMaxFrequencyHz;
}

inline bool isCardFilePath(const char *path)
{
    if (path == nullptr || path[0] != '/' || path[1] == '\0')
    {
        return false;
    }
    const char *cursor = path;
    while (*cursor != '\0')
    {
        ++cursor;
    }
    return cursor[-1] != '/';
}

inline bool isCardDirectoryPath(const char *path)
{
    return path != nullptr && path[0] == '/';
}

} // namespace storage
} // namespace displaywx
