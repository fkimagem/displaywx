#ifndef CAMERAROI_H
#define CAMERAROI_H

#include "wcameramipi.h"

/// Retângulo da ROI na tela de configuração, em coordenadas do display.
struct CameraConfigView {
  int16_t roiX = 0;
  int16_t roiY = 0;
  uint16_t roiW = 0;
  uint16_t roiH = 0;
};

enum class CameraRoiAction : uint8_t {
  None,
  Changed,
  Confirm,
  Cancel
};

enum class CameraRoiKey : uint8_t {
  None,
  ZoomOut,
  ZoomIn,
  Up,
  Down,
  Left,
  Right,
  KeepZoom,
  Ok,
  Cancel
};

/// Tela modal de enquadramento da câmera. O desenho fica nesta classe.
class CameraRoi : public WidgetBase {
public:
  CameraRoi();
  ~CameraRoi();

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  void show() override;
  void hide() override;

  void open(CameraMipi *widget, uint32_t frameW, uint32_t frameH);
  void close();
  CameraRoiAction handlePress(uint16_t xTouch, uint16_t yTouch);

  int16_t centerX() const;
  int16_t centerY() const;
  uint8_t zoom() const;
  bool keepZoom() const;
  CameraMipi *widget() const;

private:
  static const char *TAG;
  static const uint8_t kMaxKeys = 9;

  struct KeyBox {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    CameraRoiKey key;
  };

  CameraMipi *m_widget = nullptr;
  int16_t m_centerX = -1;
  int16_t m_centerY = -1;
  uint8_t m_zoom = 1;
  bool m_keepZoom = false;
  uint32_t m_frameW = 0;
  uint32_t m_frameH = 0;
  uint32_t m_plotW = 2;
  uint32_t m_plotH = 2;
  int16_t m_screenW = 0;
  int16_t m_screenH = 0;
  int16_t m_previewH = 0;
  int16_t m_stripH = 0;
  KeyBox m_keys[kMaxKeys];
  uint8_t m_keyCount = 0;

  void layout();
  void addKey(int16_t x, int16_t y, int16_t w, int16_t h, CameraRoiKey key);
  void drawKeys();
  void drawKey(const KeyBox &key);
  CameraRoiKey keyAt(uint16_t xTouch, uint16_t yTouch) const;
  const char *keyLabel(CameraRoiKey key) const;
  void pan(int16_t directionX, int16_t directionY);
  void clampDraft();
};

#endif
