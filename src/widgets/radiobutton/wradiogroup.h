#ifndef WRADIOGROUP
#define WRADIOGROUP

#include "../widgetbase.h"

/// @brief Estrutura representando um botão de rádio único com posição, ID e cor.
struct radio_t
{
  uint16_t x; ///< Coordenada X do botão de rádio.
  uint16_t y; ///< Coordenada Y do botão de rádio.
  uint16_t color; ///< Cor do botão de rádio.
  uint8_t id; ///< Identificador do botão de rádio dentro do grupo.
};

/// @brief Estrutura de configuração para o RadioGroup.
/// @details Esta estrutura contém todos os parâmetros necessários para configurar um grupo de botões de rádio.
///          Deve ser preenchida e passada para o método setup().
struct RadioGroupConfig {
  const radio_t* buttons;      ///< Ponteiro para um array definindo os botões de rádio.
  functionCB_t callback;       ///< Função callback para executar na mudança de seleção.
  uint16_t radius;             ///< Raio de cada botão de rádio.
  uint8_t group;               ///< Identificador do grupo para os botões de rádio.
  uint8_t amount;              ///< Número de botões de rádio no grupo.
  uint8_t defaultClickedId;    ///< ID do botão de rádio a ser selecionado por padrão.
};

/// @brief Widget de grupo de botões de rádio que permite seleção de uma opção dentro do grupo.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          grupos de botões de rádio interativos na tela. O RadioGroup desenha uma coleção de botões
///          de rádio onde apenas um pode estar selecionado por vez, com deseleção automática de outros.
///          O widget pode ser configurado com diferentes grupos, números de botões, raios, cores e
///          callbacks. O RadioGroup é totalmente funcional com suporte a detecção de toque, seleção
///          programática e integração com sistema de callbacks da biblioteca.
class RadioGroup : public WidgetBase
{
public:
  RadioGroup(uint8_t _screen);
  ~RadioGroup();
  
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  
  void setup(const RadioGroupConfig& config);
  void setSelected(uint16_t clickedId, bool triggerCallback = true);
  uint16_t getSelected();
  uint16_t getGroupId();

  void show() override;
  void hide() override;

private:
  static const char* TAG; ///< Tag estática para identificação em logs do ESP32.
  
  radio_t *m_buttons; ///< Ponteiro para um array de definições de botões de rádio.
  uint8_t m_clickedId; ///< ID do botão de rádio atualmente selecionado.
  bool m_shouldRedraw; ///< Flag para indicar se os botões de rádio devem ser redesenhados.
  RadioGroupConfig m_config; ///< Estrutura contendo configuração do grupo de botões de rádio.
  
  void cleanupMemory();
};

#endif