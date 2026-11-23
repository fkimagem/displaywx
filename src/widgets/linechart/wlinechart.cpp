#include "wlinechart.h"

#define CLAMP(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))

const char* LineChart::TAG = "LineChart";

// ─── Mutex ────────────────────────────────────────────────────────────────────

void LineChart::initMutex()
{
#if defined(USING_GRAPHIC_LIB)
  if (!m_mutex)
    m_mutex = xSemaphoreCreateMutex();
#endif
}

void LineChart::destroyMutex()
{
#if defined(USING_GRAPHIC_LIB)
  if (m_mutex) {
    vSemaphoreDelete(m_mutex);
    m_mutex = nullptr;
  }
#endif
}

// ─── Buffers ──────────────────────────────────────────────────────────────────

/**
 * @brief Aloca um único bloco contíguo para ring, snapshot e lastDrawn.
 * @details Layout: pool[ 3 * amountSeries * amountPoints ]
 *          Ponteiros m_ringValues[s], m_snapshotValues[s], m_lastDrawnValues[s]
 *          apontam para fatias desse bloco. Apenas 1 malloc + 3 mallocs de ponteiros.
 */
void LineChart::allocBuffers()
{
  const uint16_t S = m_config.amountSeries;
  const uint16_t P = m_amountPoints;

  // 1 malloc para todos os dados
  m_pool = (int*)malloc(sizeof(int) * 3 * S * P);
  if (!m_pool) {
    ESP_LOGE(TAG, "Failed to allocate pool (%u ints)", 3 * S * P);
    return;
  }

  // 3 mallocs para arrays de ponteiros (pequenos)
  m_ringValues      = (int**)malloc(sizeof(int*) * S);
  m_snapshotValues  = (int**)malloc(sizeof(int*) * S);
  m_lastDrawnValues = (int**)malloc(sizeof(int*) * S);

  if (!m_ringValues || !m_snapshotValues || !m_lastDrawnValues) {
    ESP_LOGE(TAG, "Failed to allocate pointer arrays");
    freeBuffers();
    return;
  }

  // Aponta cada série para sua fatia no pool
  for (uint16_t s = 0; s < S; s++) {
    m_ringValues[s]      = m_pool + (0 * S + s) * P;
    m_snapshotValues[s]  = m_pool + (1 * S + s) * P;
    m_lastDrawnValues[s] = m_pool + (2 * S + s) * P;
  }

  // Inicializa tudo com minValue
  const int fill = m_config.minValue;
  for (uint16_t i = 0; i < 3 * S * P; i++)
    m_pool[i] = fill;
}

void LineChart::freeBuffers()
{
  free(m_pool);           m_pool = nullptr;
  free(m_ringValues);     m_ringValues = nullptr;
  free(m_snapshotValues); m_snapshotValues = nullptr;
  free(m_lastDrawnValues);m_lastDrawnValues = nullptr;
}

// ─── Construtor / Destrutor ───────────────────────────────────────────────────

LineChart::LineChart(uint16_t _x, uint16_t _y, uint8_t _screen)
  : WidgetBase(_x, _y, _screen),
    m_pool(nullptr),
    m_mutex(nullptr),
    m_ringValues(nullptr),
    m_snapshotValues(nullptr),
    m_lastDrawnValues(nullptr),
    m_maxHeight(0), m_maxWidth(0),
    m_spaceBetweenPoints(0.0f),
    m_leftPadding(0),
    m_maxAmountValues(0), m_amountPoints(0),
    m_yTovmin(0), m_yTovmax(0),
    m_borderSize(2),
    m_dotRadius(2), m_minSpaceToShowDot(10),
    m_topBottomPadding(0),
    m_dataVersion(0),
    m_shouldRedraw(true)
{
  memset(&m_config, 0, sizeof(LineChartConfig));
  memset(m_colorsSeries, 0, sizeof(m_colorsSeries));
  memset(m_subtitles, 0, sizeof(m_subtitles));
  memset(m_headBySeries, 0, sizeof(m_headBySeries));

#if defined(USING_GRAPHIC_LIB)
  initMutex();
#endif
}

LineChart::~LineChart()
{
#if defined(USING_GRAPHIC_LIB)
  freeBuffers();
  destroyMutex();
#endif
  m_loaded = false;
  m_shouldRedraw = false;
}

// ─── Validação ────────────────────────────────────────────────────────────────

