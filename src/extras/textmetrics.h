#ifndef DISPLAYWX_TEXTMETRICS_H
#define DISPLAYWX_TEXTMETRICS_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
#define DWX_METRIC_DEFAULT(value) = value
extern "C" {
#else
#define DWX_METRIC_DEFAULT(value)
#endif

typedef struct {
    const char *text;
    int32_t advance;
    int16_t overhang, ascent, descent;
} dwx_text_token;
typedef struct { uint16_t left, right; int16_t adjustment; } dwx_text_pair;
typedef struct dwx_text_metrics {
    const dwx_text_token *tokens;
    uint16_t tokenCount;
    const dwx_text_pair *pairs;
    uint16_t pairCount;
    int32_t ascent, descent, leading, tabWidth;
    /* -1 searches for "?"; if absent, unknown codepoints have zero advance. */
    int32_t replacementIndex DWX_METRIC_DEFAULT(-1);
    /* Optional caller-owned immutable index, populated by dwx_text_index(). */
    const uint16_t *tokenLengths DWX_METRIC_DEFAULT(NULL);
    const uint16_t *tokenOrder DWX_METRIC_DEFAULT(NULL);
    const uint16_t *byteOffsets DWX_METRIC_DEFAULT(NULL); /* 257 elements */
    bool pairsSorted DWX_METRIC_DEFAULT(false);
} dwx_text_metrics;
typedef struct { int width, height; float baseline; } dwx_text_box;

bool dwx_text_index(dwx_text_metrics *font, uint16_t *lengths, uint16_t *order,
                    size_t capacity, uint16_t offsets[257]);
dwx_text_box dwx_text_measure(const char *text, const dwx_text_metrics *font, unsigned scale);
#ifdef __cplusplus
}
#endif
#undef DWX_METRIC_DEFAULT
#endif
