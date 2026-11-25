#ifndef WGAUGESUPER
#define WGAUGESUPER

#include "../widgetbase.h"
#if defined(USING_GRAPHIC_LIB)
#include "../../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#endif

/// @brief Estrutura de configuração para o GaugeSuper.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um gauge super.
///          Deve ser preenchida e passada para o método setup(). Todos os campos são obrigatórios
///          exceto title, intervals, colors e fontFamily que podem ser nullptr se não utilizados.
/// @note Os arrays intervals e colors devem ter o mesmo tamanho especificado em amountIntervals.
///       O valor de amountIntervals não pode exceder MAX_SERIES (10).
struct GaugeConfig {
  const char* title; ///< Título exibido no gauge. Pode ser nullptr se não houver título. Máximo de MAX_TITLE_LENGTH (20) caracteres. Será truncado automaticamente se exceder.
  const int* intervals; ///< Array de valores dos intervalos. Deve ser nullptr se amountIntervals for 0, caso contrário deve apontar para um array válido. Valores devem estar ordenados entre minValue e maxValue.
  const uint16_t* colors; ///< Array de cores correspondentes aos intervalos (formato RGB565). Deve ter o mesmo tamanho de intervals. Deve ser nullptr se amountIntervals for 0, caso contrário deve apontar para um array válido.
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* fontFamily; ///< Fonte usada para renderização de texto. Pode ser nullptr para usar fonte padrão. Necessário se showLabels for true ou se houver título.
  #endif
  int minValue; ///< Valor mínimo da faixa do gauge. Deve ser menor que maxValue. Se minValue >= maxValue, os valores serão trocados automaticamente.
  int maxValue; ///< Valor máximo da faixa do gauge. Deve ser maior que minValue. Define o limite superior da faixa de valores exibida.
  uint16_t width; ///< Largura do gauge em pixels. Deve ser maior que 0. Define a largura total do retângulo do gauge.
  uint16_t height; ///< Altura do gauge em pixels. Define a altura total do retângulo do gauge. Usado para calcular dimensões disponíveis.
  uint16_t borderColor; ///< Cor da borda do gauge (formato RGB565). Usada para desenhar as bordas do retângulo do gauge.
  uint16_t textColor; ///< Cor para texto e exibição de valores (formato RGB565). Usada para renderizar o valor numérico exibido no centro do gauge.
  uint16_t backgroundColor; ///< Cor de fundo do gauge (formato RGB565). Usada para preencher a área interna do gauge.
  uint16_t titleColor; ///< Cor do texto do título (formato RGB565). Usada apenas se title não for nullptr.
  uint16_t needleColor; ///< Cor da agulha (formato RGB565). Usada para desenhar a agulha que indica o valor atual.
  uint16_t markersColor; ///< Cor dos marcadores (formato RGB565). Usada para desenhar os marcadores graduados no arco do gauge.
  uint8_t amountIntervals; ///< Número de intervalos e cores. Máximo de MAX_SERIES (10). Se 0, não haverá intervalos coloridos. Deve corresponder ao tamanho dos arrays intervals e colors.
  bool showLabels; ///< Flag para mostrar rótulos de texto dos intervalos. Se true, exibe os valores dos intervalos como rótulos no gauge. Requer fontFamily configurado.
};

/// @brief Widget de gauge super com agulha e intervalos codificados por cores.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          gauges super avançados na tela. O GaugeSuper desenha um gauge circular com agulha móvel,
///          intervalos coloridos, marcadores graduados e rótulos opcionais. O widget pode ser configurado
///          com diferentes larguras, faixas de valores, intervalos coloridos e exibição de título.
///          O gauge é totalmente funcional com suporte a animação suave da agulha e atualização em tempo real.
class GaugeSuper : public WidgetBase
{
public:
  GaugeSuper(uint16_t _x, uint16_t _y, uint8_t _screen);
  virtual ~GaugeSuper();
  GaugeSuper(const GaugeSuper&) = delete;
  GaugeSuper& operator=(const GaugeSuper&) = delete;

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  void show() override;
  void hide() override;
  
  void setup(const GaugeConfig& config);
  void drawBackground();
  void setValue(int newValue);

private:
  static constexpr uint8_t MAX_SERIES = 10;  ///< Número máximo de séries/intervalos permitidos.
  static constexpr uint8_t MAX_TITLE_LENGTH = 20;  ///< Comprimento máximo do título.
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  // Configuration
  GaugeConfig m_config; ///< Estrutura contendo configuração completa do gauge super.
  
  // Internal state
  float m_ltx; ///< Última tangente calculada para o ângulo da agulha.
  CoordPoint_t m_lastPointNeedle; ///< Coordenadas do ponto final da agulha.
  uint16_t m_stripColor; ///< Cor para a faixa colorida.
  uint16_t m_indexCurrentStrip; ///< Índice para o segmento atual da faixa de cor.
  int m_divisor; ///< Multiplicador para valores de faixa (usado para exibir valores >999).
  bool m_isFirstDraw; ///< Flag indicando se o gauge está sendo desenhado pela primeira vez.
  
  // Dynamic arrays (managed carefully to avoid memory leaks)
  int *m_intervals; ///< Array de valores dos intervalos.
  uint16_t *m_colors; ///< Array de cores correspondentes aos intervalos.
  char *m_title; ///< Título exibido no gauge.
  
  // Memory management flags
  bool m_intervalsAllocated; ///< Flag para rastrear alocação de intervalos.
  bool m_colorsAllocated; ///< Flag para rastrear alocação de cores.
  bool m_titleAllocated; ///< Flag para rastrear alocação do título.
  
  // Calculated dimensions
  uint32_t m_height; ///< Altura do retângulo do gauge.
  uint32_t m_radius; ///< Raio do círculo do gauge.
  int m_currentValue; ///< Valor atual a ser exibido no gauge.
  int m_lastValue; ///< Último valor exibido pela agulha.
  
  // Drawing parameters
  int m_stripWeight; ///< Largura da faixa colorida.
  int m_maxAngle; ///< Metade do ângulo total em graus do gauge.
  int m_offsetYAgulha; ///< Offset Y para o ponto de pivô da agulha.
  int m_rotation; ///< Ângulo de rotação do gauge (0 = meio direito).
  int m_distanceAgulhaArco; ///< Distância do ponto final da agulha ao arco.
  uint8_t m_borderSize; ///< Largura da borda.
  uint16_t m_availableWidth; ///< Largura disponível para desenho.
  uint16_t m_availableHeight; ///< Altura disponível para desenho.
  
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* m_usedFont; ///< Fonte usada para renderização de texto.
  #endif
  TextBound_t m_textBoundForValue; ///< Caixa delimitadora para o texto do valor exibido.
  CoordPoint_t m_origem; ///< Centro do relógio do gauge.
  
  void start();
  void cleanupMemory();
  bool validateConfig(const GaugeConfig& config);
  bool isTitleVisible() const;
  bool isLabelsVisible() const;
};

#endif