bool LineChart::validateConfig(const LineChartConfig& config)
{
  if (config.width == 0 || config.height == 0) {
    ESP_LOGE(TAG, "Invalid dimensions: %dx%d", config.width, config.height);
    return false;
  }
  if (config.minValue >= config.maxValue) {
    ESP_LOGE(TAG, "Invalid range: min=%d max=%d", config.minValue, config.maxValue);
    return false;
  }
  if (config.amountSeries == 0 || config.amountSeries > MAX_SERIES) {
    ESP_LOGE(TAG, "Invalid amountSeries: %d", config.amountSeries);
    return false;
  }
  if (!config.colorsSeries) {
    ESP_LOGE(TAG, "colorsSeries is null");
    return false;
  }
  if (config.maxPointsAmount < 2) {
    ESP_LOGE(TAG, "maxPointsAmount must be >= 2");
    return false;
  }
  return true;
}

// ─── Start ────────────────────────────────────────────────────────────────────

void LineChart::start()
{
  CHECK_TFT_VOID
#if defined(USING_GRAPHIC_LIB)

  if (m_config.maxValue < m_config.minValue) {
    int tmp = m_config.minValue;
    m_config.minValue = m_config.maxValue;
    m_config.maxValue = tmp;
    ESP_LOGW(TAG, "minValue/maxValue swapped");
  }

#if defined(USING_GRAPHIC_LIB)
  WidgetBase::objTFT->setFont(m_config.font);
#endif

  // Mede largura do texto dos labels de eixo (sem String temporário)
  char bufMin[20], bufMax[20];
  snprintf(bufMin, sizeof(bufMin), "%d", m_config.minValue);
  snprintf(bufMax, sizeof(bufMax), "%d", m_config.maxValue);

  int16_t tx, ty;
  uint16_t twMin, twMax, th;
  objTFT->getTextBounds(bufMin, 10, 10, &tx, &ty, &twMin, &th);
  objTFT->getTextBounds(bufMax, 10, 10, &tx, &ty, &twMax, &th);

  int16_t biggerText = (int16_t)max(twMin, twMax) + 10;
  updateFont(FontType::UNLOAD);
  biggerText += 10;

  m_maxAmountValues = m_config.width - (biggerText + 10) - (m_borderSize * 2);
  m_amountPoints    = (uint16_t)CLAMP(m_config.maxPointsAmount, 2, m_maxAmountValues);

  m_spaceBetweenPoints = m_maxAmountValues / (float)(m_amountPoints - 1);
  m_maxWidth  = m_maxAmountValues;
  m_maxHeight = m_config.height - (m_topBottomPadding * 2) - (m_borderSize * 2);

  uint16_t restoAreaPlot = 0;
  if (m_config.verticalDivision > 0) {
    uint16_t sizeVSpace = (uint16_t)round(m_maxHeight / m_config.verticalDivision);
    restoAreaPlot = (uint16_t)(m_maxHeight - (sizeVSpace * m_config.verticalDivision));
    if (restoAreaPlot > 0) m_maxHeight -= restoAreaPlot;
  }

  m_leftPadding = biggerText;
  m_yTovmin = (uint16_t)((m_yPos + m_config.height) - (m_topBottomPadding + m_borderSize + restoAreaPlot));
  m_yTovmax = (uint16_t)(m_yPos + m_topBottomPadding + m_borderSize);

  // Aloca buffers sob mutex
  if (m_mutex) xSemaphoreTake(m_mutex, portMAX_DELAY);
  freeBuffers();
  allocBuffers();
  m_dataVersion = 0;
  m_shouldRedraw = true;
  if (m_mutex) xSemaphoreGive(m_mutex);

  ESP_LOGD(TAG, "LineChart started: %dx%d, %d series, %d points",
           m_config.width, m_config.height, m_config.amountSeries, m_amountPoints);
#endif
}

// ─── Setup ────────────────────────────────────────────────────────────────────

void LineChart::setup(const LineChartConfig& config)
{
  CHECK_TFT_VOID
  if (m_loaded) { ESP_LOGW(TAG, "Already configured"); return; }
  if (!validateConfig(config)) return;

  m_config = config;

  for (uint8_t i = 0; i < config.amountSeries && i < MAX_SERIES; i++) {
    m_colorsSeries[i] = config.colorsSeries[i];
    m_subtitles[i]    = config.subtitles ? config.subtitles[i] : nullptr;
  }
  m_config.subtitles = config.subtitles ? m_subtitles : nullptr;

  m_topBottomPadding = 5;
  m_shouldRedraw = true;

  start();
  m_loaded = true;
}

// ─── Push ─────────────────────────────────────────────────────────────────────

bool LineChart::push(uint16_t serieIndex, int newValue)
{
#if defined(USING_GRAPHIC_LIB)
  if (!m_loaded || !m_ringValues) return false;
  if (!m_config.workInBackground && WidgetBase::currentScreen != m_screen) return false;
  if (serieIndex >= m_config.amountSeries || m_amountPoints == 0) return false;

  if (m_mutex) xSemaphoreTake(m_mutex, portMAX_DELAY);

  m_ringValues[serieIndex][m_headBySeries[serieIndex]] =
    constrain(newValue, m_config.minValue, m_config.maxValue);

  m_headBySeries[serieIndex] =
    (uint16_t)((m_headBySeries[serieIndex] + 1) % m_amountPoints);

  m_dataVersion++;
  m_shouldRedraw = true;

  if (m_mutex) xSemaphoreGive(m_mutex);
  return true;
#else
  return false;
#endif
}

