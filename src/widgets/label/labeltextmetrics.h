#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>

// Shared with DisplayWX's widgets/label/labeltextmetrics.h. Values use 1/64 px,
// the same precision as Qt's text layout. No Qt or Arduino dependency.
namespace EdgewxText {
struct Token {
    const char *text;
    int32_t advance;
    int16_t overhang, ascent, descent;
};
struct Pair { uint16_t left, right; int16_t adjustment; };
struct Metrics {
    const Token *tokens;
    uint16_t tokenCount;
    const Pair *pairs;
    uint16_t pairCount;
    int32_t ascent, descent, leading, tabWidth;
};
struct Box { int width, height; float baseline; };

inline Box measure(const char *text, const Metrics &font, unsigned scale = 1)
{
    if (!text) text = "";
    if (!scale) scale = 1;
    int32_t width = 0, maximumWidth = 0, ascent = font.ascent, descent = font.descent;
    int32_t firstAscent = ascent, totalHeight = 0, layoutHeight = 0;
    int previous = -1, overhang = 0, lines = 0;
    auto finishLine = [&]() {
        if (width + overhang > maximumWidth) maximumWidth = width + overhang;
        if (!lines) firstAscent = ascent;
        else totalHeight = ((totalHeight + font.leading + 63) / 64) * 64;
        totalHeight += ascent + descent;
        layoutHeight += ((ascent + descent + 63) / 64) * 64;
        ++lines;
        width = 0; previous = -1; overhang = 0;
        ascent = font.ascent; descent = font.descent;
    };
    while (*text) {
        if (*text == '\n') { finishLine(); ++text; continue; }
        if (*text == '\t') {
            const int32_t tab = font.tabWidth > 0 ? font.tabWidth : 64;
            width = (width / tab + 1) * tab;
            previous = -1; overhang = 0; ++text; continue;
        }
        int selected = -1;
        size_t length = 0;
        for (unsigned i = 0; i < font.tokenCount; ++i) {
            const size_t n = strlen(font.tokens[i].text);
            if (n > length && strncmp(text, font.tokens[i].text, n) == 0) {
                selected = int(i); length = n;
            }
        }
        if (selected < 0) {
            // A codepoint absent from the exported table uses the replacement width.
            selected = *text == '\r' ? 0 : '?' - ' ';
            length = 1;
            while ((static_cast<unsigned char>(text[length]) & 0xc0) == 0x80) ++length;
        }
        const Token &token = font.tokens[selected];
        if (previous >= 0) {
            for (unsigned i = 0; i < font.pairCount; ++i)
                if (font.pairs[i].left == previous && font.pairs[i].right == selected) {
                    width += font.pairs[i].adjustment; break;
                }
        }
        width += token.advance;
        overhang = token.overhang;
        if (token.ascent > ascent) ascent = token.ascent;
        if (token.descent > descent) descent = token.descent;
        previous = selected;
        text += length;
    }
    finishLine();
    const int h = (totalHeight + 63) / 64;
    return {int((maximumWidth + 63) / 64 > 0 ? (maximumWidth + 63) / 64 : 1) * int(scale),
            h * int(scale), float(firstAscent + (h * 64 - layoutHeight) / 2) * scale / 64.0f};
}
}
