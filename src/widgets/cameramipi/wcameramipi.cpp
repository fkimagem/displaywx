#include "wcameramipi.h"
#include "../../check_camera.h"
#include <stdio.h>

const char* CameraMipi::TAG = "CameraMipi";
bool CameraMipi::s_hasFullScreen = false;

CameraMipi::CameraMipi(uint16_t _x, uint16_t _y, uint8_t _screen): WidgetBase(_x, _y, _screen),
m_status(false), m_shouldRedraw(true), m_initialized(false), m_enabled(false), m_fullScreen(false), m_margin(0), m_zoomLevel(1), m_centerX(-1), m_centerY(-1), m_keepZoomOnMaximized(false), m_zoomClampNoted(false) {
  m_config = {.width = 0, .height = 0, .showZoomLevel = false, .showFps = false, .fps = 5};
  ESP_LOGD(TAG, "CameraMipi created at (%d, %d) on screen %d", _x, _y, _screen);
}

CameraMipi::~CameraMipi() {
  cleanupMemory();
}

void CameraMipi::cleanupMemory() {
  ESP_LOGD(TAG, "CameraMipi memory cleanup completed");
}

void CameraMipi::setup(const CameraMipiConfig& config) {
  m_config = config;
  if (m_config.fps < 5 || m_config.fps > 24)
  {
    ESP_LOGW(TAG, "camera fps %u clamped to 5..24", (unsigned)m_config.fps);
    if (m_config.fps < 5)
    {
      m_config.fps = 5;
    }
    if (m_config.fps > 24)
    {
      m_config.fps = 24;
    }
  }
  m_shouldRedraw = true;
  m_loaded = true;
  m_initialized = true;
  m_fullScreen = false;
  m_zoomLevel = 1;
  m_centerX = -1;
  m_centerY = -1;
  m_keepZoomOnMaximized = false;
  m_zoomClampNoted = false;
  m_lastRedrawMs = 0;
  m_locked = false;
  m_visible = true;
  m_enabled = true;
  ESP_LOGD(TAG, "CameraMipi setup completed");
}

void CameraMipi::changeState() {
  ESP_LOGD(TAG, "CameraMipi changeState completed");
}

bool CameraMipi::getStatus() {
  return m_status;
}

void CameraMipi::setStatus(bool _status) {
  m_status = _status;
  ESP_LOGD(TAG, "CameraMipi setStatus completed");
}

bool CameraMipi::getEnabled() {
  return m_enabled;
}

void CameraMipi::setEnabled(bool newState) {
  m_enabled = newState;
  ESP_LOGD(TAG, "CameraMipi setEnabled completed");
}

void CameraMipi::show() {
  ESP_LOGD(TAG, "CameraMipi show completed");
}

void CameraMipi::hide() {
  ESP_LOGD(TAG, "CameraMipi hide completed");
}

bool CameraMipi::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_INITIALIZED_BOOL
  CHECK_LOADED_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_DEBOUNCE_LONG_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL

  // Validate touch coordinates
  if (_xTouch == nullptr || _yTouch == nullptr) {
    ESP_LOGW(TAG, "Touch coordinates are null");
    return false;
  }



  bool inBounds = POINT_IN_RECT(*_xTouch, *_yTouch, m_xPos, m_yPos, m_config.width, m_config.height);
  if(inBounds) {
    m_myTime = millis();
    setPressed(true);  // Mark widget as pressed
    changeState();
    m_shouldRedraw = true;
    m_margin = 1;
    ESP_LOGD(TAG, "CameraMipi detectTouch completed INSIDE BOUNDS");
    m_fullScreen = true;
    s_hasFullScreen = true;
    return true;
  }

  ESP_LOGD(TAG, "CameraMipi detectTouch completed OUTSIDE BOUNDS");
  return false;

}

void CameraMipi::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_DEBOUNCE_REDRAW_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  ESP_LOGD(TAG, "CameraMipi redraw completed");
}

void CameraMipi::forceUpdate() {
  ESP_LOGD(TAG, "CameraMipi forceUpdate completed");
  m_shouldRedraw = true;
}

