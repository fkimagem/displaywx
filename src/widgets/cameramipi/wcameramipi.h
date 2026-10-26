#ifndef WCAMERAMIPI
#define WCAMERAMIPI

#include "../widgetbase.h"

/// @brief Estrutura de configuração para o CameraMipi.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um botão retangular.
///          Deve ser preenchida e passada para o método setup().
struct CameraMipiConfig {
  uint16_t width;         ///< Largura da câmera MIPI.
  uint16_t height;        ///< Altura da câmera MIPI.
  bool showZoomLevel = false; ///< Se true, mostra o nível de zoom no canto do widget.
  bool showFps = false; ///< Se true, mostra o fps no canto da miniatura.
  uint8_t fps = 5; ///< Quadros por segundo da miniatura, de 5 a 24.
};

/// @brief Widget de câmera MIPI com tamanho e cor personalizáveis.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade para exibir a imagem da camera em um retangulo
///          na tela ou tela cheia. Precisa detectar o duplo clique.
class CameraMipi : public WidgetBase
{
public:
  CameraMipi(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~CameraMipi();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void setup(const CameraMipiConfig& config);
  void changeState();
  bool getStatus();
  void setStatus(bool _status);
  bool getEnabled();
  void setEnabled(bool newState);
  void drawBackground();
  
  void show() override;
  void hide() override;

  int16_t getXPos() const;
  int16_t getYPos() const;
  uint16_t getWidth() const;
  uint16_t getHeight() const;
  bool isFullScreen() const;
  static bool hasFullScreen();
  static void exitFullScreen();
  void leaveFullScreen();
  void setZoomLevel(uint8_t level);
  uint8_t getZoomLevel() const;
  bool showZoomLevel() const;
  bool showFps() const;
  uint8_t fps() const;
  bool isRedrawDue(uint32_t nowMs) const;
  void noteRedraw(uint32_t nowMs);
  const char *zoomLabel() const;
  int16_t centerPointX() const;
  int16_t centerPointY() const;
  void setCenterPoint(int16_t x, int16_t y);
  bool keepZoomOnMaximized() const;
  void setKeepZoomOnMaximized(bool keep);
  void noteZoomClamped(uint8_t used);
  bool openConfig();
  uint16_t getXPlot() const;
  uint16_t getYPlot() const;
  uint16_t getWidthPlot() const;
  uint16_t getHeightPlot() const;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  bool m_status; ///< Status atual ligado/desligado da câmera MIPI.
  bool m_shouldRedraw; ///< Flag indicando se o LED deve ser redesenhado.
  bool m_initialized; ///< Flag para rastrear se o widget foi adequadamente inicializado.
  bool m_enabled; ///< Indica se a câmera MIPI está habilitada ou desabilitada.
  CameraMipiConfig m_config; ///< Estrutura contendo configuração da câmera MIPI.

  bool m_fullScreen;
  uint8_t m_margin;
  uint8_t m_zoomLevel;
  int16_t m_centerX = -1;
  int16_t m_centerY = -1;
  bool m_keepZoomOnMaximized = false;
  bool m_zoomClampNoted = false;
  uint32_t m_lastRedrawMs = 0;
  mutable char m_zoomText[8] = {'1', 'x', '\0'};
  static bool s_hasFullScreen;
  
  void cleanupMemory();
  void start();
};

#endif
