#ifndef DISPLAYWX_USER_SETUP_H
#define DISPLAYWX_USER_SETUP_H
// Set DISPLAYWX_CONFIG_HEADER for every translation unit through build_opt.h.
#if defined(DISPLAYWX_CONFIG_HEADER)
#include DISPLAYWX_CONFIG_HEADER
#else
#define DISP_DEFAULT
#define TOUCH_FT6236U

#endif
#endif
