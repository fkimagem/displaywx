#include "wradiogroup.h"

const char* RadioGroup::TAG = "RadioGroup";

/**
 * @brief Construtor para a classe RadioGroup.
 * @param _screen Identificador da tela onde o RadioGroup será exibido.
 * @details Cria um RadioGroup na posição (0,0) na tela especificada.
 *          Inicializa com configuração vazia e valores padrão.
 *          O RadioGroup não será funcional até que setup() seja chamado.
 */
RadioGroup::RadioGroup(uint8_t _screen)
    : WidgetBase(0, 0, _screen), m_shouldRedraw(true), m_config{} {
      m_config = {
        .buttons = nullptr,
        .callback = nullptr,
        .radius = 0,
        .group = 0,
        .amount = 0,
        .defaultClickedId = 0
      };
    }

/**
 * @brief Destrutor da classe RadioGroup.
 * @details Limpa quaisquer recursos usados pelo RadioGroup.
 */
RadioGroup::~RadioGroup() {
    cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo RadioGroup.
 * @details Desaloca arrays dinâmicos de botões e limpa referências:
 *          - Libera m_config.buttons se alocado
 *          - Libera m_buttons se alocado
 */
void RadioGroup::cleanupMemory() {
  if (m_config.buttons) {
    delete[] m_config.buttons;
    m_config.buttons = nullptr;
  }
  if (m_buttons) {
    delete[] m_buttons;
    m_buttons = nullptr;
  }
  ESP_LOGD(TAG, "RadioGroup memory cleanup completed");
}

/**
 * @brief Força o RadioGroup a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void RadioGroup::forceUpdate() { m_shouldRedraw = true; }

/**
 * @brief Detecta se algum botão de rádio dentro do grupo foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se um botão de rádio foi tocado, False caso contrário.
 * @details Verifica se o evento de toque ocorreu dentro de algum dos botões de rádio
 *          e atualiza o estado selecionado se um botão for tocado:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento e debounce
 *          - Itera sobre todos os botões e verifica se o toque está dentro do raio de cada um
 *          - Atualiza ID selecionado e marca para redesenho se toque for detectado
 */
bool RadioGroup::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL

  int offset_radius = 5;

  for (int16_t i = 0; i < m_config.amount; i++) {
    radio_t r = m_buttons[i];
    bool inBounds = POINT_IN_CIRCLE(*_xTouch, *_yTouch, r.x, r.y, m_config.radius + offset_radius);
    if(inBounds) {
      m_clickedId = r.id;
      setPressed(true);  // Mark widget as pressed
      m_shouldRedraw = true;
      m_myTime = millis();
      return true;
    } 

  }
  return false;
}

/**
 * @brief Recupera a função callback associada ao RadioGroup.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o RadioGroup for utilizado.
 */
functionCB_t RadioGroup::getCallbackFunc() { return m_callback; }

/**
 * @brief Redesenha os botões de rádio na tela, atualizando sua aparência.
 * @details Desenha todos os botões de rádio no grupo, destacando o atualmente selecionado.
 *          Apenas redesenha se o RadioGroup estiver na tela atual e precisar atualização:
 *          - Valida TFT, visibilidade, tela atual, carregamento e flag de redesenho
 *          - Para cada botão: desenha círculo de fundo, borda e círculo interno se selecionado
 *          - Usa cores adaptadas ao modo claro/escuro
 */
void RadioGroup::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  #if defined(DISP_DEFAULT)

  // uint16_t darkBg = WidgetBase::lightMode ? CFK_GREY3 : CFK_GREY11;
  uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  ESP_LOGD(TAG, "Redraw radiogroup");

  for (int16_t i = 0; i < m_config.amount; i++) {
    radio_t r = m_buttons[i];

    WidgetBase::objTFT->fillCircle(r.x, r.y, m_config.radius, lightBg);    // Botao
    WidgetBase::objTFT->drawCircle(r.x, r.y, m_config.radius, baseBorder); // Botao
    if (r.id == m_clickedId) {
      WidgetBase::objTFT->fillCircle(r.x, r.y, m_config.radius * 0.75,
                                     r.color); // fundo dentro
      WidgetBase::objTFT->drawCircle(r.x, r.y, m_config.radius * 0.75,
                                     baseBorder); // borda dentro
    }
  }

  #endif
}

