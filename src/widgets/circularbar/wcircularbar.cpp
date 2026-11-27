#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_CIRCULARBAR)
#include "wcircularbar.h"
#include <esp_log.h>
#include <cmath>

const char *CircularBar::TAG = "CircularBar";

// ------------------------- Helpers Estáticos -------------------------

// Normaliza ângulo para o intervalo [0, 360). 360 vira 0.
static inline int normAngle(int a) {
  a %= 360;
  if (a < 0) a += 360;
  return a;
}

// Calcula o tamanho do arco no sentido horário.
static inline int getClockwiseSpan(int start, int end) {
  start = normAngle(start);
  end = normAngle(end);
  if (start == end) return 360;
  return (end > start) ? (end - start) : (360 - start + end);
}

// Desenha arco lidando com a virada dos 360 graus (wrap-around)
#if defined(DFK_HAS_FILL_ARC)
static void drawArcWrap(Arduino_GFX *tft, int x, int y, int rOut, int rIn, int aStart, int aEnd, uint16_t color) {
  aStart = normAngle(aStart);
  aEnd = normAngle(aEnd);

  if (aStart < aEnd) {
    tft->fillArc(x, y, rOut, rIn, (float)aStart, (float)aEnd, color);
  } else {
    // Quebra em dois desenhos para cruzar o ponto 0/360
    tft->fillArc(x, y, rOut, rIn, (float)aStart, 360.0f, color);
    tft->fillArc(x, y, rOut, rIn, 0.0f, (float)aEnd, color);
  }
}

#endif

// ------------------------- Implementação CircularBar -------------------------

CircularBar::CircularBar(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_lastValue(0), m_value(0) {
  m_config = {0, 100, 0, 0, 360, 0, 0, 0, 0, 10, true, false};
}

CircularBar::~CircularBar() {
  if (m_callback != nullptr) m_callback = nullptr;
}

bool CircularBar::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  WidgetStateGuard widgetStateGuard;
  return false; 
}

functionCB_t CircularBar::getCallbackFunc() {
  WidgetStateGuard widgetStateGuard; return m_callback; }


void CircularBar::drawBackground() {
  WidgetStateGuard widgetStateGuard;
#if defined(DFK_HAS_FILL_ARC)
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_LOADED_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID

  int rOut = m_config.radius;
  int rIn = rOut - m_config.thickness;
  
  // Desenha o fundo completo (setores vazios)
  drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut + 1, rIn - 1, 
              m_config.startAngle, m_config.endAngle, m_config.backgroundColor);
  
  m_lastValue = m_config.minValue; // Reseta referência de desenho
  m_shouldRedraw = true;
  redraw();
#endif
}

void CircularBar::sortValues() {
  WidgetStateGuard widgetStateGuard;
  if (m_config.minValue > m_config.maxValue) {
    std::swap(m_config.minValue, m_config.maxValue);
  }
}

