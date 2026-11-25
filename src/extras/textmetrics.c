#include "textmetrics.h"
#include <limits.h>
#include <string.h>

#ifndef DFK_TEXT_MAX_BYTES
#define DFK_TEXT_MAX_BYTES 65535U
#endif

static size_t bounded_length(const char *text, size_t maximum)
{
    size_t n = 0;
    if (text) while (n < maximum && text[n]) ++n;
    return n;
}

bool dwx_text_index(dwx_text_metrics *font, uint16_t *lengths, uint16_t *order,
                    size_t capacity, uint16_t offsets[257])
{
    if (!font || !font->tokens || !lengths || !order || !offsets || capacity < font->tokenCount) return false;
    uint16_t counts[256] = {0};
    for (size_t i = 0; i < font->tokenCount; ++i) {
        const char *text = font->tokens[i].text;
        const size_t length = bounded_length(text, (size_t)UINT16_MAX + 1);
        if (!length || length > UINT16_MAX) return false;
        lengths[i] = (uint16_t)length;
        ++counts[(unsigned char)text[0]];
    }
    offsets[0] = 0;
    for (unsigned b = 0; b < 256; ++b) offsets[b + 1] = offsets[b] + counts[b];
    memcpy(counts, offsets, sizeof(counts));
    for (uint32_t i = 0; i < font->tokenCount; ++i)
        order[counts[(unsigned char)font->tokens[i].text[0]]++] = (uint16_t)i;
    font->tokenLengths = lengths;
    font->tokenOrder = order;
    font->byteOffsets = offsets;
    return true;
}

static int find_single(const dwx_text_metrics *font, char character)
{
    for (unsigned i = 0; i < font->tokenCount; ++i) {
        const char *t = font->tokens[i].text;
        if (t && t[0] == character && t[1] == '\0') return (int)i;
    }
    return -1;
}

static int pair_adjustment(const dwx_text_metrics *font, int left, int right)
{
    if (!font->pairs || left < 0 || right < 0) return 0;
    const uint32_t key = ((uint32_t)left << 16) | (uint32_t)right;
    if (font->pairsSorted) {
        size_t lo = 0, hi = font->pairCount;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            const dwx_text_pair *pair = &font->pairs[mid];
            uint32_t candidate = ((uint32_t)pair->left << 16) | pair->right;
            if (candidate == key) return pair->adjustment;
            if (candidate < key) lo = mid + 1; else hi = mid;
        }
        return 0;
    }
    for (unsigned i = 0; i < font->pairCount; ++i)
        if (font->pairs[i].left == left && font->pairs[i].right == right) return font->pairs[i].adjustment;
    return 0;
}

static int scaled_dimension(int64_t pixels, unsigned scale)
{
    if (pixels <= 0) return 0;
    if (pixels > INT_MAX || scale > (uint64_t)INT_MAX / (uint64_t)pixels) return INT_MAX;
    return (int)(pixels * scale);
}

dwx_text_box dwx_text_measure(const char *text, const dwx_text_metrics *font, unsigned scale)
{
    dwx_text_box empty = {1, 0, 0};
    if (!font || !font->tokens || !font->tokenCount) return empty;
    if (!text) text = "";
    if (!scale) scale = 1;
    size_t remaining = bounded_length(text, DFK_TEXT_MAX_BYTES);
    int fallback = font->replacementIndex;
    if (fallback < 0 || (unsigned)fallback >= font->tokenCount) fallback = find_single(font, '?');
    const int space = find_single(font, ' ');
    int64_t width = 0, maximum = 0, ascent = font->ascent, descent = font->descent;
    int64_t first_ascent = ascent, total = 0, layout = 0, overhang = 0;
    int previous = -1;
    unsigned lines = 0;
    for (;;) {
        if (!remaining || *text == '\n') {
            if (width + overhang > maximum) maximum = width + overhang;
            if (!lines) first_ascent = ascent;
            else total = ((total + font->leading + 63) / 64) * 64;
            total += ascent + descent;
            layout += ((ascent + descent + 63) / 64) * 64;
            ++lines;
            if (!remaining) break;
            ++text; --remaining;
            width = overhang = 0; previous = -1;
            ascent = font->ascent; descent = font->descent;
            continue;
        }
        if (*text == '\t') {
            const int64_t tab = font->tabWidth > 0 ? font->tabWidth : 64;
            width = (width / tab + 1) * tab;
            previous = -1; overhang = 0;
            ++text; --remaining;
            continue;
        }
        int selected = -1;
        size_t length = 0, begin = 0, end = font->tokenCount;
        bool indexed = font->tokenOrder && font->byteOffsets;
        if (indexed) {
            begin = font->byteOffsets[(unsigned char)*text];
            end = font->byteOffsets[(unsigned char)*text + 1];
            if (begin > end || end > font->tokenCount) { indexed = false; begin = 0; end = font->tokenCount; }
        }
        for (size_t k = begin; k < end; ++k) {
            const size_t i = indexed ? font->tokenOrder[k] : k;
            if (i >= font->tokenCount) continue;
            const char *candidate = font->tokens[i].text;
            if (!candidate || candidate[0] != text[0]) continue;
            const size_t n = font->tokenLengths ? font->tokenLengths[i] : bounded_length(candidate, remaining + 1);
            if (n > length && n <= remaining && strncmp(text, candidate, n) == 0) { selected = (int)i; length = n; }
        }
        if (selected < 0) {
            selected = *text == '\r' ? space : fallback;
            length = 1;
            while (length < remaining && ((unsigned char)text[length] & 0xc0) == 0x80) ++length;
        }
        if (selected >= 0) {
            const dwx_text_token *token = &font->tokens[selected];
            width += pair_adjustment(font, previous, selected) + (int64_t)token->advance;
            overhang = token->overhang;
            if (token->ascent > ascent) ascent = token->ascent;
            if (token->descent > descent) descent = token->descent;
        }
        previous = selected;
        text += length; remaining -= length;
    }
    const int64_t height = (total + 63) / 64;
    const int64_t unscaled_width = maximum > 0 ? (maximum + 63) / 64 : 1;
    dwx_text_box result = {scaled_dimension(unscaled_width, scale), scaled_dimension(height, scale),
        (float)(first_ascent + (height * 64 - layout) / 2) * (float)scale / 64.0f};
    return result;
}
