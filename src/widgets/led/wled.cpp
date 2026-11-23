#include "wled.h"

const char* Led::TAG = "Led";

/**
 * @brief Construtor do widget Led.
 * @param _x Coordenada X para a posição do LED.
 * @param _y Coordenada Y para a posição do LED.
 * @param _screen Identificador da tela onde o LED será exibido.
 * @details Inicializa o widget com valores padrão e configuração padrão.
 *          O LED não será funcional até que setup() seja chamado.
 */
Led::Led(uint16_t _x, uint16_t _y, uint8_t _screen) 
    : WidgetBase(_x, _y, _screen),
      m_lastStatus(false),
      m_status(false),
      m_shouldRedraw(true),
      m_initialized(false)
{
  // Initialize with default config
  m_config = {.radius = 10, .colorOn = CFK_RED, .colorOff = 0, .initialState = false};
  
  // Initialize gradient array
  for (uint8_t i = 0; i < m_colorLightGradientSize; i++) {
    m_colorLightGradient[i] = CFK_WHITE;
  }
  
  ESP_LOGD(TAG, "Led created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe Led.
 * @details Limpa recursos, callbacks e estado do widget.
 *          Chama cleanupMemory() para garantir limpeza adequada.
 */
Led::~Led() {
  ESP_LOGD(TAG, "Led destroyed at (%d, %d)", m_xPos, m_yPos);
  
  // Use centralized cleanup method
  cleanupMemory();
  
  // Clear callback
  if (m_callback != nullptr) {
    m_callback = nullptr;
  }
  
  // Reset state
  m_loaded = false;
  m_initialized = false;
  m_shouldRedraw = false;
}

/**
 * @brief Detecta se o LED foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque na tela.
 * @param _yTouch Ponteiro para a coordenada Y do toque na tela.
 * @return Sempre retorna False, pois LEDs não respondem a eventos de toque.
 * @details Este método realiza validações básicas mas não processa toques,
 *          já que LEDs são elementos apenas visuais.
 */
bool Led::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  // Early validation checks using macros
  CHECK_VISIBLE_BOOL
  CHECK_INITIALIZED_BOOL
  CHECK_LOADED_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL

  // Validate touch coordinates
  if (_xTouch == nullptr || _yTouch == nullptr) {
    ESP_LOGW(TAG, "Touch coordinates are null");
    return false;
  }

  // LEDs don't respond to touch events
  UNUSED(_xTouch);
  UNUSED(_yTouch);
  return false;
}

/**
 * @brief Recupera a função callback associada ao LED.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o LED for tocado.
 *          Note que LEDs não detectam toques por padrão.
 */
functionCB_t Led::getCallbackFunc() { return m_callback; }

/**
 * @brief Define o estado do LED (ligado ou desligado).
 * @param newValue Novo estado para o LED (true = ligado, false = desligado).
 * @details Atualiza o estado e marca para redesenho apenas se houver mudança.
 *          Se ligado, atualiza o gradiente de cor para efeito visual.
 */
void Led::setState(bool newValue) {
  CHECK_LOADED_VOID
  CHECK_INITIALIZED_VOID
  
  if (m_status != newValue) {
    m_status = newValue;
    m_shouldRedraw = true;
    
    // Update gradient if needed
    if (m_status) {
      updateGradient();
    }
    
    ESP_LOGD(TAG, "Led state set to %s", m_status ? "ON" : "OFF");
  }
}

/**
 * @brief Obtém o estado atual do LED.
 * @return Estado atual do LED (true = ligado, false = desligado).
 */
bool Led::getState() const {
  return m_status;
}

/**
 * @brief Valida a estrutura de configuração do LED.
 * @param config Configuração a ser validada.
 * @return True se válida, False caso contrário.
 * @details Verifica se todos os parâmetros essenciais estão dentro dos limites aceitáveis:
 *          - Raio deve ser maior que zero
 *          - Adverte se raio é muito grande (pode impactar performance)
 */
bool Led::validateConfig(const LedConfig& config) {
  // Validate basic parameters
  if (config.radius <= 0) {
    ESP_LOGE(TAG, "Invalid radius: %d", config.radius);
    return false;
  }
  
  if (config.radius > 50) {
    ESP_LOGW(TAG, "Radius %d is quite large, may impact performance", config.radius);
  }
  
  return true;
}

/**
 * @brief Atualiza o gradiente de cor para o efeito de luz.
 * @details Cria gradiente de brilhante para escuro para simular efeito de luz.
 *          Calcula intensidade decrescente para cada anel usando mistura de cores.
 */
void Led::updateGradient() {
  // Create gradient from bright to dim
  uint16_t baseColor = m_config.colorOn;
  //uint16_t whiteColor = CFK_WHITE;

  //m_colorLightGradient = blendColors(baseColor, whiteColor, 0.5);

  uint16_t cores[m_colorLightGradientSize];
  blendColorsRGB(baseColor, 0xFFFF, m_colorLightGradientSize, cores, m_colorLightGradientSize);

  for (uint8_t i = 0; i < m_colorLightGradientSize; i++) {
    m_colorLightGradient[i] = cores[i];
  }
}

/**
 * @brief Obtém a cor para o LED desligado.
 * @return Cor a ser usada quando o LED está desligado.
 * @details Se colorOff foi especificado, retorna essa cor.
 *          Caso contrário, calcula automaticamente baseado no modo claro/escuro.
 */
uint16_t Led::getOffColor() {
  if (m_config.colorOff != 0) {
    return m_config.colorOff;
  }
  // Auto-calculate based on light/dark mode
  return WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
}

/**
 * @brief Obtém a cor de fundo para o LED.
 * @return Cor de fundo a ser usada.
 * @details Retorna a cor configurada em colorOff.
 */
uint16_t Led::getBackgroundColor() {
  return m_config.colorOff;
}

/**
 * @brief Método centralizado de limpeza de memória.
 * @details LED não usa memória dinâmica, mas o método está aqui para consistência.
 *          Reseta array de gradiente para valores padrão.
 */
void Led::cleanupMemory() {
  // Led doesn't use dynamic memory, but method is here for consistency
  // Reset gradient array to default values
  for (uint8_t i = 0; i < m_colorLightGradientSize; i++) {
    m_colorLightGradient[i] = CFK_WHITE;
  }
}

/**
 * @brief Desenha o fundo do LED na tela.
 * @details Desenha um círculo com borda na posição do LED.
 *          Usa cor de borda baseada no modo claro/escuro.
 */
void Led::drawBackground() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID

#if defined(USING_GRAPHIC_LIB)
  uint16_t borderColor = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;
  WidgetBase::objTFT->drawCircle(m_xPos, m_yPos, m_config.radius, borderColor);
#endif
  
  ESP_LOGD(TAG, "Led background drawn at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Redesenha o LED na tela, atualizando sua aparência.
 * @details Exibe o LED com representação visual adequada baseada no estado atual:
 *          - Estado ligado: desenha múltiplos círculos concêntricos com gradiente de cor
 *            para criar efeito de brilho
 *          - Estado desligado: desenha círculo sólido com cor de fundo
 *          Apenas redesenha se o LED está visível, inicializado, carregado, na tela atual
 *          e a flag m_shouldRedraw está configurada como true.
 */
void Led::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_DEBOUNCE_REDRAW_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  #if defined(USING_GRAPHIC_LIB)
  
  ESP_LOGD(TAG, "Redrawing Led at (%d, %d), status: %s", m_xPos, m_yPos, m_status ? "ON" : "OFF");

  if (m_status) {
    for(uint8_t i = 0; i < m_colorLightGradientSize; i++)
    {
      uint8_t radius = m_config.radius - (i * 4);
      if(radius > m_config.radius){
        continue;//Aborta o loop se o raio for maior que o raio do LED (overflow do tipo uint8_t)
      }
      WidgetBase::objTFT->fillCircle(m_xPos - (2 * i), m_yPos - (2 * i), radius, m_colorLightGradient[i]);
    }
  } else {
    // Draw off state
    WidgetBase::objTFT->fillCircle(m_xPos, m_yPos, m_config.radius - 1, getOffColor());
  }

  #endif
}

/**
 * @brief Força o LED a atualizar, refrescando seu estado visual no próximo redesenho.
 * @details Define a flag m_shouldRedraw como true para garantir que o widget
 *          seja redesenhado na próxima oportunidade.
 */
void Led::forceUpdate()
{
  m_shouldRedraw = true;
}

/**
 * @brief Configura o widget LED com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref LedConfig contendo os parâmetros de configuração do LED (raio e cores).
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Valida a configuração usando validateConfig()
 *          - Limpa memória existente usando cleanupMemory()
 *          - Atribui a estrutura de configuração
 *          - Atualiza gradiente de cor para efeito visual
 *          - Marca o widget como inicializado e carregado
 *          O LED não será exibido corretamente até que este método seja chamado.
 */
void Led::setup(const LedConfig& config) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGD(TAG, "Led widget already configured");
    return;
  }

  // Validate configuration first
  if (!validateConfig(config)) {
    ESP_LOGE(TAG, "Invalid configuration provided");
    return;
  }

  // Clean up any existing memory
  cleanupMemory();

  // Assign the configuration structure
  m_config = config;

  m_status = m_config.initialState;
  
  // Update gradient if needed
  updateGradient();
  
  // Initialize LED
  m_shouldRedraw = true;
  m_loaded = true;
  m_initialized = true;
  
  ESP_LOGD(TAG, "Led setup completed at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Torna o LED visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void Led::show() {
  m_visible = true;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "Led shown at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Oculta o LED da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void Led::hide() {
  m_visible = false;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "Led hidden at (%d, %d)", m_xPos, m_yPos);
}

void Led::setColor(uint16_t color) {
  m_config.colorOn = color;
  updateGradient();
  m_shouldRedraw = true;
}