/**
 * @brief Define o botão de rádio selecionado por seu ID.
 * @param clickedId ID do botão de rádio para selecionar.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar a seleção.
 * @details Atualiza o botão de rádio atualmente selecionado e dispara o callback se solicitado.
 *          Sem efeito se o ID especificado já estiver selecionado ou não existir no grupo:
 *          - Valida se widget está carregado
 *          - Verifica se ID já é o selecionado atual
 *          - Itera sobre botões para encontrar ID especificado
 *          - Atualiza ID selecionado e marca para redesenho
 *          - Dispara callback se configurado e triggerCallback for true
 */
void RadioGroup::setSelected(uint16_t clickedId, bool triggerCallback) {
  if (!m_loaded) {
    ESP_LOGE(TAG, "RadioGroup widget not loaded");
    return;
  }
  if (m_clickedId == clickedId) {
    ESP_LOGD(TAG, "RadioGroup widget already selected this id");
    return;
  }
  for (int16_t i = 0; i < m_config.amount; i++) {
    radio_t r = (m_buttons[i]);
    if (r.id == clickedId) {
      m_clickedId = r.id;
      m_shouldRedraw = true;
      if (m_callback != nullptr && triggerCallback) {
        WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
      }
    }
  }
}


/**
 * @brief Configura o RadioGroup com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref RadioGroupConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Valida parâmetros de configuração (amount não pode ser 0)
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia membros não-ponteiro da configuração
 *          - Faz cópia profunda do array de botões (m_buttons)
 *          - Define ID selecionado padrão e callback
 *          - Marca o widget como carregado
 *          O RadioGroup não será exibido corretamente até que este método seja chamado.
 */
void RadioGroup::setup(const RadioGroupConfig &config) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGW(TAG, "RadioGroup already initialized");
    return;
  }

  // Validate config parameters
  if (config.amount == 0) {
    ESP_LOGE(TAG, "Invalid config: amount cannot be 0");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();
  
  // Copy non-pointer members first
  m_config.group = config.group;
  m_config.radius = config.radius;
  m_config.amount = config.amount;
  m_config.defaultClickedId = config.defaultClickedId;
  m_config.callback = config.callback;
  
  // Deep copy buttons array
  if (config.buttons && config.amount > 0) {
    m_buttons = new radio_t[config.amount];
    if (m_buttons == nullptr) {
      ESP_LOGE(TAG, "Failed to allocate memory for buttons array");
      return;
    }
    
    // Copy each button
    for (int i = 0; i < config.amount; i++) {
      m_buttons[i] = config.buttons[i];
    }
    
    // Update config to point to our deep copy
    m_config.buttons = m_buttons;
  } else {
    m_buttons = nullptr;
    m_config.buttons = nullptr;
  }
  
  m_clickedId = config.defaultClickedId;
  m_callback = config.callback;
  m_loaded = true;
  
  ESP_LOGD(TAG, "RadioGroup configured: group=%d, radius=%d, amount=%d", 
           m_config.group, m_config.radius, m_config.amount);
}

/**
 * @brief Recupera o ID do botão de rádio atualmente selecionado.
 * @return ID do botão de rádio selecionado.
 * @details Retorna o ID armazenado em m_clickedId.
 */
uint16_t RadioGroup::getSelected() { return m_clickedId; }

/**
 * @brief Recupera o ID do grupo do RadioGroup.
 * @return Identificador do grupo para o RadioGroup.
 * @details Retorna o ID do grupo armazenado em m_config.group.
 */
uint16_t RadioGroup::getGroupId() { return m_config.group; }

/**
 * @brief Torna o RadioGroup visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void RadioGroup::show() {
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o RadioGroup da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void RadioGroup::hide() {
  m_visible = false;
  m_shouldRedraw = true;
}