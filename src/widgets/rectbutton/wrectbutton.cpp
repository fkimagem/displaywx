#include "wrectbutton.h"

const char* RectButton::TAG = "RectButton";

/**
 * @brief Construtor para a classe RectButton.
 * @param _x Coordenada X para a posição do RectButton.
 * @param _y Coordenada Y para a posição do RectButton.
 * @param _screen Identificador da tela onde o RectButton será exibido.
 * @details Inicializa o widget com valores padrão (status falso, enabled true) e configuração vazia.
 *          O botão não será funcional até que setup() seja chamado.
 */
RectButton::RectButton(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_status(false), m_enabled(true) {
      m_config = {.callback = nullptr, .width = 0, .height = 0, .pressedColor = 0};
      ESP_LOGD(TAG, "RectButton created at (%d, %d) on screen %d", _x, _y, _screen);
    }

/**
 * @brief Destrutor da classe RectButton.
 * @details Limpa quaisquer recursos usados pelo RectButton.
 */
RectButton::~RectButton() {
    cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo RectButton.
 * @details RectButton não usa alocação dinâmica de memória.
 */
void RectButton::cleanupMemory() {
    ESP_LOGD(TAG, "RectButton memory cleanup completed");
}

/**
 * @brief Detecta se o RectButton foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do RectButton, False caso contrário.
 * @details Muda o estado do botão se tocado e define a flag de redesenho:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento e debounce
 *          - Verifica se botão está habilitado (retorna false se desabilitado)
 *          - Detecta se toque está dentro dos bounds do botão
 *          - Chama changeState() e marca para redesenho se toque detectado
 */
bool RectButton::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL

  if (!m_enabled) {
    ESP_LOGD(TAG, "RectButton is disabled");
    return false;
  }

  

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
 * @brief Recupera a função callback associada ao RectButton.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o RectButton for utilizado.
 */
functionCB_t RectButton::getCallbackFunc() { return m_callback; }

/**
 * @brief Recupera o estado atual habilitado do botão.
 * @return True se o botão está habilitado, False caso contrário.
 * @details Retorna o valor de m_enabled.
 */
bool RectButton::getEnabled() { return m_enabled; }

/**
 * @brief Define o estado habilitado do botão.
 * @param newState True para habilitar o botão, False para desabilitá-lo.
 * @details Atualiza o valor de m_enabled.
 */
void RectButton::setEnabled(bool newState) { m_enabled = newState; }

/**
 * @brief Muda o estado atual do botão (pressionado ou solto).
 * @details Inverte o estado atual do botão usando m_status = !m_status.
 */
void RectButton::changeState() { m_status = !m_status; }

/**
 * @brief Simula um clique no RectButton.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o estado.
 * @details Inverte o estado, marca o redesenho e enfileira o callback se solicitado.
 */
void RectButton::onClick(bool triggerCallback)
{
  if (!m_loaded)
  {
    ESP_LOGE(TAG, "RectButton widget not loaded");
    return;
  }

  changeState();
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "RectButton clicked");
  if (m_callback != nullptr && triggerCallback)
  {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
}

/**
 * @brief Força o botão a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void RectButton::forceUpdate() { m_shouldRedraw = true; }

/**
 * @brief Redesenha o botão na tela, atualizando sua aparência.
 * @details Exibe o botão com aparência diferente baseado em seu estado atual.
 *          Apenas redesenha se o botão estiver na tela atual e precisar atualização:
 *          - Valida TFT, visibilidade, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Desenha fundo do botão com bordas arredondadas
 *          - Desenha borda do botão
 *          - Desenha área interna com cor diferente baseado no status (pressed ou background)
 */
void RectButton::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  #if defined(DISP_DEFAULT)

  // uint16_t darkBg = WidgetBase::lightMode ? CFK_GREY3 : CFK_GREY11;
  uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  WidgetBase::objTFT->fillRoundRect(m_xPos + 1, m_yPos + 1, m_config.width - 2, m_config.height - 2,
                                    5, lightBg); // Botao
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, 5,
                                    baseBorder); // borda Botao
  uint16_t bgColor = m_status ? m_config.pressedColor : lightBg;
  WidgetBase::objTFT->fillRoundRect(m_xPos + 6, m_yPos + 6, m_config.width - 12, m_config.height - 12,
                                    5, bgColor); // top botao
  WidgetBase::objTFT->drawRoundRect(m_xPos + 5, m_yPos + 5, m_config.width - 10, m_config.height - 10,
                                    5, baseBorder); // borda top botao

  #endif

}

/**
 * @brief Inicializa as configurações do botão.
 * @details Garante que as dimensões do botão estejam dentro de intervalos válidos baseado no tamanho do display.
 *          Marca o widget como carregado quando completo:
 *          - Restringe largura e altura dentro de bounds válidos (5 até largura/altura da tela)
 *          - Marca como carregado
 */
void RectButton::start() {
#if defined(DISP_DEFAULT)
  m_config.width = constrain(m_config.width, 5, WidgetBase::objTFT->width());
  m_config.height = constrain(m_config.height, 5, WidgetBase::objTFT->height());
#endif

  m_loaded = true;
}

/**
 * @brief Configura o RectButton com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref RectButtonConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia profundamente a configuração
 *          - Define callback da configuração
 *          - Marca o widget como carregado e inicializado
 *          O botão não será exibido corretamente até que este método seja chamado.
 */
void RectButton::setup(const RectButtonConfig &config) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGW(TAG, "RectButton already initialized");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();
  
  // Deep copy configuration
  m_config = config;
  
  // Set member variables from config
  m_callback = config.callback;
  
  m_loaded = true;
  
  ESP_LOGD(TAG, "RectButton configured: %dx%d, pressedColor=0x%04X", 
           m_config.width, m_config.height, m_config.pressedColor);
}

/**
 * @brief Recupera o status atual do botão.
 * @return True se o botão está pressionado, False caso contrário.
 * @details Retorna o valor de m_status.
 */
bool RectButton::getStatus() { return m_status; }

/**
 * @brief Define o status do botão.
 * @param _status Novo status do botão (true para pressionado, false para solto).
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o estado.
 * @details Atualiza o estado do botão, marca para redesenho e dispara o callback se solicitado:
 *          - Valida se widget está carregado
 *          - Atualiza m_status
 *          - Marca para redesenho
 *          - Dispara callback se configurado e triggerCallback for true
 */
void RectButton::setStatus(bool _status, bool triggerCallback) {
  if (!m_loaded) {
    ESP_LOGE(TAG, "RectButton widget not loaded");
    return;
  }

  m_status = _status;
  m_shouldRedraw = true;
  if (m_callback != nullptr && triggerCallback) {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
}

/**
 * @brief Torna o RectButton visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void RectButton::show() {
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o RectButton da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void RectButton::hide() {
  m_visible = false;
  m_shouldRedraw = true;
}