#ifndef WLINECHART
#define WLINECHART

#include "../widgetbase.h"
#if defined(USING_GRAPHIC_LIB)
#include "../../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#endif
#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "../label/wlabel.h"

struct LineChartConfig {
  uint16_t* colorsSeries;
  Label** subtitles;
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* font;
  #endif
  int minValue;
  int maxValue;
  uint16_t width;
  uint16_t height;
  uint16_t gridColor;
  uint16_t borderColor;
  uint16_t backgroundColor;
  uint16_t textColor;
  uint16_t verticalDivision;
  uint16_t maxPointsAmount;
  uint8_t amountSeries;
  bool workInBackground;
  bool showZeroLine;
  bool boldLine;
  bool showDots;
};

/// @brief Widget de gráfico de linhas com múltiplas séries, ring buffer e snapshot thread-safe.
class LineChart : public WidgetBase
{
public:
  static constexpr uint16_t SHOW_ALL = 9999;

  LineChart(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~LineChart();
  LineChart(const LineChart&) = delete;
  LineChart& operator=(const LineChart&) = delete;

  bool detectTouch(uint16_t* _xTouch, uint16_t* _yTouch) override { return false; }
  functionCB_t getCallbackFunc() override { WidgetStateGuard guard; return m_callback; }

  void drawBackground();
  bool push(uint16_t serieIndex, int newValue);
  void redraw() override;
  void forceUpdate() override { m_shouldRedraw = true; }
  void setup(const LineChartConfig& config);
  void show() override;
  void hide() override;

private:
  static const char* TAG;
  static constexpr uint8_t MAX_SERIES = 10;

  // --- Ponteiros (4 bytes cada) ---
  int* m_pool;              ///< Buffer contíguo único: [ring | snapshot | lastDrawn] x [series x points]
  SemaphoreHandle_t m_mutex;
  StaticSemaphore_t m_mutexStorage;

  // --- Layout derivado de m_pool ---
  int* m_ringValues[MAX_SERIES];
  int* m_snapshotValues[MAX_SERIES];
  int* m_lastDrawnValues[MAX_SERIES];

  // --- Config e cores (copiadas internamente) ---
  LineChartConfig m_config;
  uint16_t m_colorsSeries[MAX_SERIES];
  Label*   m_subtitles[MAX_SERIES];

  // --- Geometria (calculada em start()) ---
  uint32_t m_maxHeight;
  uint32_t m_maxWidth;
  float    m_spaceBetweenPoints;
  int16_t  m_leftPadding;
  uint16_t m_maxAmountValues;
  uint16_t m_amountPoints;
  uint16_t m_yTovmin;
  uint16_t m_yTovmax;
  uint16_t m_borderSize;
  uint8_t  m_dotRadius;
  uint8_t  m_minSpaceToShowDot;
  uint8_t  m_topBottomPadding;

  // --- Estado ---
  uint32_t m_dataVersion;
  uint16_t m_headBySeries[MAX_SERIES];
  std::atomic<bool> m_shouldRedraw;

  // Lifecycle
  bool initMutex();
  void destroyMutex();
  bool allocBuffers();
  void freeBuffers();

  // Setup
  bool validateConfig(const LineChartConfig& config);
  bool start();

  // Draw helpers
  void drawGrid();
  void drawMarkLineAt(int value);
  void eraseSerieFromBuffer(uint8_t serieIndex, const int* values);
  void drawSerieFromBuffer(uint8_t serieIndex, const int* values);
  void eraseAllFromLastDrawn();
  void drawAllFromSnapshot();

  // Snapshot
  uint32_t snapshotUnderMutex();
};

#endif