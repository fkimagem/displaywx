#ifndef DISPLAYWX_SAFE_H
#define DISPLAYWX_SAFE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool dwx_size_mul(size_t a, size_t b, size_t *result);
bool dwx_align_up(size_t bytes, size_t alignment, size_t *result);
bool dwx_normalize_angle(float angle, float *result);
/* Saturating input mapping; supports the entire int32_t domain. */
int32_t dwx_map_clamped(int32_t value, int32_t in_min, int32_t in_max,
                        int32_t out_min, int32_t out_max);
bool dwx_format_float(char *out, size_t capacity, float value, unsigned decimals);
/* Source and destination must not overlap. head is the oldest element. */
bool dwx_copy_ring_int(int *out, const int *ring, size_t count, size_t head);

#ifdef __cplusplus
}
#endif
#endif
