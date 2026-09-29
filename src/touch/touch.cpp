#include "touch.h"

#if defined(TOUCH_FT6236U)
// TouchScreen *TouchScreen::m_instance = nullptr;
#endif


const char *TouchScreen::TAG = "TouchScreen";

TouchScreen::TouchScreen()
{
  ESP_LOGD(TAG, "TouchScreen constructor");
}

#if defined(TOUCH_XPT2046)
void TouchScreen::startAsXPT2046(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSclk, uint8_t pinMosi, uint8_t pinMiso, uint8_t pinCS, SPIClass *_sharedSPI, Arduino_GFX *_objTFT, int touchFrequency, int displayFrequency, int displayPinCS)
{
  m_displayPinCS = displayPinCS;
  m_touchFrequency = touchFrequency;
  m_displayFrequency = displayFrequency;
  m_ts = new XPT2046(pinSclk, pinMiso, pinMosi, pinCS);
  this->setDimension(w, h, _rotation);
  m_objTFT = _objTFT;
  m_spitoque = _sharedSPI;
  m_pinCS = pinCS;
  if (m_pinCS)
  {
    pinMode(m_pinCS, OUTPUT);
    digitalWrite(m_pinCS, HIGH);
  }
  touch_init();
  ESP_LOGD(TAG, "Starting xpt2046 touch panel driver");
}
#elif defined(TOUCH_FT6236U)
void TouchScreen::startAsFT6236U(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  m_ts = new FT6236();
  this->setDimension(w, h, _rotation);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting ft6236 touch panel driver");
}
#elif defined(TOUCH_FT6336)
void TouchScreen::startAsFT6336(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  m_ts = new FT6336U(pinSDA, pinSCL, pinRST, pinINT);
  this->setDimension(w, h, _rotation);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting ft6336 touch panel driver");
}
#elif defined(TOUCH_CST816)
void TouchScreen::startAsCST816(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  m_ts = new CST816S(pinSDA, pinSCL, pinRST, pinINT);
  this->setDimension(w, h, _rotation);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting cst816 touch panel driver");
}
#elif defined(TOUCH_GT911)
void TouchScreen::startAsGT911(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  // m_ts = new TAMC_GT911(pinSDA, pinSCL, pinINT, pinRST, max(m_x0, m_x1), max(m_y0, m_y1));
  this->setDimension(w, h, _rotation);
  m_ts = new TAMC_GT911(pinSDA, pinSCL, pinINT, pinRST, m_widthScreen, m_heightScreen);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting gt911 touch panel driver");
}
#elif defined(TOUCH_GT911_WAVESHARE)
void TouchScreen::startAsGT911Waveshare(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL)
{
  this->setDimension(w, h, _rotation);
  m_ts = new Gt911Waveshare();
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  touch_init();
  log_d("Starting gt911 waveshare touch panel driver");
}
#elif defined(TOUCH_GSL3680)
void TouchScreen::startAsGSL3680(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  // m_ts = new gsl3680_touch(TP_I2C_SDA, TP_I2C_SCL, TP_RST, TP_INT);
  this->setDimension(w, h, _rotation);
  m_ts = new GSL3680_touch(pinSDA, pinSCL, pinRST, pinINT);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting gsl3680 touch panel driver");
}

#elif defined(TOUCH_CST820)
void TouchScreen::startAsCST820(uint16_t w, uint16_t h, uint8_t _rotation, uint8_t pinSDA, uint8_t pinSCL, uint8_t pinINT, uint8_t pinRST)
{
  m_ts = new CST820(pinSDA, pinSCL, pinRST, pinINT);
  this->setDimension(w, h, _rotation);
  m_pinSCL = pinSCL;
  m_pinSDA = pinSDA;
  m_pinINT = pinINT;
  m_pinRST = pinRST;
  touch_init();
  log_d("Starting cst820 touch panel driver");
}
#endif

uint16_t TouchScreen::getWidthScreen()
{
#if defined(HAS_TOUCH)
  return m_widthScreen;
#else
  return 0;
#endif
}

uint16_t TouchScreen::getHeightScreen()
{
#if defined(HAS_TOUCH)
  return m_heightScreen;
#else
  return 0;
#endif
}

/**
 * @brief Destructor for the TouchScreen class.
 */
TouchScreen::~TouchScreen()
{
#if defined(HAS_TOUCH)
  if (m_ts)
  {
    delete m_ts;
  }
#endif
}

void TouchScreen::setLogMessages(bool logMessages)
{
  m_logMessages = logMessages;
}

void TouchScreen::setTouchCorners(int x0, int x1, int y0, int y1)
{
  m_x0 = x0;
  m_y0 = y0;
  m_x1 = x1;
  m_y1 = y1;

  ESP_LOGD(TAG, "X0: %d X1: %d Y0: %d Y1: %d", m_x0, m_x1, m_y0, m_y1);
  
}

void TouchScreen::setInvertAxis(bool invertX, bool invertY)
{
  m_invertXAxis = invertX;
  m_invertYAxis = invertY;
}

void TouchScreen::setSwapAxis(bool swap)
{
  m_swapAxis = swap;
}

#if defined(TOUCH_FT6236)
TouchScreen *TouchScreen::m_instance = nullptr;

/**
 * @brief Handles touch events.
 * @param p Touch point.
 * @param e Touch event type.
 */
