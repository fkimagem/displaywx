#include "wlabel.h"

const char* Label::TAG = "Label";

Label::Label(uint16_t _x, uint16_t _y, uint8_t _screen)
  : WidgetBase(_x, _y, _screen)
{
  m_text[0] = '\0';
  m_lastText[0] = '\0';
  m_value[0] = '\0';
  m_prefix[0] = '\0';
  m_suffix[0] = '\0';
}

bool Label::detectTouch(uint16_t *, uint16_t *)
{
  return false;
}

functionCB_t Label::getCallbackFunc()
{
  return m_callback;
}

void Label::setPrefix(const char* str)
{
  strncpy(m_prefix, str ? str : "", LABEL_MAX_PREFIX_LENGTH - 1);
  m_prefix[LABEL_MAX_PREFIX_LENGTH - 1] = '\0';
  if (m_loaded) buildFinalText(m_value);
}

void Label::setSuffix(const char* str)
{
  strncpy(m_suffix, str ? str : "", LABEL_MAX_SUFFIX_LENGTH - 1);
  m_suffix[LABEL_MAX_SUFFIX_LENGTH - 1] = '\0';
  if (m_loaded) buildFinalText(m_value);
}

void Label::buildFinalText(const char* coreText)
{
  if (!coreText) return;
  if (coreText != m_value) {
    strncpy(m_value, coreText, LABEL_MAX_TEXT_LENGTH - 1);
    m_value[LABEL_MAX_TEXT_LENGTH - 1] = '\0';
  }

  m_text[0] = '\0';

  strncat(m_text, m_prefix, LABEL_MAX_TEXT_LENGTH - 1);
  strncat(m_text, m_value,
          LABEL_MAX_TEXT_LENGTH - strlen(m_text) - 1);
  strncat(m_text, m_suffix,
          LABEL_MAX_TEXT_LENGTH - strlen(m_text) - 1);

}

void Label::setText(const char* str)
{
  CHECK_LOADED_VOID
  if (!str) return;

  buildFinalText(str);
}

void Label::setText(const String& str)
{
  setText(str.c_str());
}

void Label::setTextFloat(float value, uint8_t decimalPlaces)
{
  setDecimalPlaces(decimalPlaces);

  char buffer[20];
  dtostrf(value, 1, m_decimalPlaces, buffer);
  buildFinalText(buffer);
}

void Label::setTextInt(int value)
{
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", value);
  buildFinalText(buffer);
}

void Label::redraw()
{
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  #if defined(USING_GRAPHIC_LIB)
  CHECK_CURRENTSCREEN_VOID
  CHECK_LOADED_VOID
  // Compare with what was actually drawn, including prefix and suffix. Pending
  // changes that return to the displayed value do not need another draw.
  if (!m_shouldRedraw && strcmp(m_text, m_lastText) == 0) return;

  WidgetBase::objTFT->setTextColor(m_config.fontColor);
  WidgetBase::objTFT->setFont(m_config.fontFamily);
  WidgetBase::objTFT->setTextSize(m_fontSize);
  WidgetBase::objTFT->setTextWrap(false);

  TextBound_t area = {0, 0, 0, 0};
  uint16_t textX = m_xPos, textY = m_yPos;

  if (m_config.textMetrics) {
    const auto box = EdgewxText::measure(m_text, *m_config.textMetrics, m_fontSize);
    const uint8_t datum = m_config.backgroundDatum;
    const int horizontal = datum % 3, vertical = datum / 3;
    const float left = m_config.backgroundAnchorX - horizontal * box.width / 2.0f;
    const float top = m_config.backgroundAnchorY - vertical * box.height / 2.0f;
    area = {int16_t(lroundf(left)), int16_t(lroundf(top)), uint16_t(box.width), uint16_t(box.height)};
    textY = int16_t(lroundf(top + box.baseline));
  }
  WidgetBase::recalculateTextPosition(m_text, &textX, &textY, m_config.datum);
  if (!m_config.textMetrics)
    WidgetBase::objTFT->getTextBounds(m_text, textX, textY, &area.x, &area.y, &area.width, &area.height);

  // Always erase the entire previous rectangle, even when the new value is
  // shorter or uses a different height. The configured color matches the
  // object behind this label; no page redraw is needed.
  const bool hadArea = m_lastArea.width && m_lastArea.height;
  if (hadArea)
    WidgetBase::objTFT->fillRect(m_lastArea.x, m_lastArea.y, m_lastArea.width, m_lastArea.height, m_config.backgroundColor);
  const bool sameArea = hadArea && area.x == m_lastArea.x && area.y == m_lastArea.y
      && area.width == m_lastArea.width && area.height == m_lastArea.height;
  if (m_config.fillBackground && !sameArea)
    WidgetBase::objTFT->fillRect(area.x, area.y, area.width, area.height, m_config.backgroundColor);
  WidgetBase::objTFT->setCursor(textX, textY);
  WidgetBase::objTFT->print(m_text);

  WidgetBase::objTFT->setTextSize(1);
  WidgetBase::setFontNull();

  m_lastArea = area;
  strcpy(m_lastText, m_text);
  m_shouldRedraw = false;
  #endif
}

void Label::forceUpdate()
{
  m_shouldRedraw = true;
}

void Label::setDecimalPlaces(uint8_t places)
{
  m_decimalPlaces = constrain(places, 0, 5);
}

void Label::setFontSize(uint8_t newSize)
{
  const uint8_t size = newSize ? newSize : 1;
  if (m_fontSize == size) return;
  m_fontSize = size;
  m_shouldRedraw = true;
}

void Label::invalidateBackground()
{
  m_lastArea = {0, 0, 0, 0};
  m_shouldRedraw = true;
}

void Label::setup(const LabelConfig& config)
{
  CHECK_TFT_VOID
  if (m_loaded) return;

  m_config = config;

  setPrefix(config.prefix);
  setSuffix(config.suffix);
  buildFinalText(config.text ? config.text : "");

  #if defined(USING_GRAPHIC_LIB)
  m_config.fontFamily = config.fontFamily;
  #endif

  m_lastArea = {0, 0, 0, 0};
  m_loaded = true;
  m_shouldRedraw = true;
}

void Label::show()
{
  m_visible = true;
  m_shouldRedraw = true;
}

void Label::hide()
{
  m_visible = false;
  m_shouldRedraw = true;
}
