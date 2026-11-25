#include "dwx_safe.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

bool dwx_size_mul(size_t a, size_t b, size_t *result)
{
    if (!result || (b && a > SIZE_MAX / b)) return false;
    *result = a * b;
    return true;
}

bool dwx_align_up(size_t bytes, size_t alignment, size_t *result)
{
    if (!result || !alignment || (alignment & (alignment - 1))) return false;
    if (bytes > SIZE_MAX - (alignment - 1)) return false;
    *result = (bytes + alignment - 1) & ~(alignment - 1);
    return true;
}

bool dwx_normalize_angle(float angle, float *result)
{
    if (!result || !isfinite(angle)) return false;
    float normalized = fmodf(angle, 360.0f);
    if (normalized < 0.0f) normalized += 360.0f;
    *result = normalized == 360.0f ? 0.0f : normalized;
    return true;
}

int32_t dwx_map_clamped(int32_t value, int32_t in_min, int32_t in_max,
                        int32_t out_min, int32_t out_max)
{
    if (in_min == in_max) return out_min;
    if (in_min > in_max) {
        int32_t swap = in_min; in_min = in_max; in_max = swap;
        swap = out_min; out_min = out_max; out_max = swap;
    }
    if (value <= in_min) return out_min;
    if (value >= in_max) return out_max;
    const uint64_t input = (uint64_t)((int64_t)value - in_min);
    const uint64_t range = (uint64_t)((int64_t)in_max - in_min);
    const int64_t delta = (int64_t)out_max - out_min;
    const uint64_t magnitude = (uint64_t)(delta < 0 ? -delta : delta);
    const int64_t offset = (int64_t)(input * magnitude / range);
    return (int32_t)((int64_t)out_min + (delta < 0 ? -offset : offset));
}

bool dwx_format_float(char *out, size_t capacity, float value, unsigned decimals)
{
    if (!out || !capacity) return false;
    if (decimals > 5) decimals = 5;
    int written = snprintf(out, capacity, "%.*f", (int)decimals, (double)value);
    if (written < 0 || (size_t)written >= capacity)
        written = snprintf(out, capacity, "%.6g", (double)value);
    if (written < 0 || (size_t)written >= capacity) {
        out[0] = '\0';
        return false;
    }
    return true;
}

bool dwx_copy_ring_int(int *out, const int *ring, size_t count, size_t head)
{
    if (!out || !ring || !count || head >= count || count > SIZE_MAX / sizeof(int)) return false;
    const size_t tail = count - head;
    memcpy(out, ring + head, tail * sizeof(int));
    if (head) memcpy(out + tail, ring, head * sizeof(int));
    return true;
}