void TouchScreen::touch(TPoint p, TEvent e)
{
  if (e != TEvent::Tap && e != TEvent::DragStart && e != TEvent::DragMove && e != TEvent::DragEnd)
  {
    return;
  }
  // translation logic depends on screen rotation
  if (m_swapAxis)
  {
    m_touch_last_x = map(p.y, m_x0, m_x1, 0, m_widthScreen);
    m_touch_last_y = map(p.x, m_y0, m_y1, 0, m_heightScreen);
  }
  else
  {
    m_touch_last_x = map(p.x, m_x0, m_x1, 0, m_widthScreen);
    m_touch_last_y = map(p.y, m_y0, m_y1, 0, m_heightScreen);
  }

  switch (e)
  {
  case TEvent::Tap:
    ESP_LOGD(TAG, "Tap");
    m_touch_touched_flag = true;
    m_touch_released_flag = true;
    break;
  case TEvent::DragStart:
    ESP_LOGD(TAG, "DragStart");
    m_touch_touched_flag = true;
    break;
  case TEvent::DragMove:
    ESP_LOGD(TAG, "DragMove");
    m_touch_touched_flag = true;
    break;
  case TEvent::DragEnd:
    ESP_LOGD(TAG, "DragEnd");
    m_touch_released_flag = true;
    break;
  default:
    ESP_LOGD(TAG, "UNKNOWN");
    break;
  }
}
#endif

/**
 * @brief Initializes the touch screen.
 */
void TouchScreen::touch_init()
{
  ESP_LOGD(TAG, "touch_init");
#if defined(TOUCH_GT911)
  m_ts->begin();
  m_ts->setRotation(m_rotation);

#elif defined(TOUCH_GT911_WAVESHARE)
  if (!m_ts->begin(m_pinSDA, m_pinSCL, m_widthScreen, m_heightScreen)) {
    Serial.println("GT911 Waveshare nao iniciado");
  }

#elif defined(TOUCH_XPT2046)

  if (m_displayPinCS > 0)
  {
    digitalWrite(m_displayPinCS, HIGH);
    delayMicroseconds(5);
  }

  m_ts->begin(m_spitoque, m_touchFrequency, m_displayFrequency);

  if (m_displayPinCS > 0)
  {
    delayMicroseconds(5);
    digitalWrite(m_displayPinCS, LOW);
  }

#elif defined(TOUCH_FT6236U)
  if (!m_ts->begin(40, m_pinSDA, m_pinSCL)) // 40 in this case represents the sensitivity. Try higer or lower for better response.
  {
    ESP_LOGD(TAG, "Unable to start the capacitive touchscreen.");
  }
  else
  {
    ESP_LOGD(TAG, "Touch initialized.");
  }
#elif defined(TOUCH_CST816)
  m_ts->begin();
#elif defined(TOUCH_FT6336)
  m_ts->begin();
  showSetup();
#elif defined(TOUCH_GSL3680)
  m_ts->begin();
  m_ts->set_rotation(m_rotation);

  #elif defined(TOUCH_CST820)
  m_ts->begin();
#endif
}

#if defined(TOUCH_FT6336)
void TouchScreen::showSetup()
{
  Serial.print("FT6336 Device Mode: ");
  Serial.println(m_ts->read_device_mode());
  Serial.print("FT6336 Threshold: 0x");
  Serial.println(m_ts->read_touch_threshold(), HEX);
  Serial.print("FT6336 Filter Coefficient: 0x");
  Serial.println(m_ts->read_filter_coefficient(), HEX);
  Serial.print("FT6336 Control Mode: 0x");
  Serial.println(m_ts->read_ctrl_mode(), HEX);
  Serial.print("FT6336 Time Period for enter to Monitor Mode: 0x");
  Serial.println(m_ts->read_time_period_enter_monitor(), HEX);
  Serial.print("FT6336 Active Rate: 0x");
  Serial.println(m_ts->read_active_rate(), HEX);
  Serial.print("FT6336 Monitor Rate: 0x");
  Serial.println(m_ts->read_monitor_rate(), HEX);

  Serial.print("FT6336 LIB Ver: 0x");
  Serial.println(m_ts->read_library_version(), HEX);
  Serial.print("FT6336 Chip ID: 0x");
  Serial.println(m_ts->read_chip_id(), HEX);
  Serial.print("FT6336 G Mode: 0x");
  Serial.println(m_ts->read_g_mode(), HEX);
  Serial.print("FT6336 POWER Mode: 0x");
  Serial.println(m_ts->read_pwrmode(), HEX);
  Serial.print("FT6336 Firm ID: 0x");
  Serial.println(m_ts->read_firmware_id(), HEX);
  Serial.print("FT6336 Focal Tehc ID: 0x");
  Serial.println(m_ts->read_focaltech_id(), HEX);
  Serial.print("FT6336 Release Code ID: 0x");
  Serial.println(m_ts->read_release_code_id(), HEX);
  Serial.print("FT6336 State: 0x");
  Serial.println(m_ts->read_state(), HEX);
}
#endif

void TouchScreen::setDimension(uint16_t _widthScreen, uint16_t _heightScreen, uint8_t _rotation)
{
  m_widthScreen = _widthScreen;
  m_heightScreen = _heightScreen;
  m_rotation = _rotation;
  m_startedTouch = true;
  // log dimension of touch
  ESP_LOGD(TAG, "Touch screen dimension: %d x %d", m_widthScreen, m_heightScreen);
}

/**
 * @brief Checks if the touch screen has a signal.
 * @return True if the touch screen has a signal, false otherwise.
 */
bool TouchScreen::touch_has_signal()
{
#if defined(TOUCH_FT6236)
  m_ts->loop();
  return m_touch_touched_flag || m_touch_released_flag;

#elif defined(TOUCH_GT911) || defined(TOUCH_GT911_WAVESHARE)
  return true;

#elif defined(TOUCH_XPT2046)
  return true;
#elif defined(TOUCH_FT6236U)
  return true;
#elif defined(TOUCH_CST816)
  return true;
#elif defined(TOUCH_GSL3680)
  return true;
#elif defined(TOUCH_FT6336)
  tp = m_ts->scan();
  return tp.touch_count > 0;
#elif defined(TOUCH_CST820)
  return true;
#else
  return false;
#endif
}

void TouchScreen::showMap(const String& axis, int16_t minValue, int16_t maxValue){
  ESP_LOGD(TAG, "Axis: %s (%d - %d)", axis.c_str(), minValue, maxValue);
}

/**
 * @brief Checks if the touch screen has been touched.
 * @return True if the touch screen has been touched, false otherwise.
 */
