#include "cameraroi.h"
#include "../../displayfk.h"
#include <string.h>

#if defined(HAS_CAMERA_MIPI) && defined(DFK_CAMERA_MIPI)

const char *CameraRoi::TAG = "CameraRoi";

CameraRoi::CameraRoi() : WidgetBase(0, 0, 0) {}

CameraRoi::~CameraRoi() {}

bool CameraRoi::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  (void)_xTouch;
  (void)_yTouch;
  return false;
}

functionCB_t CameraRoi::getCallbackFunc()
{
  return nullptr;
}

void CameraRoi::forceUpdate() {}

void CameraRoi::show() {}

void CameraRoi::hide() {}

int16_t CameraRoi::centerX() const
{
  return m_centerX;
}

int16_t CameraRoi::centerY() const
{
  return m_centerY;
}

uint8_t CameraRoi::zoom() const
{
  return m_zoom;
}

bool CameraRoi::keepZoom() const
{
  return m_keepZoom;
}

CameraMipi *CameraRoi::widget() const
{
  return m_widget;
}

void CameraRoi::open(CameraMipi *widget, uint32_t frameW, uint32_t frameH)
{
  m_widget = widget;
  m_frameW = frameW;
  m_frameH = frameH;
  m_centerX = widget != nullptr ? widget->centerPointX() : -1;
  m_centerY = widget != nullptr ? widget->centerPointY() : -1;
  if (m_centerX < 0)
  {
    m_centerX = (int16_t)(frameW / 2u);
  }
  if (m_centerY < 0)
  {
    m_centerY = (int16_t)(frameH / 2u);
  }
  m_zoom = widget != nullptr ? widget->getZoomLevel() : 1;
  if (m_zoom < 1)
  {
    m_zoom = 1;
  }
  m_keepZoom = widget != nullptr && widget->keepZoomOnMaximized();
  m_plotW = 2;
  m_plotH = 2;
  if (widget != nullptr)
  {
    m_plotW = widget->getWidthPlot();
    m_plotH = widget->getHeightPlot();
    if (m_plotW < 2)
    {
      m_plotW = widget->getWidth();
    }
    if (m_plotH < 2)
    {
      m_plotH = widget->getHeight();
    }
  }
  clampDraft();
  redraw();
}

void CameraRoi::close()
{
  m_widget = nullptr;
}

void CameraRoi::layout()
{
  m_keyCount = 0;
  if (WidgetBase::objTFT == nullptr)
  {
    return;
  }
  m_screenW = WidgetBase::objTFT->width();
  m_screenH = WidgetBase::objTFT->height();
  const bool narrow = m_screenW < 420;
  const int16_t rowH = 36;
  const int16_t gap = 4;
  m_stripH = narrow ? (int16_t)(rowH * 3 + gap * 4) : (int16_t)(rowH * 2 + gap * 3);
  if (m_screenH < m_stripH + 40)
  {
    m_stripH = m_screenH / 3;
  }
  if (m_stripH < rowH)
  {
    m_stripH = rowH;
  }
  m_previewH = m_screenH - m_stripH;
  if (m_previewH < 2)
  {
    m_previewH = 2;
    m_stripH = m_screenH - m_previewH;
  }

  const int16_t stripY = m_previewH;
  const int16_t rowCount = narrow ? 6 : 8;
  int16_t keyW = (m_screenW - gap * (rowCount + 1)) / rowCount;
  if (keyW < 8)
  {
    keyW = 8;
  }
  int16_t x = gap;
  int16_t y = (int16_t)(stripY + gap);
  const CameraRoiKey topKeys[8] = {
      CameraRoiKey::ZoomOut, CameraRoiKey::ZoomIn, CameraRoiKey::Up, CameraRoiKey::Down,
      CameraRoiKey::Left, CameraRoiKey::Right, CameraRoiKey::Ok, CameraRoiKey::Cancel};
  const int16_t topCount = narrow ? 6 : 8;
  for (int16_t index = 0; index < topCount; index++)
  {
    addKey(x, y, keyW, rowH, topKeys[index]);
    x = (int16_t)(x + keyW + gap);
  }

  y = (int16_t)(y + rowH + gap);
  addKey(gap, y, (int16_t)(m_screenW - gap * 2), rowH, CameraRoiKey::KeepZoom);
  if (!narrow)
  {
    return;
  }
  y = (int16_t)(y + rowH + gap);
  const int16_t half = (m_screenW - gap * 3) / 2;
  addKey(gap, y, half, rowH, CameraRoiKey::Ok);
  addKey((int16_t)(gap * 2 + half), y, half, rowH, CameraRoiKey::Cancel);
}

