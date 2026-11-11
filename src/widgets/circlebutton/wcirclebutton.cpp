#include "wcirclebutton.h"
#include <esp_log.h>

const char* CircleButton::TAG = "CircleButton";

/**
 * @brief Construtor da classe CircleButton.
 * @param _x Coordenada X da posição central do botão na tela.
 * @param _y Coordenada Y da posição central do botão na tela.
 * @param _screen Identificador da tela onde o botão será exibido.
 * @details Inicializa o botão circular com valores padrão: status não pressionado, raio 0,
 *          callback nullptr. O botão deve ser configurado com setup() antes de ser exibido.
 */
CircleButton::CircleButton(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_status(false)
{
    m_config = {.callback = nullptr, .radius = 0, .pressedColor = 0};
    ESP_LOGD(TAG, "CircleButton created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe CircleButton.
 * @details Registra o evento no log do ESP32 e limpa o ponteiro da função callback.
 *          Todos os recursos são liberados automaticamente pela herança de @ref WidgetBase.
 */
CircleButton::~CircleButton() {
    ESP_LOGD(TAG, "CircleButton destroyed at (%d, %d)", m_xPos, m_yPos);
    if (m_config.callback != nullptr) { m_config.callback = nullptr; }
}

/**
 * @brief Detecta se o CircleButton foi tocado pelo usuário.
 * @param _xTouch Ponteiro para a coordenada X do toque na tela.
 * @param _yTouch Ponteiro para a coordenada Y do toque na tela.
 * @return True se o toque está dentro da área circular do botão, False caso contrário.
 * @details Este método realiza múltiplas validações antes de processar o toque:
 *          - Verifica se o widget está visível, inicializado, carregado e habilitado
 *          - Valida que o teclado virtual não está ativo
 *          - Verifica que o botão está na tela atual
 *          - Aplica debounce para evitar múltiplos cliques
 *          - Verifica se o widget não está bloqueado
 *          - Usa detecção de ponto dentro de círculo para validar o toque
 *          Se todas as validações passarem e o toque estiver dentro da área circular,
 *          o estado é alternado e o botão é marcado para redesenho.
 */
bool CircleButton::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  // Early validation checks using macros
  CHECK_VISIBLE_BOOL
  CHECK_INITIALIZED_BOOL
  CHECK_LOADED_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL
  CHECK_POINTER_TOUCH_NULL_BOOL

#if defined(HAS_TOUCH)
  
  bool inBounds = POINT_IN_CIRCLE(*_xTouch, *_yTouch, m_xPos, m_yPos, m_config.radius);

  if (inBounds) {
    m_myTime = millis();
    setPressed(true);  // Mark widget as pressed
    changeState();
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "CircleButton touched at (%d, %d), new status: %s", 
                  *_xTouch, *_yTouch, m_status ? "pressed" : "released");
    return true;
  }
  return false;
#else
  return false;
#endif
}

/**
 * @brief Recupera a função callback associada ao CircleButton.
 * @return Ponteiro para a função callback, ou nullptr se nenhuma foi definida.
 * @details A função callback é executada quando o estado do botão muda.
 *          Para mais informações sobre callbacks, consulte @ref WidgetBase.
 */
functionCB_t CircleButton::getCallbackFunc() { return m_config.callback; }

/**
 * @brief Força uma atualização imediata do CircleButton.
 * @details Define a flag m_shouldRedraw para true, forçando o redesenho do botão
 *          no próximo ciclo de atualização da tela. Útil para garantir que mudanças
 *          no estado ou configuração sejam visíveis imediatamente.
 */
void CircleButton::forceUpdate() { 
  m_shouldRedraw = true; 
  ESP_LOGD(TAG, "CircleButton force update requested");
}


/**
 * @brief Alterna o estado atual do CircleButton entre pressionado e não pressionado.
 * @details Inverte o valor do atributo m_status. Este método é chamado internamente
 *          por detectTouch() quando o botão é tocado pelo usuário.
 */
void CircleButton::changeState() { m_status = !m_status; }

/**
 * @brief Redesenha o CircleButton na tela, atualizando sua aparência baseada em seu estado.
 * @details Este método é responsável por renderizar o botão circular na tela:
 *          - Verifica todas as condições necessárias para o redesenho
 *          - Desenha círculo externo com fundo e borda
 *          - Desenha círculo interno com cor diferente quando pressionado
 *          - Usa a biblioteca @ref Arduino_GFX_Library para o desenho
 *          - Cores são ajustadas automaticamente baseadas no modo claro/escuro
 *          Apenas redesenha se o botão está visível, inicializado, carregado, na tela atual
 *          e a flag m_shouldRedraw está configurada como true.
 */
void CircleButton::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_SHOULDREDRAW_VOID

#if defined(DISP_DEFAULT)
  m_shouldRedraw = false;

  uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  ESP_LOGD(TAG, "Redrawing circlebutton at (%d,%d) radius %d, status: %s", 
                m_xPos, m_yPos, m_config.radius, m_status ? "pressed" : "released");

  WidgetBase::objTFT->fillCircle(m_xPos, m_yPos, m_config.radius,
                                 lightBg); // Button background
  WidgetBase::objTFT->drawCircle(m_xPos, m_yPos, m_config.radius,
                                 baseBorder); // Button border
  uint16_t bgColor = m_status ? m_config.pressedColor : lightBg;
  WidgetBase::objTFT->fillCircle(m_xPos, m_yPos, m_config.radius * 0.75,
                                 bgColor); // Inner background
  WidgetBase::objTFT->drawCircle(m_xPos, m_yPos, m_config.radius * 0.75,
                                 baseBorder); // Inner border
#endif
}

