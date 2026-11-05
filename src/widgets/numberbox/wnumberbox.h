#ifndef WNBOX
#define WNBOX

#include "../widgetbase.h"
#include "../../extras/charstring.h"

/// @brief Estrutura de configuração para o NumberBox.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar uma caixa de número.
///          Deve ser preenchida e passada para o método setup().
struct NumberBoxConfig {
  functionLoadScreen_t funcPtr;  ///< Ponteiro de função para a função de carregamento da tela pai.
  functionCB_t callback;         ///< Função callback para executar na interação.
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* font;           ///< Fonte para usar no texto.
  #endif
  float startValue;              ///< Valor numérico inicial para exibir.
  uint16_t width;                ///< Largura da caixa de número.
  uint16_t height;               ///< Altura da caixa de número.
  uint16_t letterColor;          ///< Cor para o texto exibindo o valor.
  uint16_t backgroundColor;      ///< Cor de fundo da caixa de número.
  uint8_t decimalPlaces;         ///< Número de casas decimais para exibir.
};

/// @brief Widget de caixa de número para exibir e editar valores numéricos.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          caixas de número interativas na tela. O NumberBox desenha um retângulo com borda para exibir
///          e editar valores numéricos, permitindo entrada através de um teclado numérico virtual (@ref Numpad).
///          O widget pode ser configurado com diferentes tamanhos, cores, fontes e valores iniciais.
///          O NumberBox é totalmente funcional com suporte a detecção de toque para ativar o teclado numérico,
///          atualização de valores em tempo real e callback para eventos de interação.
class NumberBox : public WidgetBase
{
public:
  functionLoadScreen_t parentScreen; ///< Ponteiro para a função de carregamento da tela pai.

  NumberBox(uint16_t _x, uint16_t _y, uint8_t _screen);
  NumberBox();
  ~NumberBox();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void setup(const NumberBoxConfig& config);
  void onClick();
  void setValue(double str);
  double getValue();
  const char* getValueChar();
  void convertoToString(double f, char* buf, size_t bufSize);
  
  void show() override;
  void hide() override;

  NumberBoxConfig getConfig() { return m_config; }

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  CharString m_value; ///< String representando o valor numérico atual.
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* m_font; ///< Fonte para usar no texto.
  #endif
  uint8_t m_padding; ///< Preenchimento da caixa de número.
  bool m_shouldRedraw; ///< Flag para indicar se a caixa de número deve ser redesenhada.
  NumberBoxConfig m_config; ///< Estrutura contendo configuração da caixa de número.
  
  void cleanupMemory();
  #if defined(USING_GRAPHIC_LIB)
  void setup(uint16_t _width, uint16_t _height, uint16_t _letterColor, uint16_t _backgroundColor, float _startValue, const GFXfont* _font, functionLoadScreen_t _funcPtr, functionCB_t _cb);
  #endif
};

#endif