#ifndef WTBOX
#define WTBOX

#include "../widgetbase.h"
#include "../../extras/charstring.h"

/// @brief Estrutura de configuração para o TextBox.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar uma caixa de texto.
///          Deve ser preenchida e passada para o método setup().
struct TextBoxConfig {
  const char* startValue;    ///< Valor de texto inicial para exibir.
  functionLoadScreen_t funcPtr; ///< Ponteiro de função para a função de carregamento da tela pai.
  functionCB_t cb;           ///< Função callback para executar na interação.
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* font;       ///< Fonte para usar no texto.
  #endif
  uint16_t width;            ///< Largura da caixa de texto.
  uint16_t height;           ///< Altura da caixa de texto.
  uint16_t letterColor;      ///< Cor do texto para exibir o valor.
  uint16_t backgroundColor;  ///< Cor de fundo da caixa de texto.
};

/// @brief Widget de caixa de texto para exibir e editar texto.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          caixas de texto interativas na tela. O TextBox desenha um retângulo com borda para exibir
///          e editar texto, permitindo entrada através de um teclado virtual (@ref WKeyboard).
///          O widget pode ser configurado com diferentes tamanhos, cores, fontes e valores iniciais.
///          O TextBox é totalmente funcional com suporte a detecção de toque para ativar o teclado,
///          atualização de valores em tempo real e callback para eventos de interação.
class TextBox : public WidgetBase
{
public:
  functionLoadScreen_t parentScreen; ///< Ponteiro para a função de carregamento da tela pai.

  TextBox(uint16_t _x, uint16_t _y, uint8_t _screen);
  TextBox();
  ~TextBox();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void setup(const TextBoxConfig& config);
  void onClick();
  void setValue(const char* str);
  const char* getValue();
  
  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  unsigned long m_myTime; ///< Timestamp para manipular funções relacionadas a tempo.
  uint16_t m_width; ///< Largura da caixa de texto.
  uint16_t m_height; ///< Altura da caixa de texto.
  uint16_t m_backgroundColor; ///< Cor de fundo da caixa de texto.
  uint16_t m_letterColor; ///< Cor do texto para exibir o valor.
  CharString m_value; ///< String representando o valor de texto atual.
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont *m_font; ///< Fonte para usar no texto.
  #endif
  uint8_t m_padding; ///< Preenchimento da caixa de texto.
  bool m_shouldRedraw; ///< Flag para indicar se o TextBox deve ser redesenhado.
  
  #if defined(USING_GRAPHIC_LIB)
  void setup(uint16_t _width, uint16_t _height, uint16_t _letterColor, uint16_t _backgroundColor, const char* _startValue, const GFXfont* _font, functionLoadScreen_t _funcPtr, functionCB_t _cb);
  #endif
};

#endif