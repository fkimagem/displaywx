#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "../src/extras/dwx_safe.h"
#include "../src/widgets/label/labeltextmetrics.h"

int main()
{
    size_t bytes = 99;
    assert(dwx_size_mul(66, 66 * 2, &bytes) && bytes == 8712);
    assert(dwx_align_up(bytes, 128, &bytes) && bytes == 8832);
    assert(!dwx_size_mul(SIZE_MAX, 2, &bytes));
    assert(!dwx_align_up(SIZE_MAX, 128, &bytes));
    assert(!dwx_align_up(5, 3, &bytes));
    assert(!dwx_align_up(5, 0, &bytes));
    assert(dwx_size_mul(0, SIZE_MAX, &bytes) && bytes == 0);

    float angle = -1;
    assert(dwx_normalize_angle(-450.0f, &angle) && angle == 270.0f);
    assert(dwx_normalize_angle(FLT_MAX, &angle) && angle >= 0 && angle < 360);
    assert(!dwx_normalize_angle(INFINITY, &angle));
    assert(!dwx_normalize_angle(NAN, &angle));
    assert(dwx_map_clamped(INT_MIN, INT_MIN, INT_MAX, 65535, 0) == 65535);
    assert(dwx_map_clamped(INT_MAX, INT_MIN, INT_MAX, 65535, 0) == 0);
    assert(dwx_map_clamped(0, INT_MIN, INT_MAX, 65535, 0) == 32768);
    assert(dwx_map_clamped(0, INT_MIN, INT_MAX, INT_MIN, INT_MAX) == 0);
    assert(dwx_map_clamped(5, 10, 0, 0, 100) == 50);
    assert(dwx_map_clamped(-4, 0, 10, 20, 30) == 20);

    struct { char text[20]; unsigned sentinel; } formatted = {{}, 0xabcdef12};
    const float values[] = {FLT_MAX, -FLT_MAX, FLT_MIN, -123.456f, INFINITY, NAN};
    for (float value : values) {
        assert(dwx_format_float(formatted.text, sizeof(formatted.text), value, 5));
        assert(formatted.sentinel == 0xabcdef12);
        assert(strlen(formatted.text) < sizeof(formatted.text));
    }
    assert(dwx_format_float(formatted.text, sizeof(formatted.text), 12.5f, 2));
    assert(strcmp(formatted.text, "12.50") == 0);
    char tiny[2];
    assert(!dwx_format_float(tiny, sizeof(tiny), 123.0f, 2) && tiny[0] == 0);

    const size_t count = 70000;
    static int ring[count], out[count];
    for (size_t i = 0; i < count; ++i) ring[i] = (int)i;
    const size_t heads[] = {0, 1, count - 1};
    for (size_t head : heads) {
        assert(dwx_copy_ring_int(out, ring, count, head));
        for (size_t i = 0; i < count; ++i) assert(out[i] == (int)((head + i) % count));
    }
    assert(!dwx_copy_ring_int(out, ring, 0, 0));
    assert(!dwx_copy_ring_int(out, ring, count, count));

    using namespace EdgewxText;
    const Token tokens[] = {{"A", 640, 0, 512, 128}, {"B", 640, 0, 512, 128},
                            {"?", 320, 0, 512, 128}, {"AA", 1152, 0, 512, 128}};
    const Pair pairs[] = {{0, 1, -64}};
    Metrics font = {tokens, 4, pairs, 1, 512, 128, 0, 1280};
    assert(measure("AB", font).width == 19);
    assert(measure("AA", font).width == 18); // longest token wins
    assert(measure("AZ", font).width == 15); // replacement is index 2, not 31
    assert(measure("A\xc3\xa9", font).width == 15);
    auto multiline = measure("AB\n?", font, 2);
    assert(multiline.width == 38 && multiline.height == 40 && multiline.baseline == 16);
    Metrics empty = {};
    assert(measure("missing", empty).width == 1);
    Metrics shortFont = {tokens, 1, nullptr, 0, 512, 128, 0, 0};
    assert(measure("Z", shortFont).width == 1);
    assert(measure("A", font, UINT_MAX).width == INT_MAX);
    uint16_t lengths[4], order[4], offsets[257];
    assert(!dwx_text_index(&font, lengths, order, 3, offsets));
    assert(dwx_text_index(&font, lengths, order, 4, offsets));
    font.pairsSorted = true;
    assert(measure("AB", font).width == 19);
    assert(measure("AA", font).width == 18);
    assert(measure("AZ", font).width == 15);
    puts("PASS: bounded arithmetic, formatting, ring snapshots and text metrics");
}