// ─── Snapshot ─────────────────────────────────────────────────────────────────

uint32_t LineChart::snapshotUnderMutex()
{
#if defined(USING_GRAPHIC_LIB)
  if (!m_mutex) return 0;
  xSemaphoreTake(m_mutex, portMAX_DELAY);

  uint32_t ver = m_dataVersion;
  for (uint16_t s = 0; s < m_config.amountSeries; s++) {
    uint16_t head = m_headBySeries[s];
    for (uint16_t i = 0; i < m_amountPoints; i++) {
      m_snapshotValues[s][i] =
        m_ringValues[s][(head + i) % m_amountPoints];
    }
  }

  xSemaphoreGive(m_mutex);
  return ver;
#else
  return 0;
#endif
}

// ─── Draw helpers ─────────────────────────────────────────────────────────────

void LineChart::drawBackground()
{
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_LOADED_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
#if defined(USING_GRAPHIC_LIB)
  if (!m_shouldRedraw) return;

  for (uint16_t i = 0; i < m_borderSize; i++) {
    WidgetBase::objTFT->drawRect(m_xPos + i, m_yPos + i,
      m_config.width - (2 * i), m_config.height - (2 * i),
      m_config.borderColor);
  }

  WidgetBase::objTFT->fillRect(
    m_xPos + m_borderSize, m_yPos + m_borderSize,
    m_config.width  - (2 * m_borderSize),
    m_config.height - (2 * m_borderSize),
    m_config.backgroundColor);

  WidgetBase::setFontNull();
  WidgetBase::objTFT->setTextColor(m_config.textColor);

  // Sem String temporário: usa buffer local
  char buf[20];
  snprintf(buf, sizeof(buf), "%d", m_config.maxValue);
  printText(buf, m_xPos + m_borderSize + m_leftPadding - 4,
            m_yTovmax - m_topBottomPadding, TR_DATUM);

  snprintf(buf, sizeof(buf), "%d", m_config.minValue);
  printText(buf, m_xPos + m_borderSize + m_leftPadding - 4,
            m_yTovmin, BR_DATUM);

  if (m_config.showZeroLine && m_config.minValue <= 0 && m_config.maxValue >= 0) {
    uint16_t yZero = (uint16_t)map(0, m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);
    if (abs((int)m_yTovmin - (int)yZero) >= 15 && abs((int)yZero - (int)m_yTovmax) >= 15) {
      printText("0", m_xPos + m_borderSize + m_leftPadding - 4, yZero, BR_DATUM);
    }
  }
  updateFont(FontType::UNLOAD);

  WidgetBase::objTFT->drawFastVLine(
    m_xPos + m_leftPadding + m_borderSize - 1,
    m_yPos + m_topBottomPadding + m_borderSize,
    m_maxHeight + 1, m_config.textColor);

  WidgetBase::objTFT->drawFastHLine(
    m_xPos + m_leftPadding + m_borderSize - 1,
    m_yTovmin + 1, m_maxWidth, m_config.textColor);
#endif
}

void LineChart::drawGrid()
{
  CHECK_TFT_VOID
#if defined(USING_GRAPHIC_LIB)
  if (m_config.verticalDivision == 0) return;

  uint16_t sizeVSpace = (uint16_t)round(m_maxHeight / m_config.verticalDivision);
  uint16_t xStart = m_xPos + m_leftPadding + 2;
  uint16_t xEnd   = xStart + m_maxWidth;

  for (uint16_t i = 0; i < m_config.verticalDivision; i++) {
    uint16_t yLine = m_yPos + m_topBottomPadding + (sizeVSpace * i);
    if (yLine < m_yPos + m_topBottomPadding + m_maxHeight)
      WidgetBase::objTFT->drawFastHLine(xStart, yLine, m_maxWidth, m_config.gridColor);
  }
  WidgetBase::objTFT->drawFastHLine(xStart, m_yPos + m_topBottomPadding, m_maxWidth, m_config.gridColor);

  for (uint16_t x = xStart; x <= xEnd; x += sizeVSpace)
    WidgetBase::objTFT->drawFastVLine(x, m_yPos + m_topBottomPadding, m_maxHeight, m_config.gridColor);
  WidgetBase::objTFT->drawFastVLine(xEnd, m_yPos + m_topBottomPadding, m_maxHeight, m_config.gridColor);
#endif
}

