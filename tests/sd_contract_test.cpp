#include <stdio.h>
#include <string.h>

#include "../src/storage/imagebinary.h"
#include "../src/storage/sdpolicy.h"

namespace {

int g_failures = 0;

void expect(bool condition, const char *name)
{
    if (condition)
    {
        return;
    }
    ++g_failures;
    printf("FAIL %s\n", name);
}

void testFrequency()
{
    using displaywx::storage::frequencyInPolicy;
    expect(frequencyInPolicy(4000000u), "default frequency");
    expect(frequencyInPolicy(100000u), "minimum frequency");
    expect(frequencyInPolicy(20000000u), "maximum frequency");
    expect(!frequencyInPolicy(0u), "zero frequency");
    expect(!frequencyInPolicy(99999u), "frequency below policy");
    expect(!frequencyInPolicy(20000001u), "frequency above policy");
}

void testPaths()
{
    using displaywx::storage::isCardDirectoryPath;
    using displaywx::storage::isCardFilePath;
    expect(isCardFilePath("/config.txt"), "file path");
    expect(isCardFilePath("/imagens/logo.bin"), "nested file path");
    expect(!isCardFilePath(nullptr), "null file path");
    expect(!isCardFilePath(""), "empty file path");
    expect(!isCardFilePath("config.txt"), "relative file path");
    expect(!isCardFilePath("/"), "root is not a file");
    expect(!isCardFilePath("/imagens/"), "trailing slash is not a file");
    expect(isCardDirectoryPath("/"), "root directory");
    expect(isCardDirectoryPath("/imagens"), "nested directory");
    expect(!isCardDirectoryPath(nullptr), "null directory");
    expect(!isCardDirectoryPath("imagens"), "relative directory");
}

void testFixture()
{
    const uint8_t file[] = {0x00, 0x02, 0x00, 0x01, 0xF8, 0x00, 0x07, 0xE0, 0x00, 0x01, 0xC0};
    const uint16_t width = displaywx::imagebin::readBe16(file);
    const uint16_t height = displaywx::imagebin::readBe16(file + 2);
    displaywx::imagebin::Layout layout{};
    expect(displaywx::imagebin::computeLayout(width, height, 2, sizeof(file), layout), "2x1 layout");
    expect(layout.width == 2 && layout.height == 1, "2x1 dimensions");
    expect(layout.pixelBytes == 4 && layout.maskBytes == 1 && layout.totalBytes == sizeof(file), "2x1 sizes");
    expect(displaywx::imagebin::readBe16(file + 4) == 0xF800, "red pixel");
    expect(displaywx::imagebin::readBe16(file + 6) == 0x07E0, "green pixel");
    expect(displaywx::imagebin::readBe16(file + 8) == 1, "mask length");
    expect(displaywx::imagebin::maskBitVisible(file + 10, width, 0, 0), "first pixel visible");
    expect(displaywx::imagebin::maskBitVisible(file + 10, width, 1, 0), "second pixel visible");
}

void testWidthNine()
{
    const uint16_t width = 9;
    const uint16_t height = 2;
    const uint32_t pixelBytes = 9u * 2u * 2u;
    const uint32_t maskBytes = 4u;
    const uint32_t total = 6u + pixelBytes + maskBytes;
    displaywx::imagebin::Layout layout{};
    expect(displaywx::imagebin::computeLayout(width, height, 2, total, layout), "width 9 layout");
    expect(layout.maskBytes == 4, "width 9 uses two mask bytes per row");

    const uint8_t mask[] = {0x80, 0x80, 0x01, 0x00};
    expect(displaywx::imagebin::maskBitVisible(mask, width, 0, 0), "row byte alignment x0");
    expect(!displaywx::imagebin::maskBitVisible(mask, width, 1, 0), "row byte alignment x1");
    expect(displaywx::imagebin::maskBitVisible(mask, width, 8, 0), "ninth pixel uses the next byte");
    expect(displaywx::imagebin::maskBitVisible(mask, width, 7, 1), "second row stays inside its bytes");
    expect(!displaywx::imagebin::maskBitVisible(mask, width, 8, 1), "second row ninth pixel");
}

void testRejection()
{
    displaywx::imagebin::Layout layout{};
    const uint8_t shortHeader[] = {0x00, 0x02, 0x00};
    expect(sizeof(shortHeader) < 6, "short header is below the minimum record");
    expect(!displaywx::imagebin::computeLayout(2, 1, 2, sizeof(shortHeader), layout), "short file");
    expect(!displaywx::imagebin::computeLayout(2, 1, 2, 12, layout), "truncated or extra bytes");
    expect(!displaywx::imagebin::computeLayout(0, 1, 2, 6, layout), "zero width");
    expect(!displaywx::imagebin::computeLayout(1, 0, 2, 6, layout), "zero height");
    expect(!displaywx::imagebin::computeLayout(4097, 1, 2, 100, layout), "axis above 4096");
    expect(!displaywx::imagebin::computeLayout(4096, 128, 2, 6u + (4096u * 128u * 2u) + (512u * 128u), layout), "mask above 16 bits");
    expect(displaywx::imagebin::computeLayout(8, 1, 1, 6u + 8u + 1u, layout), "monochrome byte layout");
    expect(layout.pixelBytes == 8 && layout.maskBytes == 1, "monochrome sizes");
}

} // namespace

int main()
{
    testFrequency();
    testPaths();
    testFixture();
    testWidthNine();
    testRejection();
    if (g_failures != 0)
    {
        printf("%d contract checks failed\n", g_failures);
        return 1;
    }
    printf("sd contract checks passed\n");
    return 0;
}
