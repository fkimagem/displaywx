#ifndef WTOGGLE
#define WTOGGLE

#include "../widgetbase.h"

/// @brief Estrutura de configuração para o ToggleButton.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um botão toggle.
///          Deve ser preenchida e passada para o método setup().
struct ToggleButtonConfig {
  functionCB_t callback;  ///< Função callback para executar quando o botão é alternado.
  uint16_t width;         ///< Largura do botão.
  uint16_t height;        ///< Altura do botão.
  uint16_t pressedColor;  ///< Cor exibida quando o botão está ligado.
};

/// @brief Representa um widget de botão toggle que alterna entre estado ligado e desligado a cada toque.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar
///          botões toggle interativos na tela. O ToggleButton desenha um botão com uma bola deslizante
///          que move da esquerda para direita quando ligado e da direita para esquerda quando desligado.
///          O widget pode ser configurado com diferentes tamanhos, cores e callbacks.
///          O botão toggle é totalmente funcional com suporte a toque em displays capacitivos e touchscreen,
///          alternando o estado visual e disparando callbacks.
class ToggleButton : public WidgetBase
{
public:
  ToggleButton(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~ToggleButton();
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void changeState();
  void redraw() override;
  void forceUpdate() override;
  void setup(const ToggleButtonConfig& config);
  bool getStatus();
  void setStatus(bool status, bool triggerCallback = true);
  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  bool m_status; ///< Status atual ligado/desligado do botão.
  ToggleButtonConfig m_config; ///< Estrutura de configuração para o ToggleButton.

  void cleanupMemory();
  void start();
  void setup(uint16_t _width, uint16_t _height, uint16_t _pressedColor, functionCB_t _cb);
  void blitRowFromCanvas(int x, int y, int w);
};
#endif