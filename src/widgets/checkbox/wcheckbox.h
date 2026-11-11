#ifndef WCHECKBOX
#define WCHECKBOX

#include "../widgetbase.h"

enum class CheckBoxWeight {
  LIGHT = 1,
  MEDIUM = 2,
  HEAVY = 3
};

/// @brief Configuration structure for CheckBox
struct CheckBoxConfig {
  functionCB_t callback;  ///< Callback function to execute when the CheckBox state changes
  CheckBoxWeight weight; ///< Weight of the checkbox (LIGHT, MEDIUM, HEAVY)
  uint16_t size;          ///< Size of the CheckBox (width and height will be equal)
  uint16_t checkedColor;  ///< Color displayed when the checkbox is checked
  uint16_t uncheckedColor; ///< Color displayed when the checkbox is unchecked (optional, 0 = auto)
};



/// @brief Widget de checkbox interativo que permite alternar entre estados marcado e desmarcado.
/// @details Esta classe herda de @ref WidgetBase e fornece funcionalidade completa para criar e gerenciar 
///          checkboxes interativos na tela. O CheckBox desenha um quadrado com bordas arredondadas e uma 
///          marca de verificação quando está no estado marcado. O widget pode ser configurado com diferentes
///          tamanhos, cores, pesos de linha para a marca e callbacks para responder a mudanças de estado.
///          O checkbox é totalmente funcional com suporte a toque em displays capacitivos e touchscreen.
class CheckBox : public WidgetBase
{
public:
  CheckBox(uint16_t _x, uint16_t _y, uint8_t _screen);
  ~CheckBox();
  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;
  void redraw() override;
  void forceUpdate() override;
  void setup(const CheckBoxConfig& config);
  bool getStatus() const;
  void setStatus(bool status, bool triggerCallback = true);
  void show() override;
  void hide() override;
  void setSize(uint16_t newSize);
  uint16_t getSize() const;
  void setColors(uint16_t checkedColor, uint16_t uncheckedColor = 0);

private:
  static const char *TAG; ///< Tag estática para identificação em logs do ESP32.
  bool m_status; ///< Status atual do checkbox: true = marcado, false = desmarcado.
  uint8_t m_borderWidth; ///< Largura da borda do checkbox em pixels.
  uint8_t m_borderRadius; ///< Raio dos cantos arredondados do checkbox em pixels.
  CheckBoxConfig m_config; ///< Estrutura contendo configuração completa do checkbox (tamanho, cores, peso, callback).
  CoordPoint_t m_topRightPoint; ///< Ponto superior direito para desenho da marca de verificação.
  CoordPoint_t m_bottomCenterPoint; ///< Ponto central inferior para desenho da marca de verificação.
  CoordPoint_t m_middleLeftPoint; ///< Ponto esquerdo médio para desenho da marca de verificação.

  void changeState();
  uint16_t getUncheckedColor();
  uint16_t getBorderColor();
  void drawCheckmark();
  void drawBorder();
};

#endif