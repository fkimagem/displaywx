#ifndef WNUMPAD
#define WNUMPAD

#include "../widgetbase.h"
#include "wnumberbox.h"
#include "../../extras/charstring.h"
#if defined(USING_GRAPHIC_LIB)
#include "../../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#endif

/// @brief Número de linhas na grade do teclado numérico
#define NROWS 4

/// @brief Número de colunas na grade do teclado numérico
#define NCOLS 4

/// @brief Estrutura de configuração para o Numpad.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um teclado numérico.
///          Deve ser preenchida e passada para o método setup().
struct NumpadConfig {
  #if defined(USING_GRAPHIC_LIB)
  const GFXfont* fontKeys;       ///< Fonte para as teclas.
  const GFXfont* fontPreview;    ///< Fonte para a área de preview.
  #endif
  uint16_t backgroundColor;      ///< Cor de fundo do Numpad.
  uint16_t letterColor;          ///< Cor para o texto das teclas.
  uint16_t keyColor;             ///< Cor de fundo das teclas.
};

/// @brief Widget de teclado numérico que permite entrada de números através de um teclado na tela.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          teclados numéricos interativos na tela. O Numpad desenha um teclado numérico com teclas
///          para dígitos (0-9), ponto decimal, operações matemáticas (incremento, decremento, inversão)
///          e ações de controle (Delete, OK). O widget pode ser configurado com diferentes cores e fontes
///          e trabalha em conjunto com NumberBox para entrada de dados numéricos. O teclado é totalmente
///          funcional com suporte a detecção de toque e atualização de valores em tempo real.
class Numpad : public WidgetBase 
{
public:
  static uint16_t m_backgroundColor; ///< Cor de fundo estática para o Numpad.
  static uint16_t m_letterColor; ///< Cor de letras estática para o Numpad.
  static uint16_t m_keyColor; ///< Cor de teclas estática para o Numpad.
  
  NumberBox *m_field; ///< Ponteiro para o NumberBox associado ao Numpad.
  
  Numpad(uint16_t _x, uint16_t _y, uint8_t _screen);
  Numpad();
  ~Numpad();

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch, PressedKeyType *pressedKey);

  void drawKeys(bool fullScreen, bool onlyContent);
  void redraw() override;
  void forceUpdate() override;
  
  bool setup();
  void setup(const NumpadConfig& config);
  
  void open(NumberBox *_field);
  void close();
  void insertChar(char c);

  void show() override;
  void hide() override;
  
  // Internal onRelease system for key visual feedback
  struct KeyState {
    uint16_t row;        ///< Row index of pressed key
    uint16_t col;        ///< Column index of pressed key
    bool isPressed;      ///< True if a key is currently pressed
  };
  
  // Public methods for external access
  bool isKeyPressed() const { WidgetStateGuard guard; return m_pressedKey.isPressed; }
  KeyState getPressedKey() const { WidgetStateGuard guard; return m_pressedKey; }
  bool isPointInKey(uint16_t xTouch, uint16_t yTouch, const KeyState& keyState);
  void releaseKey();

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  const uint16_t aRows = NROWS; ///< Número de linhas no Numpad.
  const uint16_t aCols = NCOLS; ///< Número de colunas no Numpad.
  
  uint32_t m_yStart; ///< Ponto de partida da coordenada Y para o layout do Numpad.
  uint32_t m_keyW; ///< Largura de teclas individuais no Numpad.
  uint32_t m_keyH; ///< Altura de teclas individuais no Numpad.
  unsigned long m_myTime; ///< Timestamp para manipulação de funções relacionadas a tempo.
  bool m_shouldRedraw; ///< Flag indicando se o numpad deve ser redesenhado.
  static const Key_t m_pad[4][4]; ///< Array 2D definindo os caracteres exibidos nas teclas do Numpad.
  int32_t m_screenW; ///< Largura de tela disponível para o Numpad.
  int32_t m_screenH; ///< Altura de tela disponível para o Numpad.
  uint16_t m_availableWidth; ///< Largura disponível para desenhar o Numpad.
  uint16_t m_availableHeight; ///< Altura disponível para desenhar o Numpad.
  CharString m_content; ///< Mantém o conteúdo atual sendo inserido no Numpad.
  Rect_t m_pontoPreview; ///< Área de preview para exibir a entrada.
  TextBound_t m_lastArea; ///< Última área calculada para o rótulo.
  NumpadConfig m_config; ///< Estrutura contendo configuração do Numpad.
  CoordPoint_t m_backKeyPos = {0,0}; ///< Posição da tecla de retorno.

  char m_previousContent[MAX_LENGTH_CSTR + 1];
  
  #if defined(USING_GRAPHIC_LIB)
  GFXfont *m_fontKeys; ///< Fonte para as teclas.
  GFXfont *m_fontPreview; ///< Fonte para a área de preview.
  #endif
  
  void addLetter(char c);
  void removeLetter();
  void cleanupMemory();
  
  KeyState m_pressedKey; ///< State of currently pressed key
  
  void drawSingleKey(uint16_t row, uint16_t col, bool isPressed);
  void drawBackKey(uint16_t x, uint16_t y);
  void pressKey(uint16_t row, uint16_t col);
  
};

#endif