#ifndef WCircularBar
#define WCircularBar

#include "../widgetbase.h"

#if defined(DISP_DEFAULT)
#include "../../fonts/RobotoRegular/RobotoRegular10pt7b.h"
#endif

/// @brief Estrutura de configuração para o CircularBar.
struct CircularBarConfig {
  int minValue;               ///< Valor mínimo da faixa da barra.
  int maxValue;               ///< Valor máximo da faixa da barra.
  uint16_t radius;            ///< Raio da barra circular em pixels.
  uint16_t startAngle;        ///< Ângulo inicial da barra circular em graus (0-360).
  uint16_t endAngle;          ///< Ângulo final da barra circular em graus (0-360).
  uint16_t color;             ///< Cor RGB565 da barra circular quando preenchida.
  uint16_t backgroundColor;   ///< Cor RGB565 de fundo da barra circular.
  uint16_t textColor;         ///< Cor RGB565 do texto que exibe o valor.
  uint16_t backgroundText;    ///< Cor RGB565 do fundo da área do texto.
  uint8_t thickness;          ///< Espessura da linha da barra circular em pixels.
  bool showValue;             ///< Flag para mostrar/ocultar o texto do valor.
  bool inverted;              ///< Flag para inverter a direção de preenchimento.
#if defined(USING_GRAPHIC_LIB)
  const GFXfont* font = nullptr; ///< Fixed font chosen by the editor; no automatic font fitting.
#endif
};

class CircularBar : public WidgetBase
{
public:
  CircularBar(uint16_t _x, uint16_t _y, uint8_t _screen);
  virtual ~CircularBar();

  bool detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) override;
  functionCB_t getCallbackFunc() override;

  void redraw() override;
  void forceUpdate() override;
  void show() override;
  void hide() override;

  void setup(const CircularBarConfig& config);
  void drawBackground();

  void setValue(int newValue);
  void setMinValue(int newValue);
  void setMaxValue(int newValue);
  int getMinValue();
  int getMaxValue();
  void setScale(int newMinValue, int newMaxValue);

private:
  static const char* TAG;

  int m_lastValue;   ///< Último valor efetivamente desenhado na tela.
  int m_value;       ///< Valor alvo para o próximo desenho.
  
  CircularBarConfig m_config;
  bool m_changedScale = false;

  void sortValues();
};

#endif