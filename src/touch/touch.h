/*******************************************************************************
 * Touch libraries:
 * FT6X36: https://github.com/strange-v/FT6X36.git
 * GT911: https://github.com/TAMCTec/gt911-arduino.git
 * FT6236U https://github.com/DustinWatts/FT6236/releases
 * CST816S https://github.com/fbiego/CST816S/tree/main
 * FT6336 https://github.com/aselectroworks/Arduino-FT6336/tree/master
 ******************************************************************************/
#ifndef WTOUCH_H
#define WTOUCH_H


#include <Arduino.h>

#include "../../user_setup.h"


#include <cstdint>
#include "../extras/baseTypes.h"

#include "check_touch.h"

#if defined(DEBUG_DISPLAY_FK)
  #if defined(TOUCH_XPT2046)
    #pragma message("Using XPT2046 touch driver")
  #elif defined(TOUCH_FT6236U)
    #pragma message("Using FT6236U touch driver")
  #elif defined(TOUCH_FT6336)
    #pragma message("Using FT6336 touch driver")
  #elif defined(TOUCH_CST816)
    #pragma message("Using CST816 touch driver")
  #elif defined(TOUCH_GT911)
    #pragma message("Using GT911 touch driver")
  #elif defined(TOUCH_GT911_WAVESHARE)
    #pragma message("Using GT911 Waveshare touch driver")
  #elif defined(TOUCH_GSL3680)
    #pragma message("Using GSL3680 touch driver")
  #elif defined(TOUCH_CST820)
    #pragma message("Using CST820 touch driver")
  #else
    #pragma message("No touch driver selected")
  #endif
  #endif

  // Include the touch library
  #if defined(TOUCH_XPT2046)
    #include "xpt2046/XPT2046.h"
    #include <SPI.h>
    #include <Arduino_GFX_Library.h>
  #elif defined(TOUCH_FT6236U)
    #include "ft6236/FT6236.h"
  #elif defined(TOUCH_FT6336)
    #include "ft6336/src/FT6336U.h"
  #elif defined(TOUCH_CST816)
    #include <CST816S.h>
  #elif defined(TOUCH_GT911)
    #include <Wire.h>
    #include "gt911/TAMC_GT911.h"
  #elif defined(TOUCH_GT911_WAVESHARE)
    #include "gt911waveshare/gt911waveshare.h"
  #elif defined(TOUCH_GSL3680)
    //#include <Wire.h>
    #include "gsl3680/src/gsl3680_touch.h"
  #elif defined(TOUCH_CST820)
    #include "cst820/cst820.h"
  #endif


/// @brief Structure representing a calibration point for touch input.
/// @details Contains raw touch coordinates and corresponding screen coordinates.
typedef struct
{
  int16_t xTouch;   ///< Raw value of X from touch.
  int16_t yTouch;   ///< Raw value of Y from touch.
  uint16_t xScreen; ///< X position of the point on the screen.
  uint16_t yScreen; ///< Y position of the point on the screen.
} CalibrationPoint_t;

/// @brief Structure representing a point on the screen.
/// @details Contains X and Y coordinates for screen positioning.
typedef struct
{
  uint16_t x; ///< X-coordinate of the point on the screen.
  uint16_t y; ///< Y-coordinate of the point on the screen.
} ScreenPoint_t;



/// @brief Class for handling touch input on various touch screen controllers.
class TouchScreen
{
private:
static const char *TAG; ///< Tag estática para identificação em logs do ESP32.
  // Private for each touch controller
  #if defined(TOUCH_XPT2046)
  XPT2046 *m_ts = nullptr;
  uint16_t *m_calibration = nullptr;
  #if defined(DISP_DEFAULT)
  Arduino_GFX *m_objTFT = nullptr;
  #elif defined(DISP_PCD8544)
  Adafruit_PCD8544 *m_objTFT = nullptr;
  #elif defined(DISP_SSD1306)
  Adafruit_SSD1306 *m_objTFT = nullptr;
  #else
  #error "Undefined display type"
  #endif
  uint16_t m_touchCalibration_x0 = 300;
  uint16_t m_touchCalibration_x1 = 3600;
  uint16_t m_touchCalibration_y0 = 300;
  uint16_t m_touchCalibration_y1 = 3600;
  uint8_t m_touchCalibration_rotate = 1;
  uint8_t m_touchCalibration_invert_x = 2;
  uint8_t m_touchCalibration_invert_y = 0;
  void drawStar(int16_t xPos, int16_t yPos, uint8_t size, uint16_t color);
  SPIClass *m_spitoque = nullptr;
  uint8_t m_pinCS = 0;
  /// @brief Pointer to calibration matrix. [0] = TL, [1] = TR, [2] = BR, [3] = BL
  CalibrationPoint_t *m_calibMatrix = nullptr;
  #elif defined(TOUCH_FT6236U)

