#ifndef CHECK_TOUCH_H
#define CHECK_TOUCH_H

#include "../user_setup.h"

// Touch integration is available with the Arduino GFX backend only.
#if defined(DISP_DEFAULT) && (defined(TOUCH_FT6236U) || defined(TOUCH_FT6336) || defined(TOUCH_CST816) || defined(TOUCH_GT911) || defined(TOUCH_GT911_WAVESHARE) || defined(TOUCH_XPT2046) || defined(TOUCH_GSL3680) || defined(TOUCH_CST820))
#if !defined(HAS_TOUCH)
#define HAS_TOUCH 1
#endif
#else
#undef HAS_TOUCH
#endif

#if defined(DISP_DEFAULT) && defined(TOUCH_XPT2046)
#define DFK_TOUCH_XPT2046
#endif

#endif
