#ifndef CHECKOUT_CAMERA_H
#define CHECKOUT_CAMERA_H

#include "../user_setup.h"
#include <esp_idf_version.h>
#include "sdkconfig.h"

// C-compatible gate shared by the widgets and the bundled sensor drivers.
#if defined(DISP_DEFAULT) && defined(CONFIG_IDF_TARGET_ESP32P4) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 4, 0)) && (defined(CAM_OV02C10) || defined(CAM_OV5647) || defined(CAM_SC2336) || defined(CAM_OV2710))
#if !defined(HAS_CAMERA_MIPI)
#define HAS_CAMERA_MIPI 1
#endif
#else
#undef HAS_CAMERA_MIPI
#endif

#endif