void CameraMipi::drawBackground() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID

  uint16_t borderColor = CFK_RED;
  uint16_t backgroundColor = CFK_BLACK;
  WidgetBase::objTFT->fillRect(m_xPos, m_yPos, m_config.width, m_config.height, backgroundColor);
  WidgetBase::objTFT->drawRect(m_xPos, m_yPos, m_config.width, m_config.height, borderColor);

  ESP_LOGD(TAG, "CameraMipi drawBackground completed");
}

functionCB_t CameraMipi::getCallbackFunc() { return m_callback; }

int16_t CameraMipi::getXPos() const
{
  return m_xPos;
}

int16_t CameraMipi::getYPos() const
{
  return m_yPos;
}

uint16_t CameraMipi::getWidth() const
{
  return m_config.width;
}

uint16_t CameraMipi::getHeight() const
{
  return m_config.height;
}

bool CameraMipi::isFullScreen() const
{
  return m_fullScreen;
}

bool CameraMipi::hasFullScreen()
{
  return s_hasFullScreen;
}

void CameraMipi::exitFullScreen()
{
  s_hasFullScreen = false;
}

void CameraMipi::leaveFullScreen()
{
  m_fullScreen = false;
}

void CameraMipi::setZoomLevel(uint8_t level)
{
  if (level == 0)
  {
    ESP_LOGW(TAG, "CameraMipi zoom must be >= 1");
    return;
  }
  if (m_zoomLevel != level)
  {
    m_zoomClampNoted = false;
  }
  m_zoomLevel = level;
}

uint8_t CameraMipi::getZoomLevel() const
{
  return m_zoomLevel;
}

bool CameraMipi::showZoomLevel() const
{
  return m_config.showZoomLevel;
}

bool CameraMipi::showFps() const
{
  return m_config.showFps;
}

uint8_t CameraMipi::fps() const
{
  return m_config.fps;
}

bool CameraMipi::isRedrawDue(uint32_t nowMs) const
{
  if (m_lastRedrawMs == 0)
  {
    return true;
  }
  const uint32_t periodMs = 1000u / m_config.fps;
  return (nowMs - m_lastRedrawMs) >= periodMs;
}

void CameraMipi::noteRedraw(uint32_t nowMs)
{
  m_lastRedrawMs = nowMs;
}

const char *CameraMipi::zoomLabel() const
{
  snprintf(m_zoomText, sizeof(m_zoomText), "%ux", (unsigned)m_zoomLevel);
  return m_zoomText;
}

int16_t CameraMipi::centerPointX() const
{
  return m_centerX;
}

int16_t CameraMipi::centerPointY() const
{
  return m_centerY;
}

void CameraMipi::setCenterPoint(int16_t x, int16_t y)
{
  m_centerX = x;
  m_centerY = y;
}

bool CameraMipi::keepZoomOnMaximized() const
{
  return m_keepZoomOnMaximized;
}

void CameraMipi::setKeepZoomOnMaximized(bool keep)
{
  m_keepZoomOnMaximized = keep;
}

void CameraMipi::noteZoomClamped(uint8_t used)
{
  if (m_zoomClampNoted || used == m_zoomLevel)
  {
    return;
  }
  m_zoomClampNoted = true;
  ESP_LOGW(TAG, "zoom %u reduced to %u to stay inside the PPA scale", (unsigned)m_zoomLevel, (unsigned)used);
}

#if !defined(HAS_CAMERA_MIPI) || !defined(DFK_CAMERA_MIPI)
bool CameraMipi::openConfig()
{
  ESP_LOGW(TAG, "camera config needs a MIPI sensor");
  return false;
}
#endif

uint16_t CameraMipi::getXPlot() const
{
  return m_xPos + m_margin;
}

uint16_t CameraMipi::getYPlot() const
{
  return m_yPos + m_margin;
}

uint16_t CameraMipi::getWidthPlot() const
{
  return m_config.width - 2 * m_margin;
}

uint16_t CameraMipi::getHeightPlot() const
{
  return m_config.height - 2 * m_margin;
}