void CameraRoi::addKey(int16_t x, int16_t y, int16_t w, int16_t h, CameraRoiKey key)
{
  if (m_keyCount >= kMaxKeys || w < 2 || h < 2)
  {
    return;
  }
  KeyBox &box = m_keys[m_keyCount];
  box.x = x;
  box.y = y;
  box.w = w;
  box.h = h;
  box.key = key;
  m_keyCount++;
}

const char *CameraRoi::keyLabel(CameraRoiKey key) const
{
  if (key == CameraRoiKey::ZoomOut)
  {
    return "-";
  }
  if (key == CameraRoiKey::ZoomIn)
  {
    return "+";
  }
  if (key == CameraRoiKey::Up)
  {
    return "^";
  }
  if (key == CameraRoiKey::Down)
  {
    return "v";
  }
  if (key == CameraRoiKey::Left)
  {
    return "<";
  }
  if (key == CameraRoiKey::Right)
  {
    return ">";
  }
  if (key == CameraRoiKey::Ok)
  {
    return "OK";
  }
  if (key == CameraRoiKey::Cancel)
  {
    return "ESC";
  }
  return "";
}

void CameraRoi::drawKey(const KeyBox &key)
{
  uint16_t background = CFK_GREY8;
  if (key.key == CameraRoiKey::Ok)
  {
    background = CFK_GREEN;
  }
  if (key.key == CameraRoiKey::Cancel)
  {
    background = CFK_RED;
  }
  WidgetBase::objTFT->fillRect(key.x, key.y, key.w, key.h, background);
  WidgetBase::objTFT->drawRect(key.x, key.y, key.w, key.h, CFK_WHITE);
  WidgetBase::setFontNull();
  WidgetBase::objTFT->setTextSize(1);
  WidgetBase::objTFT->setTextColor(CFK_WHITE);

  if (key.key == CameraRoiKey::KeepZoom)
  {
    const int16_t box = 16;
    const int16_t boxX = (int16_t)(key.x + 6);
    const int16_t boxY = (int16_t)(key.y + (key.h - box) / 2);
    WidgetBase::objTFT->drawRect(boxX, boxY, box, box, CFK_WHITE);
    if (m_keepZoom)
    {
      WidgetBase::objTFT->fillRect((int16_t)(boxX + 3), (int16_t)(boxY + 3), (int16_t)(box - 6), (int16_t)(box - 6), CFK_WHITE);
    }
    WidgetBase::objTFT->setCursor((int16_t)(boxX + box + 6), (int16_t)(key.y + (key.h - 8) / 2));
    WidgetBase::objTFT->print("keep zoom on maximized");
    return;
  }

  const char *label = keyLabel(key.key);
  const int16_t textW = (int16_t)(strlen(label) * 6);
  int16_t textX = (int16_t)(key.x + (key.w - textW) / 2);
  if (textX < key.x)
  {
    textX = key.x;
  }
  WidgetBase::objTFT->setCursor(textX, (int16_t)(key.y + (key.h - 8) / 2));
  WidgetBase::objTFT->print(label);
}

void CameraRoi::drawKeys()
{
  if (WidgetBase::objTFT == nullptr)
  {
    return;
  }
  WidgetBase::objTFT->fillRect(0, m_previewH, m_screenW, m_stripH, CFK_BLACK);
  for (uint8_t index = 0; index < m_keyCount; index++)
  {
    drawKey(m_keys[index]);
  }
}

CameraRoiKey CameraRoi::keyAt(uint16_t xTouch, uint16_t yTouch) const
{
  for (uint8_t index = 0; index < m_keyCount; index++)
  {
    const KeyBox &key = m_keys[index];
    const bool inside = xTouch >= key.x && xTouch < (uint16_t)(key.x + key.w) && yTouch >= key.y && yTouch < (uint16_t)(key.y + key.h);
    if (inside)
    {
      return key.key;
    }
  }
  return CameraRoiKey::None;
}

