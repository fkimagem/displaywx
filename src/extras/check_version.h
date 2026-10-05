#ifndef CHECK_VERSION_H
#define CHECK_VERSION_H

#include <Arduino.h>
#include "../../user_setup.h"

// Ensure version builder macro exists
#ifndef ESP_ARDUINO_VERSION_VAL
#define ESP_ARDUINO_VERSION_VAL(major, minor, patch) \
  (((major) << 16) | ((minor) << 8) | (patch))
#endif

#if defined(ESP_ARDUINO_VERSION)

// Ensure version component macros exist if the core does not declare them
#ifndef ESP_ARDUINO_VERSION_MAJOR
#define ESP_ARDUINO_VERSION_MAJOR 0
#endif

#ifndef ESP_ARDUINO_VERSION_MINOR
#define ESP_ARDUINO_VERSION_MINOR 0
#endif

#ifndef ESP_ARDUINO_VERSION_PATCH
#define ESP_ARDUINO_VERSION_PATCH 0
#endif

// Valid ESP32 Arduino Core version range
#define ESP32_CORE_MIN ESP_ARDUINO_VERSION_VAL(3, 3, 11)
#define ESP32_CORE_MAX ESP_ARDUINO_VERSION_VAL(3, 3, 12)

// Helpers to convert numbers to strings
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Full Arduino Core version string
#define ARDUINO_CORE_VERSION_STR \
  STR(ESP_ARDUINO_VERSION_MAJOR) "." \
  STR(ESP_ARDUINO_VERSION_MINOR) "." \
  STR(ESP_ARDUINO_VERSION_PATCH)

#if defined(DEBUG_DISPLAY_FK)
#pragma message("Using Arduino Core version: " ARDUINO_CORE_VERSION_STR)
#endif

#if ESP_ARDUINO_VERSION < ESP32_CORE_MIN
  #error "ESP32 Arduino Core version is too old. Please upgrade to a version between 3.3.11 and 3.3.12."
#endif

#if ESP_ARDUINO_VERSION > ESP32_CORE_MAX
  #error "ESP32 Arduino Core version is too new. Please downgrade to a version between 3.3.11 and 3.3.12."
#endif

#if (ESP_ARDUINO_VERSION >= ESP32_CORE_MIN) && (ESP_ARDUINO_VERSION <= ESP32_CORE_MAX)
  #define ESP_ARDUINO_VERSION_OK
  #if defined(DEBUG_DISPLAY_FK)
    #pragma message("ESP32 Arduino Core version " ARDUINO_CORE_VERSION_STR " detected. Version OK.")
  #endif
#endif

#else

#error "ESP32 Arduino Core version could not be detected. Please install ESP32 Arduino Core between version 3.3.11 and 3.3.12."

#endif // ESP_ARDUINO_VERSION

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)
#error "ESP32 IDF version is too old. Please upgrade to a version between 5.4.0 and 5.4.3."
#endif

#if (defined(CAM_OV02C10) + defined(CAM_OV5647) + defined(CAM_SC2336) + defined(CAM_OV2710)) > 1
#error "Only one camera sensor can be enabled. Choose one of CAM_OV02C10, CAM_OV5647, CAM_SC2336 or CAM_OV2710 in user_setup.h."
#endif

#if (defined(TOUCH_FT6236U) + defined(TOUCH_FT6336) + defined(TOUCH_CST816) + defined(TOUCH_GT911) + defined(TOUCH_GT911_WAVESHARE) + defined(TOUCH_XPT2046) + defined(TOUCH_GSL3680) + defined(TOUCH_CST820)) > 1
#error "Only one touch controller can be enabled. Choose one of TOUCH_FT6236U, TOUCH_FT6336, TOUCH_CST816, TOUCH_GT911, TOUCH_GT911_WAVESHARE, TOUCH_XPT2046, TOUCH_GSL3680 or TOUCH_CST820 in user_setup.h."
#endif

#endif // CHECK_VERSION_H