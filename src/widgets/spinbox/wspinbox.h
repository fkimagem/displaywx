#ifndef WSPINBOX
#define WSPINBOX

#include "../widgetbase.h"

/// @brief Estrutura de configuração para o SpinBox.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar uma caixa de spin.
///          Deve ser preenchida e passada para o método setup().
struct SpinBoxConfig {
  functionCB_t callback;  ///< Função callback para executar quando o valor mudar.
  int minValue;                ///< Valor mínimo da faixa da caixa de spin.
  int maxValue;               ///< Valor máximo da faixa da caixa de spin.
  int startValue;         ///< Valor atual da caixa de spin.
  uint16_t width;         ///< Largura da caixa de spin.
  uint16_t height;        ///< Altura da caixa de spin.
  uint16_t step;          ///< Valor do passo para incrementos e decrementos da caixa de spin.
  uint16_t color;         ///< Cor para os elementos de exibição.
  uint16_t textColor;     ///< Cor do texto.
};

/// @brief Widget de caixa de spin para entrada numérica com botões de incremento e decremento.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          caixas de spin interativas na tela. O SpinBox desenha uma caixa com dois botões (incremento
///          e decremento) e uma área central para exibir o valor atual. O widget pode ser configurado
///          com diferentes tamanhos, faixas de valores, passos de incremento/decremento, cores e
///          callbacks. A caixa é totalmente funcional com suporte a detecção de toque nos botões,
///          atualização de valores dentro da faixa definida e callbacks para notificar mudanças.
class SpinBox : public WidgetBase
{
public:
  SpinBox(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~SpinBox();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void drawBackground();
  void setup(const SpinBoxConfig& config);
  int getValue();
  void setValue(int _value, bool triggerCallback = true);
  
  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  uint16_t m_pressedColor; ///< Cor exibida quando a caixa de spin está pressionada.
  bool m_status; ///< Status atual da caixa de spin (pressionada ou não).
  int m_currentValue; ///< Valor numérico atual da caixa de spin.
  uint8_t m_radius = 5; ///< Raio para os botões circulares na caixa de spin.
  uint8_t m_offset; ///< Offset para posicionar elementos dentro da caixa de spin.
  TextBound_t m_lastArea; ///< Última área da caixa de spin.
  SpinBoxConfig m_config; ///< Estrutura contendo configuração da caixa de spin.
  Rect_t m_buttonSize;
  Rect_t m_textAreaSize;
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* m_font;  ///< Ponteiro para a fonte usada na caixa de spin.
  #endif
  
  void cleanupMemory();
  void start();
  void increaseValue();
  void decreaseValue();
  #if defined(USING_GRAPHIC_LIB)
  void setup(uint16_t _width, uint16_t _height, uint16_t _step, int _min, int _max, int _startValue, uint16_t _cor, uint16_t _textCor, functionCB_t _cb);
  #endif
};

#endif