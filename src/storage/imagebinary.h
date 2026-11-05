#pragma once

#include <stddef.h>
#include <stdint.h>

namespace displaywx {
namespace imagebin {

constexpr uint16_t kMaxAxis = 4096;
constexpr uint32_t kMaxMaskBytes = 65535u;

struct Layout {
    uint16_t width;
    uint16_t height;
    uint32_t pixelBytes;
    uint16_t maskBytes;
    uint32_t totalBytes;
};

inline uint16_t readBe16(const uint8_t *data)
{
    return static_cast<uint16_t>((static_cast<uint16_t>(data[0]) << 8) | data[1]);
}

inline bool multiply(uint32_t left, uint32_t right, uint32_t &product)
{
    if (left != 0 && right > UINT32_MAX / left)
    {
        return false;
    }
    product = left * right;
    return true;
}

inline bool computeLayout(uint16_t width, uint16_t height, uint8_t bytesPerPixel, uint32_t fileSize, Layout &layout)
{
    if (width == 0 || height == 0 || bytesPerPixel == 0)
    {
        return false;
    }
    if (width > kMaxAxis || height > kMaxAxis)
    {
        return false;
    }

    uint32_t pixelCount = 0;
    uint32_t pixelBytes = 0;
    uint32_t maskBytes = 0;
    const uint32_t rowBytes = (static_cast<uint32_t>(width) + 7u) / 8u;
    if (!multiply(width, height, pixelCount) || !multiply(pixelCount, bytesPerPixel, pixelBytes))
    {
        return false;
    }
    if (!multiply(rowBytes, height, maskBytes))
    {
        return false;
    }
    if (maskBytes == 0 || maskBytes > kMaxMaskBytes)
    {
        return false;
    }
    if (pixelBytes > UINT32_MAX - 6u)
    {
        return false;
    }

    uint32_t total = 6u + pixelBytes;
    if (total > UINT32_MAX - maskBytes)
    {
        return false;
    }
    total += maskBytes;
    if (fileSize != total)
    {
        return false;
    }

    layout.width = width;
    layout.height = height;
    layout.pixelBytes = pixelBytes;
    layout.maskBytes = static_cast<uint16_t>(maskBytes);
    layout.totalBytes = total;
    return true;
}

inline bool maskBitVisible(const uint8_t *mask, uint16_t width, uint16_t x, uint16_t y)
{
    const uint32_t rowBytes = (static_cast<uint32_t>(width) + 7u) / 8u;
    const uint32_t offset = (static_cast<uint32_t>(y) * rowBytes) + (static_cast<uint32_t>(x) / 8u);
    const uint8_t bit = static_cast<uint8_t>(0x80u >> (x & 7u));
    return (mask[offset] & bit) != 0;
}

} // namespace imagebin
} // namespace displaywx
