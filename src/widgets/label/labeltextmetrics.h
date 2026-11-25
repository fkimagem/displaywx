#pragma once
#include "../../extras/textmetrics.h"

// C++ adapter for editor-generated metric tables (1/64 px).
namespace EdgewxText {
using Token = dwx_text_token;
using Pair = dwx_text_pair;
using Metrics = dwx_text_metrics;
using Box = dwx_text_box;
inline Box measure(const char *text, const Metrics &font, unsigned scale = 1)
{
    return dwx_text_measure(text, &font, scale);
}
}