bool TouchScreen::touch_touched()
{
#if !defined(HAS_TOUCH)
  return false;
#else

#if defined(TOUCH_GSL3680)
  uint16_t touchX, touchY;
  bool touched = m_ts->getTouch(&touchX, &touchY);
  m_touch_last_x = touchX;
  m_touch_last_y = touchY;
  return touched;


#elif defined(TOUCH_GT911) || defined(TOUCH_GT911_WAVESHARE)
  m_ts->read();
  if (m_ts->isTouched)
  {
    m_touch_last_z = -1;

    ESP_LOGD(TAG, "Touch raw: %d, %d", m_ts->points[0].x, m_ts->points[0].y);



    if(!m_swapAxis){

      if(m_invertXAxis){
        m_touch_last_x = map(m_ts->points[0].x, m_x0, m_x1, m_widthScreen - 1, 0);
      }else{
        m_touch_last_x = map(m_ts->points[0].x, m_x0, m_x1, 0, m_widthScreen - 1);
      }

      if(m_invertYAxis){
        m_touch_last_y = map(m_ts->points[0].y, m_y0, m_y1, m_heightScreen - 1, 0);
      }else{
        m_touch_last_y = map(m_ts->points[0].y, m_y0, m_y1, 0, m_heightScreen - 1);
      }
      
    }else{
      if(m_invertXAxis){
        m_touch_last_x = map(m_ts->points[0].y, m_y0, m_y1, m_widthScreen - 1, 0);
      }else{
        m_touch_last_x = map(m_ts->points[0].y, m_y0, m_y1, 0, m_widthScreen - 1);
      }
      

      if(m_invertYAxis){
        m_touch_last_y = map(m_ts->points[0].x, m_x0, m_x1, m_heightScreen - 1, 0);
      }else{
        m_touch_last_y = map(m_ts->points[0].x, m_x0, m_x1, 0, m_heightScreen - 1);
      }
      
    }

    return true;
  }
  return false;

#elif defined(TOUCH_XPT2046)

  if (!m_calibMatrix)
  {
    Serial.println("Calibration not defined");
    return false;
  }

  int16_t xTouch = 0, yTouch = 0, zTouch = 0;
  int16_t touchAmount = 8;
  uint8_t touchDetect = touchAmount;
  // bool validTouch = false;

  while (m_ts->getInput() && touchDetect > 0)
  {
    xTouch += m_ts->x;
    yTouch += m_ts->y;
    zTouch += m_ts->z;
    touchDetect--;
  }
  xTouch /= touchAmount;
  yTouch /= touchAmount;
  zTouch /= touchAmount;

  bool hasTouch = (touchDetect == 0);

  if (m_displayPinCS)
  {
    // delayMicroseconds(5);
    // digitalWrite(DISP_CS, LOW);
    // log_d("Pino %i modo LOW", DISP_CS);
  }

  if (hasTouch)
  {

    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Raw xyz[%i\t%i\t%i]\n", xTouch, yTouch, zTouch);
    }
    ScreenPoint_t toque = touchToTelaPonto0e2(xTouch, yTouch);

    m_touch_last_x = toque.x;
    m_touch_last_y = toque.y;
    m_touch_last_z = zTouch;

    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_FT6236U)
  if (m_ts->touched())
  {
    // Retrieve a point
    TS_Point p = m_ts->getPoint();

    // Print coordinates to the serial output
    // ESP_LOGD(TAG, "X: %i, Y: %i\n", p.x, p.y);
    if (m_swapAxis)
    {
      m_touch_last_x = map(p.y, m_x0, m_x1, 0, m_widthScreen - 1);
      m_touch_last_y = map(p.x, m_y0, m_y1, 0, m_heightScreen - 1);
      m_touch_last_z = 0;
    }
    else
    {
      m_touch_last_x = map(p.x, m_x0, m_x1, 0, m_widthScreen - 1);
      m_touch_last_y = map(p.y, m_y0, m_y1, 0, m_heightScreen - 1);
      m_touch_last_z = 0;
    }

    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_CST816)
  bool touched;
  uint8_t gesture;
  uint16_t touchX, touchY;
  touched = m_ts->available();
  if (touched)
  {
    m_touch_last_x = m_ts->data.x;
    m_touch_last_y = m_ts->data.y;
    m_gesture = m_ts->data.gestureID;
    m_touch_last_z = -1;
  }

  return touched;