void CameraRoi::pan(int16_t directionX, int16_t directionY)
{
  uint32_t cropW = 2;
  uint32_t cropH = 2;
  int16_t centerX = m_centerX;
  int16_t centerY = m_centerY;
  if (DisplayFK::instance != nullptr)
  {
    DisplayFK::instance->clampCameraRoiCenter(m_frameW, m_frameH, m_plotW, m_plotH, &centerX, &centerY, m_zoom, &cropW, &cropH);
  }
  uint32_t stepX = cropW / 16u;
  uint32_t stepY = cropH / 16u;
  if (stepX < 2u)
  {
    stepX = 2u;
  }
  if (stepY < 2u)
  {
    stepY = 2u;
  }
  if ((stepX & 1u) != 0u)
  {
    stepX++;
  }
  if ((stepY & 1u) != 0u)
  {
    stepY++;
  }
  m_centerX = (int16_t)(m_centerX + directionX * (int16_t)stepX);
  m_centerY = (int16_t)(m_centerY + directionY * (int16_t)stepY);
}

void CameraRoi::clampDraft()
{
  if (DisplayFK::instance == nullptr)
  {
    return;
  }
  uint32_t cropW = 0;
  uint32_t cropH = 0;
  DisplayFK::instance->clampCameraRoiCenter(m_frameW, m_frameH, m_plotW, m_plotH, &m_centerX, &m_centerY, m_zoom, &cropW, &cropH);
}

void CameraRoi::redraw()
{
  layout();
  CameraConfigView view;
  if (DisplayFK::instance != nullptr)
  {
    DisplayFK::instance->drawFrozenCamera(m_widget, m_centerX, m_centerY, m_zoom, 0, 0, m_screenW, m_previewH, &view);
  }
  if (WidgetBase::objTFT != nullptr && view.roiW > 2 && view.roiH > 2)
  {
    WidgetBase::objTFT->drawRect(view.roiX, view.roiY, view.roiW, view.roiH, CFK_YELLOW);
    WidgetBase::objTFT->drawRect((int16_t)(view.roiX + 1), (int16_t)(view.roiY + 1), (uint16_t)(view.roiW - 2), (uint16_t)(view.roiH - 2), CFK_YELLOW);
  }
  drawKeys();
}

CameraRoiAction CameraRoi::handlePress(uint16_t xTouch, uint16_t yTouch)
{
  layout();
  const CameraRoiKey key = keyAt(xTouch, yTouch);
  if (key == CameraRoiKey::None)
  {
    return CameraRoiAction::None;
  }
  if (key == CameraRoiKey::Ok)
  {
    return CameraRoiAction::Confirm;
  }
  if (key == CameraRoiKey::Cancel)
  {
    return CameraRoiAction::Cancel;
  }
  if (key == CameraRoiKey::ZoomIn && m_zoom < 255)
  {
    m_zoom++;
  }
  if (key == CameraRoiKey::ZoomOut && m_zoom > 1)
  {
    m_zoom--;
  }
  if (key == CameraRoiKey::KeepZoom)
  {
    m_keepZoom = !m_keepZoom;
  }
  if (key == CameraRoiKey::Left)
  {
    pan(-1, 0);
  }
  if (key == CameraRoiKey::Right)
  {
    pan(1, 0);
  }
  if (key == CameraRoiKey::Up)
  {
    pan(0, -1);
  }
  if (key == CameraRoiKey::Down)
  {
    pan(0, 1);
  }
  clampDraft();
  redraw();
  return CameraRoiAction::Changed;
}

#else

const char *CameraRoi::TAG = "CameraRoi";

CameraRoi::CameraRoi() : WidgetBase(0, 0, 0) {}

CameraRoi::~CameraRoi() {}

bool CameraRoi::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  (void)_xTouch;
  (void)_yTouch;
  return false;
}

functionCB_t CameraRoi::getCallbackFunc()
{
  return nullptr;
}

void CameraRoi::redraw() {}

void CameraRoi::forceUpdate() {}

void CameraRoi::show() {}

void CameraRoi::hide() {}

void CameraRoi::open(CameraMipi *widget, uint32_t frameW, uint32_t frameH)
{
  (void)widget;
  (void)frameW;
  (void)frameH;
}

void CameraRoi::close() {}

CameraRoiAction CameraRoi::handlePress(uint16_t xTouch, uint16_t yTouch)
{
  (void)xTouch;
  (void)yTouch;
  return CameraRoiAction::None;
}

int16_t CameraRoi::centerX() const { return 0; }
int16_t CameraRoi::centerY() const { return 0; }
uint8_t CameraRoi::zoom() const { return 1; }
bool CameraRoi::keepZoom() const { return false; }
CameraMipi *CameraRoi::widget() const { return nullptr; }

#endif
