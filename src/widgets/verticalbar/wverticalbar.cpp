#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_VBAR)
#include "wverticalbar.h"
#include "../../extras/dwx_safe.h"

const char *VBar::TAG = "[VBar]";

/**
 * @brief Construtor do widget VBar.
 * @param _x Coordenada X para a posição do VBar.
 * @param _y Coordenada Y para a posição do VBar.
 * @param _screen Identificador da tela onde o VBar será exibido.
 * @details Inicializa o widget VBar com posição e tela especificadas.
 */
VBar::VBar(uint16_t _x, uint16_t _y, uint8_t _screen) : WidgetBase(_x, _y, _screen)
{
}

/**
 * @brief Limpa recursos alocados pelo VBar.
 * @details Desaloca memória dinâmica e remove referências a objetos.
 */
void VBar::cleanupMemory()
{
  WidgetStateGuard widgetStateGuard;
  // VBar não aloca memória dinâmica no momento
}

/**
 * @brief Destrutor do widget VBar.
 * @details Limpa recursos alocados.
 */
VBar::~VBar()
{
  cleanupMemory();
}

/**
 * @brief Detecta um toque no widget VBar.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque é detectado, False caso contrário.
 * @details Método override que sempre retorna false. O VBar não tem detecção de toque ativa.
 */
bool VBar::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  WidgetStateGuard widgetStateGuard;
  return false;
}

/**
 * @brief Recupera a função callback para o widget VBar.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o VBar for utilizado.
 */
functionCB_t VBar::getCallbackFunc()
{
  WidgetStateGuard widgetStateGuard;
  return m_callback;
}

/**
 * @brief Define o valor do widget VBar.
 * @param newValue Novo valor para definir.
 * @details Atualiza o valor da barra:
 *          - Restringe valor usando constrain() dentro da faixa min/max
 *          - Marca para redesenho
 */
void VBar::setValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  m_currentValue = constrain(newValue, m_config.minValue, m_config.maxValue);
  // Serial.println("ajusta currentValue: " + String(currentValue));
  m_shouldRedraw = true;
  // redraw();
}

/**
 * @brief Redesenha o widget VBar.
 * @details Atualiza a exibição da barra:
 *          - Valida TFT, visibilidade, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Calcula altura/largura proporcional usando dwx_map_clamped() baseado no valor atual
 *          - Atualiza área preenchida (limpando se valor diminuiu, preenchendo se aumentou)
 *          - Suporta orientação vertical e horizontal
 *          - Usa cantos arredondados conforme configurado
 *          - Armazena último valor e reseta flag de redesenho
 */
void VBar::redraw()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  #if defined(USING_GRAPHIC_LIB)

  int innerX = m_xPos + 1;
  int innerY = m_yPos + 1;
  int innerHeight = m_config.height - 2;
  int innerWidth = m_config.width - 2;
  int minHeight = m_config.round;
  int minWidth = m_config.round;
  int innerRound = m_config.round > 0 ? m_config.round - 1 : m_config.round;

  if (m_config.orientation == Orientation::VERTICAL)
  {
    uint32_t clearArea = 0;
    //uint32_t colorArea = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, minHeight, innerHeight);
    //uint32_t emptyArea = dwx_map_clamped(m_config.maxValue - m_currentValue, m_config.minValue, m_config.maxValue, minHeight, innerHeight);

    if (m_currentValue < m_lastValue)
    {
      clearArea = dwx_map_clamped(m_config.maxValue - m_currentValue + m_config.minValue, m_config.minValue, m_config.maxValue, minHeight, innerHeight);
    }
    
    if(m_changedScale){
      clearArea = innerHeight;
    }

    if(clearArea > 0){
      WidgetBase::objTFT->fillRoundRect(innerX, innerY, innerWidth, clearArea, innerRound, m_config.backgroundColor); // fundo total
    }

    uint32_t proportionalHeight = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, minHeight, innerHeight);
    WidgetBase::objTFT->fillRoundRect(innerX, innerY + (innerHeight - proportionalHeight), innerWidth, proportionalHeight, innerRound, m_config.filledColor); // cor fill
  }
  else if (m_config.orientation == Orientation::HORIZONTAL)
  {
    uint32_t clearArea = 0;
    uint32_t xValue = 0;

    if (m_currentValue < m_lastValue)
    {
      clearArea = dwx_map_clamped(m_config.maxValue - m_currentValue + m_config.minValue, m_config.minValue, m_config.maxValue, minWidth, innerWidth);
      xValue = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, innerX, innerX + innerWidth);
    }
    
    if(m_changedScale){
      clearArea = innerWidth;
      xValue = innerX;
    }

    if(clearArea > 0){
      WidgetBase::objTFT->fillRoundRect(xValue, innerY, clearArea, innerHeight, innerRound, m_config.backgroundColor); // fundo total
    }

    uint32_t proportionalWidth = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, minWidth, innerWidth);        // O +1 é para tirar a borda da contagem
    WidgetBase::objTFT->fillRoundRect(innerX, innerY, proportionalWidth, innerHeight, innerRound, m_config.filledColor); // cor fill
  }

  if (m_config.subtitle)
  {
    m_config.subtitle->setTextInt(m_currentValue);
  }

  if(m_changedScale){
    m_changedScale = false;
  }

  m_lastValue = m_currentValue;
  m_shouldRedraw = false;

  #endif
}