void LineChart::drawMarkLineAt(int value)
{
#if defined(USING_GRAPHIC_LIB)
  uint16_t y = (uint16_t)map(value, m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);
  WidgetBase::objTFT->drawFastHLine(
    m_xPos + m_leftPadding + m_borderSize, y, m_maxWidth, m_config.textColor);
#endif
}

void LineChart::eraseSerieFromBuffer(uint8_t serieIndex, const int* values)
{
  CHECK_TFT_VOID
#if defined(USING_GRAPHIC_LIB)
  const uint16_t startX = m_xPos + m_leftPadding + m_borderSize + 1;
  const uint16_t bg     = m_config.backgroundColor;

  for (uint16_t i = 1; i < m_amountPoints; i++) {
    int16_t x0 = (int16_t)round(startX + (i - 1) * m_spaceBetweenPoints);
    int16_t x1 = (int16_t)round(startX +  i      * m_spaceBetweenPoints);
    int16_t y0 = (int16_t)map(values[i-1], m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);
    int16_t y1 = (int16_t)map(values[i],   m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);

    WidgetBase::objTFT->drawLine(x0, y0, x1, y1, bg);
    if (m_config.boldLine) {
      WidgetBase::objTFT->drawLine(x0, y0-1, x1, y1-1, bg);
      WidgetBase::objTFT->drawLine(x0, y0+1, x1, y1+1, bg);
    }
    if (m_config.showDots && m_spaceBetweenPoints >= m_minSpaceToShowDot)
      WidgetBase::objTFT->fillCircle(x0, y0, m_dotRadius, bg);
  }
#endif
}

void LineChart::drawSerieFromBuffer(uint8_t serieIndex, const int* values)
{
  CHECK_TFT_VOID
#if defined(USING_GRAPHIC_LIB)
  const uint16_t startX = m_xPos + m_leftPadding + m_borderSize + 1;
  const uint16_t color  = m_colorsSeries[serieIndex];

  for (uint16_t i = 1; i < m_amountPoints; i++) {
    int16_t x0 = (int16_t)round(startX + (i - 1) * m_spaceBetweenPoints);
    int16_t x1 = (int16_t)round(startX +  i      * m_spaceBetweenPoints);
    int16_t y0 = (int16_t)map(values[i-1], m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);
    int16_t y1 = (int16_t)map(values[i],   m_config.minValue, m_config.maxValue, m_yTovmin, m_yTovmax);

    WidgetBase::objTFT->drawLine(x0, y0, x1, y1, color);
    if (m_config.boldLine) {
      WidgetBase::objTFT->drawLine(x0, y0-1, x1, y1-1, color);
      WidgetBase::objTFT->drawLine(x0, y0+1, x1, y1+1, color);
    }
    if (m_config.showDots && m_spaceBetweenPoints >= m_minSpaceToShowDot)
      WidgetBase::objTFT->fillCircle(x0, y0, m_dotRadius, color);
  }

  if (m_config.subtitles && m_config.subtitles[serieIndex])
    m_config.subtitles[serieIndex]->setTextInt(values[m_amountPoints - 1]);
#endif
}

void LineChart::eraseAllFromLastDrawn()
{
#if defined(USING_GRAPHIC_LIB)
  for (uint16_t s = 0; s < m_config.amountSeries; s++)
    eraseSerieFromBuffer((uint8_t)s, m_lastDrawnValues[s]);
#endif
}

void LineChart::drawAllFromSnapshot()
{
#if defined(USING_GRAPHIC_LIB)
  for (uint16_t s = 0; s < m_config.amountSeries; s++)
    drawSerieFromBuffer((uint8_t)s, m_snapshotValues[s]);
#endif
}

// ─── Redraw ───────────────────────────────────────────────────────────────────

void LineChart::redraw()
{
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_LOADED_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_DEBOUNCE_REDRAW_VOID
#if defined(USING_GRAPHIC_LIB)
  if (!m_shouldRedraw) return;

  uint32_t capturedVer = snapshotUnderMutex();

  eraseAllFromLastDrawn();
  drawGrid();

  if (m_config.showZeroLine && m_config.minValue <= 0 && m_config.maxValue >= 0)
    drawMarkLineAt(0);

  drawAllFromSnapshot();

  for (uint16_t s = 0; s < m_config.amountSeries; s++)
    memcpy(m_lastDrawnValues[s], m_snapshotValues[s], sizeof(int) * m_amountPoints);

  // Verifica se houve push durante o desenho
  uint32_t nowVer = 0;
  if (m_mutex) {
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    nowVer = m_dataVersion;
    xSemaphoreGive(m_mutex);
  }
  m_shouldRedraw = (nowVer != capturedVer);
#endif
}

// ─── Show / Hide ──────────────────────────────────────────────────────────────

void LineChart::show()
{
  m_visible = true;
  m_shouldRedraw = true;
}

void LineChart::hide()
{
  m_visible = false;
  m_shouldRedraw = false;
}