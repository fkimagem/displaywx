#ifndef WVERTICALBAR
#define WVERTICALBAR

#include "../widgetbase.h"
#include "../label/wlabel.h" // Para ponteiro Label

/// @brief Estrutura de configuração para o VBar.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar uma barra vertical.
///          Deve ser preenchida e passada para o método setup().
struct VerticalBarConfig {
  Label* subtitle;
  int minValue;                ///< Valor mínimo da faixa da barra.
  int maxValue;                ///< Valor máximo da faixa da barra.
  int round; ///< Raio para os cantos arredondados da barra.
  Orientation orientation; ///< Orientação da barra (vertical ou horizontal).
  uint16_t width;          ///< Largura da exibição do VBar.
  uint16_t height;         ///< Altura da exibição do VBar.
  uint16_t backgroundColor = CFK_GREY11; ///< Empty track and erased pixels.
  uint16_t borderColor = CFK_BLACK; ///< One-pixel outline.
  uint16_t filledColor;    ///< Cor usada para a porção preenchida da barra.
};

/// @brief Representa um widget de barra vertical usado para exibir um valor como uma barra preenchida dentro de uma faixa especificada.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar
///          barras verticais ou horizontais na tela. O VBar desenha uma barra que se preenche proporcionalmente
///          ao valor atual dentro de uma faixa definida. O widget pode ser configurado com diferentes tamanhos,
///          cores, faixas de valores, raios de canto e orientação.
class VBar : public WidgetBase
{
public:
  VBar(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~VBar();
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void setValue(int newValue);
  void setMinValue(int newValue);
  void setMaxValue(int newValue);
  int getMinValue();
  int getMaxValue();
  void setScale(int newMinValue, int newMaxValue);
  void redraw() override;
  void forceUpdate() override;
  void drawBackground();
  void setup(const VerticalBarConfig& config);
  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  int m_currentValue; ///< Valor atual representado pela porção preenchida da barra.
  int m_lastValue; ///< Último valor representado pela porção preenchida da barra.
  VerticalBarConfig m_config; ///< Estrutura de configuração para o VBar.
  void sortValues();

  void cleanupMemory();
  void start();

  bool m_changedScale = false;
};
#endif