#elif defined(TOUCH_FT6336)
  bool touched = tp.touch_count > 0;
  if (!touched)
    return false;



  TouchStatusEnum status0 = (tp.tp[0].status);
  TouchStatusEnum status1 = (tp.tp[1].status);

  uint16_t rx = 0; // raw X do FT6336
  uint16_t ry = 0; // raw Y do FT6336
  if(status0 == TouchStatusEnum::touch || status0 == TouchStatusEnum::stream){
    rx = tp.tp[0].x;
    ry = tp.tp[0].y;
  }else if(status1 == TouchStatusEnum::touch || status1 == TouchStatusEnum::stream){
    rx = tp.tp[1].x;
    ry = tp.tp[1].y;
  }else{
    return false;
  }
  if(m_logMessages){
    //char tempString[128];
    //sprintf(tempString, "FT6336U TD Count %d / TD1 (%d, %4d, %4d) / TD2 (%d, %4d, %4d)\n", tp.touch_count, tp.tp[0].status, tp.tp[0].x, tp.tp[0].y, tp.tp[1].status, tp.tp[1].x, tp.tp[1].y);
    //Serial.print(tempString);


    //char printString[128];
    //sprintf(printString, "Touch value x:%d, y:%d\n", rx, ry);
    //Serial.print(printString);

    /*if(m_invertXAxis){
      Serial.print("Invert X axis\n");
    }
    if(m_invertYAxis){
      Serial.print("Invert Y axis\n");
    }*/
  }
  uint16_t sx, sy;          // coordenadas já convertidas

  int startX = 0;
  int endX = 0;
  int xToMap = 0;

  if(!m_swapAxis){
    xToMap = rx;
    if(!m_invertXAxis){
      startX = m_x0;
      endX = m_x1;
    }else{
      startX = m_x1;
      endX = m_x0; 
    }
  }else{
    xToMap = ry;
    if(!m_invertXAxis){
      startX = m_y0;
      endX = m_y1;
    }else{
      startX = m_y1;
      endX = m_y0; 
    }
  }
  // print mapping Serial.print('X', xToMap, startX, endX, 0, m_widthScreen - 1);
  //char printString[128];
  //sprintf(printString, "'X'\t%d \t%d, \t%d, \t%d, \t%d\n", xToMap, startX, endX, 0, m_widthScreen - 1);
  //Serial.print(printString);




  sx = map(xToMap, startX, endX, 0, m_widthScreen - 1);

  int startY = 0;
  int endY = 0;
  int yToMap = 0;

  if(!m_swapAxis){
    yToMap = ry;
    if(!m_invertYAxis){
      startY = m_y0;
      endY = m_y1;
    }else{
      startY = m_y1;
      endY = m_y0; 
    }
  }else{
    yToMap = rx;
    if(!m_invertYAxis){
      startY = m_x0;
      endY = m_x1;
    }else{
      startY = m_x1;
      endY = m_x0; 
    }
  }

  //char printStringY[128];
  //sprintf(printStringY, "'Y'\t%d \t%d, \t%d, \t%d, \t%d\n", yToMap, startY, endY, 0, m_heightScreen - 1);
  //Serial.print(printStringY);
  sy = map(yToMap, startY, endY, 0, m_heightScreen - 1);
  

  /* Garante que nada extrapole os limites físicos do display */
  m_touch_last_x = constrain(sx, 0, m_widthScreen - 1);
  m_touch_last_y = constrain(sy, 0, m_heightScreen - 1);
  m_touch_last_z = -1; // pressão fixa ou não usada

  return true;

#elif defined(TOUCH_CST820)

  uint16_t touchX, touchY;
  uint8_t gesture;
  bool touched = m_ts->getTouch(&touchX, &touchY, &gesture);

  if(m_swapAxis){
    if(m_invertXAxis){
      m_touch_last_x = map(touchY, m_y0, m_y1, m_widthScreen, 0);
    }else{
      m_touch_last_x = map(touchY, m_y0, m_y1, 0, m_widthScreen);
    }

    if(m_invertYAxis){
      m_touch_last_y = map(touchX, m_x0, m_x1, m_heightScreen, 0);
    }else{
      m_touch_last_y = map(touchX, m_x0, m_x1, 0, m_heightScreen);
    }
  }else{
    if(m_invertXAxis){
      m_touch_last_x = m_x1 - touchX;
    }else{
      m_touch_last_x = touchX;
    }

    if(m_invertYAxis){
      m_touch_last_y = m_y1 - touchY;
    }else{
      m_touch_last_y = touchY;
    }
  }
  m_touch_last_z = -1;
  m_gesture = gesture;


  return touched;
#else
  return false;
#endif
#endif
}

/**
 * @brief Checks if the touch screen has been released.
 * @return True if the touch screen has been released, false otherwise.
 */
bool TouchScreen::touch_released()
{
#if defined(TOUCH_FT6236)
  if (m_touch_released_flag)
  {
    m_touch_released_flag = false;
    return true;
  }
  else
  {
    return false;
  }

#elif defined(TOUCH_GT911) || defined(TOUCH_GT911_WAVESHARE)
  return true;

#elif defined(TOUCH_XPT2046)
  return true;
#elif defined(TOUCH_FT6236U)
  return true;
#elif defined(TOUCH_GSL3680)
  return true;

#else
  return false;
#endif
}

#if defined(TOUCH_FT6236)
/**
 * @brief Handles touch events.
 * @param p Touch point.
 * @param e Touch event type.
 */
void TouchScreen::touchHandler(TPoint p, TEvent e)
{
  if (m_instance)
  {
    m_instance->touch(p, e); // Redireciona para o método de instância
  }
}

#endif

/**
 * @brief Gets the rotation of the touch screen.
 * @return Rotation of the touch screen.
 */
uint8_t TouchScreen::getRotation()
{
  return m_rotation;
}

/**
 * @brief Sets the calibration of the touch screen.
 * @param array Array of calibration points.
 */
#if defined(HAS_TOUCH) && defined(TOUCH_XPT2046)
void TouchScreen::setCalibration(CalibrationPoint_t *array)
{
  delete[] m_calibMatrix;
  if (m_calibMatrix)
  {
    delete m_calibMatrix;
  }
  m_calibMatrix = new CalibrationPoint_t[4];
  memcpy(m_calibMatrix, array, sizeof(array) * sizeof(CalibrationPoint_t));
  // ESP_LOGD(TAG, "Tamanho array %u", sizeof(array) * sizeof(CalibrationPoint_t));

  for (size_t i = 0; i < 5; i++)
  {
    // calibMatrix[i] = array[i];
  }
}
#endif

/**
 * @brief Calibrates the touch screen using a structure.
 * @param points Array of calibration points.
 * @param length Length of the array of calibration points.
 * @param rectScreen Rectangle of the screen.
 * @param color_fg Color of the foreground.
 * @param color_bg Color of the background.
 * @param sizeMarker Size of the marker.
 */