  bool m_touch_touched_flag = true;
  bool m_touch_released_flag = true;
  FT6236 *m_ts = nullptr;
  static TouchScreen *m_instance;
  int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;

  #elif defined(TOUCH_FT6336)
  FT6336U *m_ts = nullptr;
  FT6336U_TouchPointType tp;
  int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;
  
  #elif defined(TOUCH_CST816)
  CST816S *m_ts = nullptr;
  int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;

  #elif defined(TOUCH_GT911)
  TAMC_GT911 *m_ts = nullptr;
  int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;

  #elif defined(TOUCH_GT911_WAVESHARE)
  Gt911Waveshare *m_ts = nullptr;
  int8_t m_pinSCL = 0, m_pinSDA = 0;

  #elif defined(TOUCH_GSL3680)
  GSL3680_touch *m_ts = nullptr;
  int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;

  #elif defined(TOUCH_CST820)
 CST820 *m_ts = nullptr;
int8_t m_pinSCL = 0, m_pinSDA = 0, m_pinINT = 0, m_pinRST = 0;
  #else
  //
  #endif

  bool touch_has_signal();
  bool touch_touched();
  bool touch_released();

  void showMap(const String& axis, int16_t minValue, int16_t maxValue);
  

  uint16_t m_widthScreen;      ///< Width of the screen.
  uint16_t m_heightScreen;     ///< Height of the screen.
  uint16_t m_touch_last_x;     ///< Last recorded X-coordinate of the touch.
  uint16_t m_touch_last_y;     ///< Last recorded Y-coordinate of the touch.
  uint16_t m_touch_last_z;     ///< Last recorded Y-coordinate of the touch.
  bool m_startedTouch = false; ///< Flag indicating if a touch has started.
  uint8_t m_rotation = 0;      ///< Rotation setting for the touch screen.
  int m_touchFrequency = 2500000;    ///< Touch frequency setting for the touch screen.
  int m_displayFrequency = 27000000;  ///< Display frequency setting for the touch screen.

  int m_x0 = 0, m_x1 = 0, m_y0 = 0, m_y1 = 0;
  bool m_invertXAxis = false, m_invertYAxis = false, m_swapAxis = false;
  int m_displayPinCS = -1;
  bool m_logMessages = false;


public:
  TouchScreen();
  ~TouchScreen();
  uint16_t getWidthScreen();
  uint16_t getHeightScreen();
  void touch_init();
  void setDimension(uint16_t _width, uint16_t _height, uint8_t _rotation);
  uint8_t m_gesture = 0; ///< Gesture identifier for touch events.
  bool getTouch(uint16_t *xTouch, uint16_t *yTouch, int *zPressure);
  uint8_t getGesture();
  uint8_t getRotation();

  void setTouchCorners(int x0, int x1, int y0, int y1);
  void setInvertAxis(bool invert_x, bool invert_y);
  void setSwapAxis(bool swap);
  void setLogMessages(bool enable);

  #if defined(TOUCH_XPT2046)
  CalibrationPoint_t getMinPoint(CalibrationPoint_t pontos[4]);
  CalibrationPoint_t getMaxPoint(CalibrationPoint_t pontos[4]);
  void startAsXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSclk,uint8_t pinMosi, uint8_t pinMiso, uint8_t pinCS, SPIClass *_sharedSPI, Arduino_GFX *_objTFT, int touchFrequency, int displayFrequency, int displayPinCS);
  void calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size);
  void calibrateTouchStruct(CalibrationPoint_t *points, uint8_t length, Rect_t* rectScreen, uint32_t color_fg, uint32_t color_bg, uint8_t sizeMarker);
  ScreenPoint_t getScreenPosition(int16_t xTouch, int16_t yTouch);
  ScreenPoint_t touchToTelaPonto0e2(int16_t xTouch, int16_t yTouch);
  int mapTouch(int val, int in_min, int in_max, int out_min, int out_max);


  void setCalibration(CalibrationPoint_t *array);

  #elif defined(TOUCH_FT6236U)

  void startAsFT6236U(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);
  //void touch(TPoint p, TEvent e);
  //static void touchHandler(TPoint p, TEvent e);

  #elif defined(TOUCH_FT6336)

  void startAsFT6336(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);
  void showSetup();

  #elif defined(TOUCH_CST816)
  void startAsCST816(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);

  #elif defined(TOUCH_GT911)
  void startAsGT911(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);

  #elif defined(TOUCH_GT911_WAVESHARE)
  void startAsGT911Waveshare(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL);

  #elif defined(TOUCH_GSL3680)
  void startAsGSL3680(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);

  #elif defined(TOUCH_CST820)
  void startAsCST820(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST);

  #endif
};

#endif