/**
 * @brief Inicializa o CircleButton com restrições de tamanho padrão.
 * @details Limita o raio do botão entre 5 e 200 pixels usando a função constrain().
 *          Valores abaixo de 5 são ajustados para 5, valores acima de 200 para 200.
 *          Este método é chamado internamente por setup() após a configuração.
 */
void CircleButton::start() {
  m_config.radius =
      constrain(m_config.radius, 5, 200); // Limits the button radius between 5 and 200
}


/**
 * @brief Configura o CircleButton com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref CircleButtonConfig contendo os parâmetros de configuração do botão.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Copia todas as configurações para m_config
 *          - Chama start() para aplicar restrições de tamanho
 *          - Marca o widget como inicializado e carregado
 *          - O botão não será exibido corretamente até que este método seja chamado
 */
void CircleButton::setup(const CircleButtonConfig &config) {
  // Validate TFT object
  CHECK_TFT_VOID
  
  m_config = config;
  start();
  // redraw();

  m_loaded = true;
  m_initialized = true;
  ESP_LOGD(TAG, "CircleButton setup completed at (%d, %d) with radius %d", 
                m_xPos, m_yPos, m_config.radius);
}

/**
 * @brief Recupera o status atual do CircleButton.
 * @return True se o botão está pressionado, False se está não pressionado.
 */
bool CircleButton::getStatus() const { return m_status; }

/**
 * @brief Simula um clique no CircleButton.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o estado.
 * @details Inverte o estado, marca o redesenho e enfileira o callback se solicitado.
 */
void CircleButton::onClick(bool triggerCallback)
{
  if (!m_loaded)
  {
    ESP_LOGE(TAG, "CircleButton widget not loaded");
    return;
  }

  changeState();
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "CircleButton clicked");
  if (m_config.callback != nullptr && triggerCallback)
  {
    WidgetBase::addCallback(m_config.callback, WidgetBase::CallbackOrigin::SELF);
  }
}

/**
 * @brief Define o estado atual do CircleButton.
 * @param _status True para pressionar o botão, False para liberar.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o estado.
 * @details Este método permite mudar programaticamente o estado do botão:
 *          - Verifica se o widget está carregado antes de fazer alterações
 *          - Evita atualizações desnecessárias se o estado já é o solicitado
 *          - Atualiza o atributo m_status
 *          - Marca o widget para redesenho
 *          - Adiciona o callback na fila se uma função foi definida e triggerCallback for true
 *          - Registra o evento no log do ESP32
 *          Útil para sincronizar o estado visual com valores de variáveis ou configurações.
 */
void CircleButton::setStatus(bool _status, bool triggerCallback) {
  CHECK_LOADED_VOID
  
  if (m_status == _status) {
    ESP_LOGD(TAG, "CircleButton widget already set to this status");
    return;
  }

  m_status = _status;
  m_shouldRedraw = true;

  if (m_config.callback != nullptr && triggerCallback) {
    WidgetBase::addCallback(m_config.callback, WidgetBase::CallbackOrigin::SELF);
  }
  ESP_LOGD(TAG, "CircleButton status changed to %s", _status ? "pressed" : "released");
}

/**
 * @brief Torna o botão circular visível na tela.
 * @details Define m_visible como true e marca para redesenho. O botão será
 *          desenhado na próxima chamada de redraw() se estiver na tela atual.
 */
void CircleButton::show() {
  m_visible = true;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "CircleButton shown at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Oculta o botão circular da tela.
 * @details Define m_visible como false e marca para redesenho. O botão não será
 *          mais desenhado até que show() seja chamado novamente.
 */
void CircleButton::hide() {
  m_visible = false;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "CircleButton hidden at (%d, %d)", m_xPos, m_yPos);
}