void CircularBar::setScale(int newMinValue, int newMaxValue) {
  WidgetStateGuard widgetStateGuard;
  m_config.minValue = newMinValue;
  m_config.maxValue = newMaxValue;
  sortValues();
  m_value = constrain(m_value, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}

void CircularBar::setMinValue(int newValue) {
  WidgetStateGuard widgetStateGuard;
  m_config.minValue = newValue;
  sortValues();
  m_changedScale = true;
  m_shouldRedraw = true;
}

void CircularBar::setMaxValue(int newValue) {
  WidgetStateGuard widgetStateGuard;
  m_config.maxValue = newValue;
  sortValues();
  m_changedScale = true;
  m_shouldRedraw = true;
}

int CircularBar::getMinValue() {
  WidgetStateGuard widgetStateGuard; return m_config.minValue; }
int CircularBar::getMaxValue() {
  WidgetStateGuard widgetStateGuard; return m_config.maxValue; }

void CircularBar::setValue(int newValue) {
  WidgetStateGuard widgetStateGuard;
  int constrained = constrain(newValue, m_config.minValue, m_config.maxValue);
  if (m_value != constrained) {
    m_value = constrained;
    m_shouldRedraw = true;
  }
}

void CircularBar::redraw() {
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_SHOULDREDRAW_VOID

#if defined(DFK_HAS_FILL_ARC)
  CHECK_DEBOUNCE_REDRAW_VOID
  m_shouldRedraw = false;

  int rOut = m_config.radius;
  int rIn = rOut - m_config.thickness;
  int startA = normAngle(m_config.startAngle);
  int fullSpan = getClockwiseSpan(m_config.startAngle, m_config.endAngle);

  // Se a escala mudou, limpa o widget para o estado inicial
  if (m_changedScale) {
    drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut, rIn, 
                m_config.startAngle, m_config.endAngle, m_config.backgroundColor);
    m_lastValue = m_config.minValue;
    m_changedScale = false;
  }

  // Calcula proporções (0.0 a 1.0)
  float range = (float)(m_config.maxValue - m_config.minValue);
  if (range <= 0) range = 1.0f;

  float pOld = (float)(m_lastValue - m_config.minValue) / range;
  float pNew = (float)(m_value - m_config.minValue) / range;

  // Converte proporção em deslocamento angular (offset do startAngle)
  int arcOld = (int)(pOld * fullSpan);
  int arcNew = (int)(pNew * fullSpan);

  // Determina o que desenhar (Diferença)
  if (arcNew != arcOld) {
    if (!m_config.inverted) {
      if (arcNew > arcOld) {
        // Aumentou: Desenha de arcOld até arcNew com a cor principal
        drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut, rIn, 
                    startA + arcOld, startA + arcNew, m_config.color);
      } else {
        // Diminuiu: Apaga de arcNew até arcOld com a cor de fundo
        drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut, rIn, 
                    startA + arcNew, startA + arcOld, m_config.backgroundColor);
      }
    } else {
      // Lógica Invertida (preenche do fim para o começo)
      int offsetOld = fullSpan - arcOld;
      int offsetNew = fullSpan - arcNew;
      if (arcNew > arcOld) {
        // Aumentou valor: diminui buraco. Desenha de arcNew invertido até arcOld invertido
        drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut, rIn, 
                    startA + offsetNew, startA + offsetOld, m_config.color);
      } else {
        // Diminuiu valor: aumenta buraco. Apaga de arcOld invertido até arcNew invertido
        drawArcWrap(WidgetBase::objTFT, m_xPos, m_yPos, rOut, rIn, 
                    startA + offsetOld, startA + offsetNew, m_config.backgroundColor);
      }
    }
  }

  // Atualiza Texto Central apenas se habilitado e valor mudou
  if (m_config.showValue && (m_value != m_lastValue || m_changedScale)) {
    WidgetBase::objTFT->fillCircle(m_xPos, m_yPos, rIn - 5, m_config.backgroundText);
    
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", m_value);
    
    WidgetBase::objTFT->setTextColor(m_config.textColor);
    WidgetBase::objTFT->setFont(m_config.font ? m_config.font : &RobotoBold10pt7b);
    printText(buf, m_xPos, m_yPos, MC_DATUM);
    updateFont(FontType::UNLOAD);
  }

  m_lastValue = m_value; // Guarda o estado atual do display
#endif
}

void CircularBar::forceUpdate() {
  WidgetStateGuard widgetStateGuard;
  m_shouldRedraw = true;
}

void CircularBar::setup(const CircularBarConfig &config) {
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  m_config = config;
  sortValues();
  
  m_value = m_config.minValue;
  m_lastValue = m_config.minValue;
  
  if ((int)m_config.radius - (int)m_config.thickness < 15) {
    m_config.showValue = false;
  }

  m_loaded = true;
  m_initialized = true;
  m_shouldRedraw = true;
}

void CircularBar::show() {
  WidgetStateGuard widgetStateGuard;
  m_visible = true;
  m_shouldRedraw = true;
}

void CircularBar::hide() {
  WidgetStateGuard widgetStateGuard;
  m_visible = false;
  m_shouldRedraw = true;
}

#endif // DFK_DISABLE_CIRCULARBAR
