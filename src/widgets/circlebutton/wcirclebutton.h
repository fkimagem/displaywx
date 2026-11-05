//#ifndef WCIRCLEBUTTON
//#define WCIRCLEBUTTON
#pragma once
#include "../widgetbase.h"

/// @brief Estrutura de configuração para o CircleButton.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um botão circular.
///          Deve ser preenchida e passada para o método setup().
struct CircleButtonConfig {
  functionCB_t callback; ///< Função callback para executar quando o botão é pressionado.
  uint16_t radius; ///< Raio do botão circular em pixels. Recomendado: 5-200.
  uint16_t pressedColor; ///< Cor RGB565 exibida quando o botão está pressionado.
};

/// @brief Widget de botão circular interativo que permite controle de estados pressionado/não pressionado.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          botões circulares interativos na tela. O CircleButton desenha um círculo com borda dupla e
///          feedback visual quando pressionado. O widget pode ser configurado com diferentes raios, cores
///          e callbacks para responder a interações do usuário. O botão é totalmente funcional com suporte
///          a toque em displays capacitivos e touchscreen, detectando toques dentro da área circular.
class CircleButton : public WidgetBase
{
public:
  CircleButton(uint16_t _x, uint16_t _y, uint8_t _screen);
  virtual ~CircleButton();
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  void show() override;
  void hide() override;
  void setup(const CircleButtonConfig& config);
  void onClick();
  bool getStatus() const;
  void setStatus(bool _status);

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  bool m_status; ///< Status atual do botão: true = pressionado, false = não pressionado.
  CircleButtonConfig m_config; ///< Estrutura contendo configuração completa do botão (raio, cor, callback).
  
  void start();
  void changeState();
};

//#endif