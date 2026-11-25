#ifndef WLABEL
#define WLABEL

#include "../widgetbase.h"
#include "labeltextmetrics.h"

#define LABEL_MAX_TEXT_LENGTH 64
#define LABEL_MAX_PREFIX_LENGTH 32
#define LABEL_MAX_SUFFIX_LENGTH 32

struct LabelConfig {
  const char* text;
  const char* prefix;
  const char* suffix;
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* fontFamily;
  #endif
  uint16_t datum;
  uint16_t fontColor;
  uint16_t backgroundColor;
  // Optional editor layout. Old sketches retain their existing Label behavior.
  const EdgewxText::Metrics* textMetrics = nullptr;
  uint8_t backgroundDatum = TL_DATUM;
  float backgroundAnchorX = 0, backgroundAnchorY = 0;
  bool fillBackground = true;
  // Retained for source compatibility; Label updates never reload the page.
  functionLoadScreen_t restoreBackground = nullptr;
};

class Label : public WidgetBase
{
public:
  Label(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~Label() = default;

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;

  void setText(const char* str);
  void setText(const String& str);
  void setPrefix(const char* str);
  void setSuffix(const char* str);
  void setTextFloat(float value, uint8_t decimalPlaces = 2);
  void setTextInt(int value);

  void redraw() override;
  void forceUpdate() override;
  void setDecimalPlaces(uint8_t places);
  void setFontSize(uint8_t newSize);
  void setup(const LabelConfig& config);
  void show() override;
  void hide() override;
  void invalidateBackground();

private:
  static const char* TAG;

  char m_text[LABEL_MAX_TEXT_LENGTH];
  char m_lastText[LABEL_MAX_TEXT_LENGTH];
  char m_value[LABEL_MAX_TEXT_LENGTH];
  char m_prefix[LABEL_MAX_PREFIX_LENGTH];
  char m_suffix[LABEL_MAX_SUFFIX_LENGTH];

  bool m_shouldRedraw = false;
  TextBound_t m_lastArea = {0, 0, 0, 0};

  uint8_t m_fontSize = 1;
  uint8_t m_decimalPlaces = 1;

  LabelConfig m_config;

  void buildFinalText(const char* coreText);
};

#endif
