#include "../user_setup.h"

#if (defined(TOUCH_FT6236U) || defined(TOUCH_FT6336) || defined(TOUCH_CST816) || defined(TOUCH_GT911) || defined(TOUCH_GT911_WAVESHARE) || defined(TOUCH_XPT2046) || defined(TOUCH_GSL3680) || defined(TOUCH_CST820))
#if !defined(HAS_TOUCH)
#define HAS_TOUCH true
#endif
#else
#undef HAS_TOUCH
#endif