/** @brief Define o valor mínimo para o widget VBar
 * @details Define o valor mínimo para o widget VBar e marca para redesenho
 * @param newValue Novo valor para definir
 */
void VBar::setMinValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newValue == m_config.maxValue) return;
  m_config.minValue = newValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}

/** @brief Define o valor máximo para o widget VBar
 * @details Define o valor máximo para o widget VBar e marca para redesenho
 * @param newValue Novo valor para definir
 */
void VBar::setMaxValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newValue == m_config.minValue) return;
  m_config.maxValue = newValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}

void VBar::sortValues()
{
  WidgetStateGuard widgetStateGuard;
  if (m_config.minValue > m_config.maxValue)
  {
    int temp = m_config.minValue;
    m_config.minValue = m_config.maxValue;
    m_config.maxValue = temp;
  }
}

void VBar::setScale(int newMinValue, int newMaxValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newMinValue == newMaxValue) return;
  m_config.minValue = newMinValue;
  m_config.maxValue = newMaxValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}

/** @brief Recupera o valor mínimo para o widget VBar
 * @details Recupera o valor mínimo para o widget VBar
 * @return Valor mínimo
 */
int VBar::getMinValue()
{
  WidgetStateGuard widgetStateGuard;
  return m_config.minValue;
}

/** @brief Recupera o valor máximo para o widget VBar
 * @details Recupera o valor máximo para o widget VBar
 * @return Valor máximo
 */
int VBar::getMaxValue()
{
  WidgetStateGuard widgetStateGuard;
  return m_config.maxValue;
}


/**
 * @brief Inicia o widget VBar.
 * @details Garante que a largura e altura da barra estejam dentro de intervalos válidos:
 *          - Restringe altura e largura dentro de bounds válidos (20 até dimensão da tela)
 */
void VBar::start()
{
  WidgetStateGuard widgetStateGuard;
#if defined(USING_GRAPHIC_LIB)
  m_config.height = constrain(m_config.height, 20, WidgetBase::objTFT->height());
  m_config.width = constrain(m_config.width, 20, WidgetBase::objTFT->width());
#endif
}

/**
 * @brief Força o widget VBar a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void VBar::forceUpdate()
{
  WidgetStateGuard widgetStateGuard;
  m_shouldRedraw = true;
}

/**
 * @brief Desenha o fundo do widget VBar.
 * @details Desenha o container visual da barra incluindo fundo e borda:
 *          - Valida TFT e visibilidade
 *          - Desenha fundo com cantos arredondados
 *          - Desenha borda ao redor da barra
 */
void VBar::drawBackground()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  #if defined(USING_GRAPHIC_LIB)
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_config.round, m_config.backgroundColor); // fundo total
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_config.round, m_config.borderColor);  // borda total
  #endif
}

/**
 * @brief Configura o VBar com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref VerticalBarConfig contendo os parâmetros de configuração.
 * @details Chama método setup() com parâmetros individuais da configuração.
 */
void VBar::setup(const VerticalBarConfig &config)
{
  WidgetStateGuard widgetStateGuard;
  if (!WidgetBase::objTFT)
  {
    ESP_LOGE(TAG, "TFT not defined on WidgetBase");
    return;
  }
  if (m_loaded)
  {
    ESP_LOGD(TAG, "VBar widget already configured");
    return;
  }
  if (config.minValue >= config.maxValue || config.width < 5 || config.height < 5 ||
      uint32_t(m_xPos) + config.width > INT16_MAX || uint32_t(m_yPos) + config.height > INT16_MAX) return;
  m_config = config;
  start();
  m_loaded = true;
  m_initialized = true;
}

/**
 * @brief Torna o VBar visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void VBar::show()
{
  WidgetStateGuard widgetStateGuard;
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o VBar da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void VBar::hide()
{
  WidgetStateGuard widgetStateGuard;
  m_visible = false;
  m_shouldRedraw = true;
}
#endif // DFK_DISABLE_VBAR