#if defined(HAS_TOUCH) && defined(TOUCH_XPT2046)
void TouchScreen::calibrateTouchStruct(CalibrationPoint_t *points, uint8_t length, Rect_t *rectScreen, uint32_t color_fg, uint32_t color_bg, uint8_t sizeMarker)
{
  // CalibrationPoint_t points[4];

  if (!m_objTFT)
  {
    ESP_LOGD(TAG, "Display not defined");
    return;
  }
  else
  {
    ESP_LOGD(TAG, "Obj display definido");
    m_objTFT->fillScreen(0xffff);
    ESP_LOGD(TAG, "Width: %i", m_objTFT->width());
  }
  // uint16_t x_tmp, y_tmp;

  ESP_LOGD(TAG, "Width: %i, Height: %i", m_widthScreen, m_heightScreen);

  points[0].xScreen = rectScreen->x;
  points[0].yScreen = rectScreen->y;
  points[0].xTouch = 0;
  points[0].yTouch = 0;

  points[1].xScreen = rectScreen->x + rectScreen->width;
  points[1].yScreen = rectScreen->y;
  points[1].xTouch = 0;
  points[1].yTouch = 0;

  points[2].xScreen = rectScreen->x + rectScreen->width;
  points[2].yScreen = rectScreen->y + rectScreen->height;
  points[2].xTouch = 0;
  points[2].yTouch = 0;

  points[3].xScreen = rectScreen->x;
  points[3].yScreen = rectScreen->y + rectScreen->height;
  points[3].xTouch = 0;
  points[3].yTouch = 0;

  uint8_t sizeSquare = 10;

  const char* messageWaiting = "Waiting";
  const char *messageClick = "Click on red square";

  TextBound_t tbWaiting;
  m_objTFT->getTextBounds(messageWaiting, 0, 0, &tbWaiting.x, &tbWaiting.y, &tbWaiting.width, &tbWaiting.height);

  TextBound_t tbClick;
  m_objTFT->getTextBounds(messageClick, 0, 0, &tbClick.x, &tbClick.y, &tbClick.width, &tbClick.height);

  for (auto i = 0; i < length; i++)
  {

    CalibrationPoint_t currentPoint = points[i];

    //m_objTFT->fillCircle(currentPoint.xScreen, currentPoint.yScreen, sizeMarker, color_fg);

    switch(i){
      case 0:
        m_objTFT->fillRect(currentPoint.xScreen, currentPoint.yScreen, sizeSquare, sizeSquare, color_fg);
      break;
      case 1:
        m_objTFT->fillRect(currentPoint.xScreen - sizeSquare, currentPoint.yScreen, sizeSquare, sizeSquare, color_fg);
      break;
      case 2:
        m_objTFT->fillRect(currentPoint.xScreen - sizeSquare, currentPoint.yScreen - sizeSquare, sizeSquare, sizeSquare, color_fg);
      break;
      case 3:
        m_objTFT->fillRect(currentPoint.xScreen, currentPoint.yScreen - sizeSquare, sizeSquare, sizeSquare, color_fg);
      break;
      default:
      break;
    }




    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Calibration point %i: %i, %i\n", i, currentPoint.xScreen, currentPoint.yScreen);
    }

    // drawStar(points[i].xScreen, points[i].yScreen, sizeMarker, color_bg);

    delay(1000);

    // objTFT->fillCircle(widthScreen / 2, heightScreen / 2, 20, 0x07c0);

    const uint16_t margin = 20;

    
    

    m_objTFT->fillRect(rectScreen->x + margin, rectScreen->y + margin, rectScreen->width - 2 * margin, rectScreen->height - 2 * margin, 0xffff);
    m_objTFT->setTextColor(0x0);

    CoordPoint_t pointWaiting = {rectScreen->width / 2 - tbWaiting.width / 2, rectScreen->height / 2 - tbWaiting.height / 2};
    CoordPoint_t pointClick = {rectScreen->width / 2 - tbClick.width / 2, rectScreen->height / 2 - tbClick.height / 2};


    m_objTFT->setCursor(pointWaiting.x, pointWaiting.y);
    m_objTFT->print("Waiting");

    auto lineHeight = (int)round(tbWaiting.height * 1.6);

    m_objTFT->setCursor(pointClick.x, pointClick.y + lineHeight);
    m_objTFT->print("Click on red square");

    const uint8_t captureCount = 8;

    for (uint8_t j = 0; j < captureCount; j++)
    {
      while (!m_ts->getInput())
        ;
      if (m_logMessages)
      {
        ESP_LOGD(TAG, "%i, %i\n", m_ts->x, m_ts->y);
      }
      points[i].xTouch += m_ts->x;
      points[i].yTouch += m_ts->y;
      if (m_logMessages)
      {
        ESP_LOGD(TAG, "{%i\t%i\t%i}\n", m_ts->x, m_ts->y, m_ts->z);
      }
    }
    points[i].xTouch /= captureCount;
    points[i].yTouch /= captureCount;


    m_objTFT->fillCircle(points[i].xScreen, points[i].yScreen, sizeMarker, ~color_bg);



    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Calibrado %i => %i, %i\n", i, points[i].xTouch, points[i].yTouch);
    }
    m_objTFT->setCursor(rectScreen->width / 2, rectScreen->height / 2);
    m_objTFT->fillScreen(0xffff);
    m_objTFT->print("OK");

    delay(2000);
  }

  uint16_t mediaX = 0, mediaMinX = 0, mediaMaxX = 0;
  uint16_t mediaY = 0, mediaMinY = 0, mediaMaxY = 0;

  // Somar o X de todos os pontos, e somar todos os Y tambem
  for (uint8_t i = 0; i < length; i++)
  {
    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Tela: %i x %i = Touch: %i x %i\n", points[i].xScreen, points[i].yScreen, points[i].xTouch, points[i].yTouch);
    }
    mediaX += points[i].xTouch;
    mediaY += points[i].yTouch;
  }

  // Tirar a media para depois encontrar os dois menores e dois maiores
  mediaX /= length;
  mediaY /= length;

  // Para cada ponto, verificar se X é menor ou maior que a media e somar na varivel para depois tirar a media entre os dois menores e dois maiores
  for (uint8_t i = 0; i < length; i++)
  {
    if (points[i].xTouch < mediaX)
    {
      mediaMinX += points[i].xTouch;
    }
    else
    {
      mediaMaxX += points[i].xTouch;
    }

    if (points[i].yTouch < mediaY)
    {
      mediaMinY += points[i].yTouch;
    }
    else
    {
      mediaMaxY += points[i].yTouch;
    }
  }

  // media dos pontos menores e maiores
  mediaMinX /= (length / 2);
  mediaMaxX /= (length / 2);
  mediaMinY /= (length / 2);
  mediaMaxY /= (length / 2);

  // Normatiza os pontos para formar um retangulo medio do touch
  for (uint8_t i = 0; i < length; i++)
  {
    points[i].xTouch = points[i].xTouch < mediaX ? mediaMinX : mediaMaxX;
    points[i].yTouch = points[i].yTouch < mediaY ? mediaMinY : mediaMaxY;
  }

  // Mostra o retangulo touch normatizado
  for (uint8_t i = 0; i < length; i++)
  {
    ESP_LOGD(TAG, "Normatized Tela: %i x %i = Touch: %i x %i", points[i].xScreen, points[i].yScreen, points[i].xTouch, points[i].yTouch);
  }

  ESP_LOGD(TAG, "Terminou");
}
#endif
/**
 * @brief Draws a star on the screen.
 * @param xPos X position of the star.
 * @param yPos Y position of the star.
 * @param size Size of the star.
 * @param color Color of the star.
 */
