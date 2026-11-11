#ifndef WSLIDER
#define WSLIDER

#include "../widgetbase.h"
#include <Arduino.h>
#include <esp_log.h>
#include "../label/wlabel.h" // Para ponteiro Label

/// @brief Estrutura de configuração para o HSlider.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um slider horizontal.
///          Deve ser preenchida e passada para o método setup().
struct HSliderConfig {
  functionCB_t callback;  ///< Função callback para executar quando o valor do slider muda.
  Label* subtitle;
  int minValue;           ///< Valor mínimo da faixa do slider.
  int maxValue;           ///< Valor máximo da faixa do slider.
  uint32_t radius;        ///< Raio do controle deslizante em pixels.
  uint16_t width;         ///< Largura da trilha do slider em pixels.
  uint16_t pressedColor;  ///< Cor RGB565 exibida quando o slider está pressionado.
  uint16_t backgroundColor; ///< Cor RGB565 de fundo do slider.
};

/// @brief Widget de slider horizontal interativo que permite controle de valores através de deslizamento.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          sliders horizontais interativos na tela. O HSlider desenha uma trilha horizontal com um controle
///          deslizante que pode ser movido pelo usuário para ajustar valores dentro de uma faixa especificada.
///          O widget pode ser configurado com diferentes larguras, cores, faixas de valores e callbacks.
///          O slider é totalmente funcional com suporte a toque em displays capacitivos e touchscreen,
///          detectando toques na área da trilha e movendo o controle proporcionalmente.
class HSlider : public WidgetBase
{
public:
  HSlider(uint16_t _x, uint16_t _y, uint8_t _screen);
  virtual ~HSlider();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  void show() override;
  void hide() override;
  
  void setup(const HSliderConfig& config);
  int getValue() const;
  void setValue(int newValue, bool triggerCallback = true);
  void drawBackground();

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  HSliderConfig m_config; ///< Estrutura contendo configuração completa do slider (largura, cores, faixa, callback).
  uint16_t m_currentPos; ///< Posição atual do controle deslizante em pixels.
  uint16_t m_lastPos; ///< Última posição registrada do controle deslizante em pixels.
  uint32_t m_height; ///< Altura da trilha do slider em pixels.
  uint8_t m_contentRadius; ///< Raio da área de conteúdo dentro do controle deslizante.
  uint16_t m_centerV; ///< Posição vertical central da trilha do slider.
  uint16_t m_minX; ///< Coordenada X mínima para a posição do controle deslizante.
  uint16_t m_maxX; ///< Coordenada X máxima para a posição do controle deslizante.
  int m_value; ///< Valor atual do slider dentro da faixa configurada.
  
  void start();
  bool validateConfig(const HSliderConfig& config);
  void updateDimensions();
  void updateValue();
};
#endif