#include "wtoggle.h"

const char* ToggleButton::TAG = "[ToggleButton]";

/**
 * @brief Construtor do widget ToggleButton.
 * @param _x Coordenada X para a posição do ToggleButton.
 * @param _y Coordenada Y para a posição do ToggleButton.
 * @param _screen Identificador da tela onde o ToggleButton será exibido.
 * @details Inicializa o widget ToggleButton com posição e tela especificadas.
 */
ToggleButton::ToggleButton(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen)
{
}

/**
 * @brief Limpa recursos alocados pelo ToggleButton.
 * @details Desaloca memória dinâmica e remove referências a objetos.
 */
void ToggleButton::cleanupMemory() {
  
}

/**
 * @brief Destrutor do widget ToggleButton.
 * @details Limpa o ponteiro da função callback e libera recursos alocados.
 */
ToggleButton::~ToggleButton()
{
  cleanupMemory();
  m_callback = nullptr;
}

/**
 * @brief Detecta se o ToggleButton foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do ToggleButton, False caso contrário.
 * @details Quando tocado, alterna o estado do botão e marca para redesenho:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento, debounce, habilitado, bloqueado e ponteiro de toque
 *          - Detecta se toque está dentro dos bounds do ToggleButton
 *          - Altera estado usando changeState() se toque detectado
 *          - Marca para redesenho e retorna true
 */
bool ToggleButton::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  CHECK_VISIBLE_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL
  CHECK_POINTER_TOUCH_NULL_BOOL

  bool inBounds = POINT_IN_RECT(*_xTouch, *_yTouch, m_xPos, m_yPos, m_config.width, m_config.height);

  if(inBounds) {
    m_myTime = millis();
    setPressed(true);  // Mark widget as pressed
    changeState();
    m_shouldRedraw = true;
    return true;
  }
  return false;
}

/**
 * @brief Recupera a função callback associada ao ToggleButton.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o ToggleButton for alternado.
 */
functionCB_t ToggleButton::getCallbackFunc() { return m_callback; }

/**
 * @brief Altera o estado atual do botão (ligado ou desligado).
 * @details Inverte o estado atual do botão toggle:
 *          - Loga mudança de estado
 *          - Alterna m_status de true para false ou vice-versa
 */
void ToggleButton::changeState()
{
  ESP_LOGD(TAG, "Changing state from %d to %d", m_status, !m_status);
  m_status = !m_status;
}

/**
 * @brief Força o ToggleButton a redesenhar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void ToggleButton::forceUpdate() { m_shouldRedraw = true; }

/**
 * @brief Redesenha o ToggleButton na tela, atualizando sua aparência baseada no estado.
 * @details Desenha o botão toggle com uma bola deslizante:
 *          - Valida TFT, visibilidade, tela atual, carregamento e flag de redesenho
 *          - Escolhe cores baseadas no modo claro/escuro
 *          - Desenha fundo do botão com cor dependente do estado
 *          - Desenha borda
 *          - Calcula posição da bola deslizante (esquerda quando desligado, direita quando ligado)
 *          - Desenha bola com bordas
 *          - Limpa buffer do canvas se existir
 */
void ToggleButton::redraw()
{
  CHECK_TFT_VOID
  if (!m_visible)
  {
    return;
  }
#if defined(DISP_DEFAULT)
  if (WidgetBase::currentScreen != m_screen || !m_loaded || !m_shouldRedraw)
  {
    return;
  }
  m_shouldRedraw = false;
  // uint16_t darkBg = WidgetBase::lightMode ? CFK_GREY3 : CFK_GREY11;
  uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;
  uint16_t statusBall = m_status ? CFK_WHITE : CFK_GREY3;

  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_config.height / 2,
                                    m_status ? m_config.pressedColor
                                             : lightBg); // fundo
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_config.height / 2,
                                    baseBorder); // borda

  uint8_t offsetBorder = 5;

  uint8_t raioBall = (m_config.height - (2 * offsetBorder)) / 2;
  uint16_t posBall = m_xPos;

  if (m_status)
  {
    posBall = m_xPos + m_config.width - offsetBorder - raioBall;
  }
  else
  {
    posBall = m_xPos + offsetBorder + raioBall;
  }

  WidgetBase::objTFT->fillCircle(posBall, m_yPos + m_config.height / 2, raioBall,
                                 statusBall); // circulo
  WidgetBase::objTFT->drawCircle(posBall, m_yPos + m_config.height / 2, raioBall,
                                 baseBorder); // circulo
  /*
  if(m_status){
    WidgetBase::objTFT->fillRect(50,50, 20, 20, CFK_CHOCOLATE);
  }else{
    for (int r = 0; r < 20; r++)
    {
      blitRowFromCanvas(50, 50 + r, 20);
    }
  }*/
#endif
}

/**
 * @brief Copia uma linha do framebuffer do canvas para o TFT.
 * @param x Coordenada X da linha para copiar.
 * @param y Coordenada Y da linha para copiar.
 * @param w Largura da linha para copiar.
 * @details Copia dados do buffer do canvas para o display TFT.
 */
void ToggleButton::blitRowFromCanvas(int x, int y, int w)
{
  
}

/**
 * @brief Inicia o widget ToggleButton.
 * @details Garante que a largura do botão esteja dentro de intervalos válidos:
 *          - Restringe largura dentro de bounds válidos (30 até largura da tela)
 */
void ToggleButton::start()
{
  CHECK_TFT_VOID
#if defined(DISP_DEFAULT)
  m_config.width = constrain(m_config.width, 30, WidgetBase::objTFT->width());
#endif
}

/**
 * @brief Configura o ToggleButton com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref ToggleButtonConfig contendo os parâmetros de configuração do botão.
 * @details Chama método setup() com parâmetros individuais da configuração.
 */
void ToggleButton::setup(const ToggleButtonConfig &config)
{
  if (!WidgetBase::objTFT)
  {
    ESP_LOGE(TAG, "TFT not defined on WidgetBase");
    return;
  }
  if (m_loaded)
  {
    ESP_LOGD(TAG, "ToggleButton widget already configured");
    return;
  }
  m_config = config;
  m_callback = config.callback;
  start();

    m_loaded = true;
    m_initialized = true;
}

/**
 * @brief Recupera o status atual do botão.
 * @return True se o botão está ligado, False caso contrário.
 * @details Consulta se o botão está no estado ligado ou desligado.
 */
bool ToggleButton::getStatus() { return m_status; }

/**
 * @brief Define o estado atual do botão toggle.
 * @param status True para ligado, False para desligado.
 * @param triggerCallback True para disparar callback, False para não disapar callback.
 * @details Atualiza o estado do botão, marca para redesenho e dispara callback se fornecido:
 *          - Valida se widget está carregado
 *          - Define novo status e marca para redesenho
 *          - Dispara callback se não for nullptr
 */
void ToggleButton::setStatus(bool status, bool triggerCallback)
{
  if (!m_loaded)
  {
    ESP_LOGE(TAG, "ToggleButton widget not loaded");
    return;
  }
  ESP_LOGD(TAG, "Setting status to %d", status);
  m_status = status;
  m_shouldRedraw = true;
  if (m_callback != nullptr && triggerCallback)
  {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
}

/**
 * @brief Torna o ToggleButton visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void ToggleButton::show()
{
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o ToggleButton da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void ToggleButton::hide()
{
  m_visible = false;
  m_shouldRedraw = true;
}