#if defined(TOUCH_XPT2046) && defined(HAS_TOUCH)

CalibrationPoint_t TouchScreen::getMinPoint(CalibrationPoint_t pontos[4])
{
  CalibrationPoint_t minPoint = pontos[0];

  for (int i = 1; i < 4; ++i)
  {
    if (pontos[i].xTouch <= minPoint.xTouch && pontos[i].yTouch <= minPoint.yTouch)
    {
      minPoint = pontos[i];
    }
  }

  return minPoint;
}

// Retorna o ponto com o maior xTouch e maior yTouch
CalibrationPoint_t TouchScreen::getMaxPoint(CalibrationPoint_t pontos[4])
{
  CalibrationPoint_t maxPoint = pontos[0];

  for (int i = 1; i < 4; ++i)
  {
    if (pontos[i].xTouch >= maxPoint.xTouch && pontos[i].yTouch >= maxPoint.yTouch)
    {
      maxPoint = pontos[i];
    }
  }

  return maxPoint;
}

void TouchScreen::drawStar(int16_t xPos, int16_t yPos, uint8_t size, uint16_t color)
{
  if (m_objTFT)
  {

    m_objTFT->fillCircle(xPos, yPos, size, color);
  }
}
#endif

/**
 * @brief Gets the screen position of the touch screen.
 * @param xTouch X position of the touch.
 * @param yTouch Y position of the touch.
 * @return Screen position of the touch.
 */
#if defined(TOUCH_XPT2046) && defined(HAS_TOUCH)

