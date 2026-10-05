#ifndef CHECKOUT_CAMERA_H
#define CHECKOUT_CAMERA_H

#include "../user_setup.h"

// Define HAS_CAMERA se a camera estiver definida na user_setup.h
#if (defined(CAM_OV02C10) || defined(CAM_OV5647) || defined(CAM_SC2336) || defined(CAM_OV2710))
#if !defined(HAS_CAMERA_MIPI)
#define HAS_CAMERA_MIPI 1
#endif
#else
#undef HAS_CAMERA_MIPI
#endif

#endif