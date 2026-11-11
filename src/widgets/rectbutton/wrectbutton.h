#ifndef WRECTBUTTON
#define WRECTBUTTON

#include "../widgetbase.h"

/// @brief Estrutura de configuração para o RectButton.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um botão retangular.
///          Deve ser preenchida e passada para o método setup().
struct RectButtonConfig {
  functionCB_t callback;  ///< Função callback para executar quando o botão é interagido.
  uint16_t width;         ///< Largura do botão.
  uint16_t height;        ///< Altura do botão.
  uint16_t pressedColor;  ///< Cor exibida quando o botão está pressionado.
};

/// @brief Widget de botão retangular com tamanho e cor personalizáveis.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          botões retangulares interativos na tela. O RectButton desenha um botão com bordas arredondadas
///          que muda de aparência quando pressionado, mostrando uma cor diferente para o estado ativo.
///          O widget pode ser configurado com diferentes larguras, alturas, cores para estado pressionado
///          e callbacks. O botão é totalmente funcional com suporte a estados habilitado/desabilitado,
///          detecção de toque e mudança de estado visual.
class RectButton : public WidgetBase
{
public:
  RectButton(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~RectButton();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void setup(const RectButtonConfig& config);
  void onClick(bool triggerCallback = true);
  void changeState();
  bool getStatus();
  void setStatus(bool _status, bool triggerCallback = true);
  bool getEnabled();
  void setEnabled(bool newState);
  
  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  bool m_status; ///< Status atual ligado/desligado do botão.
  bool m_enabled; ///< Indica se o botão está habilitado ou desabilitado.
  RectButtonConfig m_config; ///< Estrutura contendo configuração do botão retangular.
  
  void cleanupMemory();
  void start();
};

#endif