int TouchScreen::mapTouch(int val, int in_min, int in_max, int out_min, int out_max)
{
  if (in_max == in_min)
    return out_min;
  int result = (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

  if (m_logMessages)
  {
    ESP_LOGD(TAG, "mapTouch: val=%d, in_min=%d, in_max=%d, out_min=%d, out_max=%d -> result=%d",val, in_min, in_max, out_min, out_max, result);
  }

  return result;
}

ScreenPoint_t TouchScreen::touchToTelaPonto0e2(int16_t xTouch, int16_t yTouch)
{
  ScreenPoint_t screenPos = {0, 0};
  if (!m_calibMatrix)
    return screenPos;

  CalibrationPoint_t p0 = m_calibMatrix[0]; // Ponto mais próximo da origem
  CalibrationPoint_t p2 = m_calibMatrix[2]; // Ponto mais distante da origem

  if (m_logMessages)
  {
    ESP_LOGD(TAG, "Ponto 0: (%d,%d) = (%d,%d)", p0.xScreen, p0.yScreen, p0.xTouch, p0.yTouch);
    ESP_LOGD(TAG, "Ponto 2: (%d,%d) = (%d,%d)", p2.xScreen, p2.yScreen, p2.xTouch, p2.yTouch);
  }

  if (m_swapAxis)
  {
    if(m_invertXAxis){
      screenPos.x = mapTouch(yTouch, p0.yTouch, p2.yTouch, p2.xScreen, p0.xScreen);
    }else{
      screenPos.x = mapTouch(yTouch, p0.yTouch, p2.yTouch, p0.xScreen, p2.xScreen);
    }

    if(m_invertYAxis){
      screenPos.y = mapTouch(xTouch, p0.xTouch, p2.xTouch, p2.yScreen, p0.yScreen);
    }else{
      screenPos.y = mapTouch(xTouch, p0.xTouch, p2.xTouch, p0.yScreen, p2.yScreen);
    }
    
    
  }
  else
  {
    if(m_invertXAxis){
      screenPos.x = mapTouch(xTouch, p0.xTouch, p2.xTouch, p2.xScreen, p0.xScreen);
    }else{
      screenPos.x = mapTouch(xTouch, p0.xTouch, p2.xTouch, p0.xScreen, p2.xScreen);
    }

    if(m_invertYAxis){
      screenPos.y = mapTouch(yTouch, p0.yTouch, p2.yTouch, p2.yScreen, p0.yScreen);
    }else{
      screenPos.y = mapTouch(yTouch, p0.yTouch, p2.yTouch, p0.yScreen, p2.yScreen);
    }
    
    
  }

  return screenPos;
}

ScreenPoint_t TouchScreen::getScreenPosition(int16_t xTouch, int16_t yTouch)
{
  ScreenPoint_t screenPos = {0, 0};

  if (!m_calibMatrix)
    return screenPos;
  if (m_logMessages)
  {
    ESP_LOGD(TAG, "Get screen position for xTouch: %i and yTouch: %i\n", xTouch, yTouch);
  }
  CalibrationPoint_t cornerMin = getMinPoint(m_calibMatrix);
  CalibrationPoint_t cornerMax = getMaxPoint(m_calibMatrix);

  if (m_logMessages)
  {
    ESP_LOGD(TAG, "Ponto minimo: %i x %i e ponto maximo: %i x %i\n", cornerMin.xTouch, cornerMin.yTouch, cornerMax.xTouch, cornerMax.yTouch);
  }
  xTouch = constrain(xTouch, cornerMin.xTouch, cornerMax.xTouch);
  yTouch = constrain(yTouch, cornerMin.yTouch, cornerMax.yTouch);

  int16_t xMapFrom, xMapTo, yMapFrom, yMapTo;
  int16_t xMapVal, yMapVal;

  switch (m_rotation)
  {
  case 0:
    xMapVal = xTouch;
    yMapVal = yTouch;

    xMapFrom = m_invertXAxis ? cornerMax.xTouch : cornerMin.xTouch;
    xMapTo = m_invertXAxis ? cornerMin.xTouch : cornerMax.xTouch;

    yMapFrom = m_invertYAxis ? cornerMin.yTouch : cornerMax.yTouch;
    yMapTo = m_invertYAxis ? cornerMax.yTouch : cornerMin.yTouch;
    break;

  case 1:
    xMapVal = yTouch;
    yMapVal = xTouch;

    xMapFrom = m_invertXAxis ? cornerMin.yTouch : cornerMax.yTouch;
    xMapTo = m_invertXAxis ? cornerMax.yTouch : cornerMin.yTouch;

    yMapFrom = m_invertYAxis ? cornerMin.xTouch : cornerMax.xTouch;
    yMapTo = m_invertYAxis ? cornerMax.xTouch : cornerMin.xTouch;
    break;

  case 2:
    xMapVal = xTouch;
    yMapVal = yTouch;

    xMapFrom = m_invertXAxis ? cornerMin.xTouch : cornerMax.xTouch;
    xMapTo = m_invertXAxis ? cornerMax.xTouch : cornerMin.xTouch;

    yMapFrom = m_invertYAxis ? cornerMax.yTouch : cornerMin.yTouch;
    yMapTo = m_invertYAxis ? cornerMin.yTouch : cornerMax.yTouch;
    break;

  case 3:
    xMapVal = yTouch;
    yMapVal = xTouch;

    xMapFrom = m_invertXAxis ? cornerMax.yTouch : cornerMin.yTouch;
    xMapTo = m_invertXAxis ? cornerMin.yTouch : cornerMax.yTouch;

    yMapFrom = m_invertYAxis ? cornerMax.xTouch : cornerMin.xTouch;
    yMapTo = m_invertYAxis ? cornerMin.xTouch : cornerMax.xTouch;
    break;

  default:
    return screenPos;
  }

  screenPos.x = map(xMapVal, xMapFrom, xMapTo, 0, m_widthScreen);
  screenPos.y = map(yMapVal, yMapFrom, yMapTo, 0, m_heightScreen);
  if (m_logMessages)
  {
    ESP_LOGD(TAG, "Mapped x: %i from [%i → %i] to [0 → %i] = %i\n", xMapVal, xMapFrom, xMapTo, m_widthScreen, screenPos.x);
    ESP_LOGD(TAG, "Mapped y: %i from [%i → %i] to [0 → %i] = %i\n", yMapVal, yMapFrom, yMapTo, m_heightScreen, screenPos.y);
  }
  return screenPos;
}
#endif
/**
 * @brief Calibrates the touch screen.
 * @param parameters Array of calibration parameters.
 * @param color_fg Color of the foreground.
 * @param color_bg Color of the background.
 * @param size Size of the calibration points.
 */
#if defined(TOUCH_XPT2046) && defined(HAS_TOUCH)
void TouchScreen::calibrateTouch(uint16_t *parameters, uint32_t color_fg, uint32_t color_bg, uint8_t size)
{
  int16_t values[] = {0, 0, 0, 0, 0, 0, 0, 0};

  if (!m_objTFT)
  {
    ESP_LOGD(TAG, "Display not defined");
    return;
  }
  else
  {
    ESP_LOGD(TAG, "Obj display definido");
    m_objTFT->fillScreen(0xffff);
    ESP_LOGD(TAG, "Width: %i", m_objTFT->width());
  }
  // uint16_t x_tmp, y_tmp;

  for (uint8_t i = 0; i < 4; i++)
  {
    m_objTFT->fillRect(0, 0, size + 1, size + 1, color_bg);
    m_objTFT->fillRect(0, m_heightScreen - size - 1, size + 1, size + 1, color_bg);
    m_objTFT->fillRect(m_widthScreen - size - 1, 0, size + 1, size + 1, color_bg);
    m_objTFT->fillRect(m_widthScreen - size - 1, m_heightScreen - size - 1, size + 1, size + 1, color_bg);

    if (i == 5)
      break; // used to clear the arrows

    switch (i)
    {
    case 0: // up left
      m_objTFT->drawLine(0, 0, 0, size, color_fg);
      m_objTFT->drawLine(0, 0, size, 0, color_fg);
      m_objTFT->drawLine(0, 0, size, size, color_fg);
      break;
    case 1: // bot left
      m_objTFT->drawLine(0, m_heightScreen - size - 1, 0, m_heightScreen - 1, color_fg);
      m_objTFT->drawLine(0, m_heightScreen - 1, size, m_heightScreen - 1, color_fg);
      m_objTFT->drawLine(size, m_heightScreen - size - 1, 0, m_heightScreen - 1, color_fg);
      break;
    case 2: // up right
      m_objTFT->drawLine(m_widthScreen - size - 1, 0, m_widthScreen - 1, 0, color_fg);
      m_objTFT->drawLine(m_widthScreen - size - 1, size, m_widthScreen - 1, 0, color_fg);
      m_objTFT->drawLine(m_widthScreen - 1, size, m_widthScreen - 1, 0, color_fg);
      break;
    case 3: // bot right
      m_objTFT->drawLine(m_widthScreen - size - 1, m_heightScreen - size - 1, m_widthScreen - 1, m_heightScreen - 1, color_fg);
      m_objTFT->drawLine(m_widthScreen - 1, m_heightScreen - 1 - size, m_widthScreen - 1, m_heightScreen - 1, color_fg);
      m_objTFT->drawLine(m_widthScreen - 1 - size, m_heightScreen - 1, m_widthScreen - 1, m_heightScreen - 1, color_fg);
      break;
    }

    // user has to get the chance to release
    if (i > 0)
      delay(1000);

    m_objTFT->fillCircle(m_widthScreen / 2, m_heightScreen / 2, 20, 0x07c0);
    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Calibrando %i\n", i);
    }
    for (uint8_t j = 0; j < 8; j++)
    {
      // Use a lower detect threshold as corners tend to be less sensitive
      while (!m_ts->getInput())
        ;
      if (m_logMessages)
      {
        ESP_LOGD(TAG, "%i, %i\n", m_ts->x, m_ts->y);
      }
      values[i * 2] += m_ts->x;
      values[i * 2 + 1] += m_ts->y;
      if (m_logMessages)
      {
        ESP_LOGD(TAG, "{%i\t%i\t%i}\n", m_ts->x, m_ts->y, m_ts->z);
      }
    }
    values[i * 2] /= 8;
    values[i * 2 + 1] /= 8;
    if (m_logMessages)
    {
      ESP_LOGD(TAG, "Calibrado %i => %i, %i\n", i, values[i * 2], values[i * 2 + 1]);
    }
    m_objTFT->fillCircle(m_widthScreen / 2, m_heightScreen / 2, 20, 0xf800);
    delay(2000);
  }

  // from case 0 to case 1, the y value changed.
  // If the measured delta of the touch x axis is bigger than the delta of the y axis, the touch and TFT axes are switched.
  m_touchCalibration_rotate = false;
  if (abs(values[0] - values[2]) > abs(values[1] - values[3]))
  {
    m_touchCalibration_rotate = true;
    m_touchCalibration_x0 = (values[1] + values[3]) / 2; // calc min x
    m_touchCalibration_x1 = (values[5] + values[7]) / 2; // calc max x
    m_touchCalibration_y0 = (values[0] + values[4]) / 2; // calc min y
    m_touchCalibration_y1 = (values[2] + values[6]) / 2; // calc max y
  }
  else
  {
    m_touchCalibration_x0 = (values[0] + values[2]) / 2; // calc min x
    m_touchCalibration_x1 = (values[4] + values[6]) / 2; // calc max x
    m_touchCalibration_y0 = (values[1] + values[5]) / 2; // calc min y
    m_touchCalibration_y1 = (values[3] + values[7]) / 2; // calc max y
  }

  if (m_logMessages)
  {
    for (auto i = 0; i < 8; ++i)
    {
      Serial.print(values[i]);
      Serial.print("\t");
    }
  }

  // in addition, the touch screen axis could be in the opposite direction of the TFT axis
  m_touchCalibration_invert_x = false;
  if (m_touchCalibration_x0 > m_touchCalibration_x1)
  {
    values[0] = m_touchCalibration_x0;
    m_touchCalibration_x0 = m_touchCalibration_x1;
    m_touchCalibration_x1 = values[0];
    m_touchCalibration_invert_x = true;
  }
  m_touchCalibration_invert_y = false;
  if (m_touchCalibration_y0 > m_touchCalibration_y1)
  {
    values[0] = m_touchCalibration_y0;
    m_touchCalibration_y0 = m_touchCalibration_y1;
    m_touchCalibration_y1 = values[0];
    m_touchCalibration_invert_y = true;
  }

  // pre calculate
  // touchCalibration_x1 -= touchCalibration_x0;
  // touchCalibration_y1 -= touchCalibration_y0;

  if (m_touchCalibration_x0 == 0)
    m_touchCalibration_x0 = 1;
  if (m_touchCalibration_x1 == 0)
    m_touchCalibration_x1 = 1;
  if (m_touchCalibration_y0 == 0)
    m_touchCalibration_y0 = 1;
  if (m_touchCalibration_y1 == 0)
    m_touchCalibration_y1 = 1;

  if (m_logMessages)
  {
    ESP_LOGD(TAG, "Calibrated values");
    ESP_LOGD(TAG, "%i, %i, %i, %i", m_touchCalibration_x0, m_touchCalibration_x1, m_touchCalibration_y0, m_touchCalibration_y1);
    ESP_LOGD(TAG, "-----------------");
  }

  // export parameters, if pointer valid
  if (parameters != NULL)
  {
    parameters[0] = m_touchCalibration_x0;
    parameters[1] = m_touchCalibration_x1;
    parameters[2] = m_touchCalibration_y0;
    parameters[3] = m_touchCalibration_y1;
    parameters[4] = m_touchCalibration_rotate | (m_touchCalibration_invert_x << 1) | (m_touchCalibration_invert_y << 2);
  }
  else
  {
    ESP_LOGD(TAG, "No parameter to calibrate");
  }
}

#endif

/**
 * @brief Gets the gesture of the touch screen.
 * @return Gesture of the touch screen.
 */
uint8_t TouchScreen::getGesture()
{
  return m_gesture;
}

/**
 * @brief Gets the touch of the touch screen.
 * @param xTouch X position of the touch.
 * @param yTouch Y position of the touch.
 * @param zPressure Z position of the touch.
 * @return True if the touch screen has been touched, false otherwise.
 */
bool TouchScreen::getTouch(uint16_t *xTouch, uint16_t *yTouch, int *zPressure)
{

  if (!m_startedTouch)
  {
    return false;
  }

  if (touch_has_signal())
  {
    if (touch_touched())
    {
      (*xTouch) = m_touch_last_x;
      (*yTouch) = m_touch_last_y;
      (*zPressure) = m_touch_last_z;
      ESP_LOGD(TAG, "getTouch: xTouch: %d, yTouch: %d, zPressure: %d", m_touch_last_x, m_touch_last_y, m_touch_last_z);
      return true;
    }
  }
  return false;
}