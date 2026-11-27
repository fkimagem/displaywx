#ifndef WVANALOG
#define WVANALOG

#include "../widgetbase.h"
#if defined(USING_GRAPHIC_LIB)
#include "../../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#endif

/// @brief Estrutura de configuração para o VAnalog.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um display analógico vertical.
///          Deve ser preenchida e passada para o método setup().
struct VerticalAnalogConfig {
  int minValue;                 ///< Valor mínimo da escala.
  int maxValue;                 ///< Valor máximo da escala.
  uint16_t width;           ///< Largura do display VAnalog.
  uint16_t height;          ///< Altura do display VAnalog.
  uint16_t arrowColor;      ///< Cor da seta marcadora.
  uint16_t textColor;       ///< Cor do texto de exibição.
  uint16_t backgroundColor; ///< Cor de fundo do display.
  uint16_t borderColor;     ///< Cor da borda do display.
  uint8_t steps;            ///< Número de divisões na escala.
};

/// @brief Representa um widget de display analógico vertical usado para visualizar valores numéricos em uma escala vertical.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar
///          displays analógicos verticais na tela. O VAnalog desenha uma escala vertical com marcações,
///          uma seta indicadora que move baseada no valor atual, e opcionalmente exibe o valor numérico.
///          O widget pode ser configurado com diferentes tamanhos, cores, faixas de valores e número de divisões.
class VAnalog : public WidgetBase
{
public:
  VAnalog(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~VAnalog();
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void drawBackground();
  void setValue(int newValue, bool _viewValue);
  void redraw() override;
  void forceUpdate() override;
  void setup(const VerticalAnalogConfig& config);
  void show() override;
  void hide() override;
  int getMinValue();
  int getMaxValue();
  void setScale(int newMinValue, int newMaxValue);
  void setMinValue(int newValue);
  void setMaxValue(int newValue);

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  bool m_updateText; ///< Flag indicando se o texto de exibição precisa ser atualizado.
  int m_currentValue; ///< Valor atual exibido na escala.
  int m_lastValue; ///< Último valor exibido na escala.
  Margin_t m_margin; ///< Margin data.

  uint8_t m_arrowSize = 6; ///< Size of the arrow.
  Rect_t m_drawArea; ///< Area to draw the analog scale.
  Rect_t m_arrowArea; ///< Area to draw the arrow.
  Rect_t m_textArea; ///< Area to draw the text.

  VerticalAnalogConfig m_config; ///< Estrutura de configuração para o VAnalog.

  void cleanupMemory();
  void start();
  uint16_t calculateArrowVerticalPosition(int value);
  void drawArrow();
  void clearArrow();
  void drawText();
  void sortValues();
  bool m_changedScale = false;
};

#endif
