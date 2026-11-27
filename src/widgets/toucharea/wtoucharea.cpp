#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_TOUCHAREA)
#include "wtoucharea.h"

const char* TouchArea::TAG = "[TouchArea]";

/**
 * @brief Construtor do widget TouchArea.
 * @param _x Coordenada X para a posição do TouchArea.
 * @param _y Coordenada Y para a posição do TouchArea.
 * @param _screen Identificador da tela onde o TouchArea será exibido.
 * @details Inicializa o widget TouchArea com posição e tela especificadas.
 */
TouchArea::TouchArea(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen) {}

/**
 * @brief Limpa recursos alocados pelo TouchArea.
 * @details Desaloca memória dinâmica e remove referências a objetos.
 */
void TouchArea::cleanupMemory() {
  WidgetStateGuard widgetStateGuard;
  // TouchArea não aloca memória dinâmica no momento
}

/**
 * @brief Destrutor do widget TouchArea.
 * @details Limpa recursos alocados.
 */
TouchArea::~TouchArea() {
  cleanupMemory();
}

/**
 * @brief Detecta se o TouchArea foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro do TouchArea, False caso contrário.
 * @details Quando tocado, registra o tempo do evento:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento, debounce, habilitado, bloqueado e ponteiro de toque
 *          - Detecta se toque está dentro dos bounds do TouchArea
 *          - Registra timestamp se toque detectado
 *          - Retorna resultado da detecção
 */
bool TouchArea::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  WidgetStateGuard widgetStateGuard;
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
  }

  return inBounds;
}

/**
 * @brief Recupera a função callback associada ao TouchArea.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o TouchArea for tocado.
 */
functionCB_t TouchArea::getCallbackFunc() {
  WidgetStateGuard widgetStateGuard; return m_callback; }

/**
 * @brief Redesenha o TouchArea na tela.
 * @details Implementação vazia pois o TouchArea é invisível por padrão:
 *          - Valida se o widget está visível
 *          - Não desenha nada na tela
 */
void TouchArea::redraw() {
  WidgetStateGuard widgetStateGuard;
  if (!m_visible) {
    return;
  }
}

/**
 * @brief Simula um clique no TouchArea.
 * @details Dispara a função callback como se a área fosse tocada:
 *          - Valida se widget está carregado
 *          - Adiciona callback à fila se não for nullptr
 *          - Útil para ativar a área de toque programaticamente
 */
void TouchArea::onClick() {
  WidgetStateGuard widgetStateGuard;
#if defined(DFK_TOUCHAREA)
  if (!m_loaded) {
    ESP_LOGE(TAG, "TouchArea widget not loaded");
    return;
  }
  if (m_callback != nullptr) {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
#endif
}


/**
 * @brief Configura o TouchArea com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref TouchAreaConfig contendo os parâmetros de configuração da área de toque.
 * @details Chama método setup() com parâmetros individuais da configuração.
 */
void TouchArea::setup(const TouchAreaConfig &config) {
  WidgetStateGuard widgetStateGuard;
  m_config = config;
  m_callback = config.callback;
  m_loaded = true;
  m_initialized = true;
}

/**
 * @brief Changes the current state of the touch area.
 *
 * Not implemented in the current version.
 */
void TouchArea::changeState() {
  WidgetStateGuard widgetStateGuard;
  // Not implemented
}

/**
 * @brief Recupera o status atual do TouchArea.
 * @return Status atual da área de toque.
 * @details Consulta o status interno da área de toque.
 */
bool TouchArea::getStatus() {
  WidgetStateGuard widgetStateGuard; return m_status; }

/**
 * @brief Torna o TouchArea visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void TouchArea::show() {
  WidgetStateGuard widgetStateGuard;
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o TouchArea da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void TouchArea::hide() {
  WidgetStateGuard widgetStateGuard;
  m_visible = false;
  m_shouldRedraw = true;
}

/**
 * @brief Força o TouchArea a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void TouchArea::forceUpdate() {
  WidgetStateGuard widgetStateGuard; m_shouldRedraw = true; }
#endif // DFK_DISABLE_TOUCHAREA
