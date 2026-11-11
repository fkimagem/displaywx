#include "whslider.h"
#include <Arduino.h>
#include <esp_log.h>

const char* HSlider::TAG = "HSlider";

// if defined detect touch only when on handler circle, else, detect on rect total area and calculate handler position
// #define DETECT_ON_HANDLER

/**
 * @brief Construtor da classe HSlider.
 * @param _x Coordenada X para a posição do HSlider.
 * @param _y Coordenada Y para a posição do HSlider.
 * @param _screen Identificador da tela onde o HSlider será exibido.
 * @details Inicializa o widget com valores padrão e configuração vazia.
 *          O slider não será funcional até que setup() seja chamado.
 */
HSlider::HSlider(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_currentPos(0), m_lastPos(0),
      m_height(0), m_contentRadius(0), m_centerV(0), m_minX(0), m_maxX(0),
      m_value(0)
{
  // Initialize with default config
  m_config = {.callback = nullptr, .subtitle = nullptr, .minValue = 0, .maxValue = 0, .radius = 0, .width = 0, .pressedColor = 0, .backgroundColor = 0};
  
  ESP_LOGD(TAG, "HSlider created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe HSlider.
 * @details Limpa recursos e callbacks associados ao widget.
 */
HSlider::~HSlider() {
  ESP_LOGD(TAG, "HSlider destroyed at (%d, %d)", m_xPos, m_yPos);
  
  // Clear callback
  if (m_callback != nullptr) {
    m_callback = nullptr;
  }
}

/**
 * @brief Detecta se o HSlider foi tocado pelo usuário e processa a entrada de toque.
 * @param _xTouch Ponteiro para a coordenada X do toque na tela.
 * @param _yTouch Ponteiro para a coordenada Y do toque na tela.
 * @return True se o toque está dentro da área do slider, False caso contrário.
 * @details Este método realiza múltiplas validações antes de processar o toque:
 *          - Verifica se o widget está visível, inicializado, carregado e habilitado
 *          - Valida que o teclado virtual não está ativo
 *          - Verifica que o slider está na tela atual
 *          - Verifica se o widget não está bloqueado
 *          - Detecta toque na área da trilha e move o controle proporcionalmente
 *          - Atualiza o valor baseado na nova posição e marca para redesenho
 *          Suporta dois modos: detecção apenas no controle (DETECT_ON_HANDLER) ou na área total.
 */
bool HSlider::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  // Early validation checks using macros
  CHECK_VISIBLE_BOOL
  CHECK_INITIALIZED_BOOL
  CHECK_LOADED_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL
  CHECK_POINTER_TOUCH_NULL_BOOL
  CHECK_DEBOUNCE_FAST_REDRAW_BOOL

  bool detected = false;

  #if defined(DETECT_ON_HANDLER)
  int32_t offsetRadius = 10; // Offset to account for the radius of the slider handle
  int32_t radiusToDetect = m_config.radius + offsetRadius; // Add a small margin for touch detection
  int32_t deltaX = (*_xTouch - m_currentPos) * (*_xTouch - m_currentPos);
  int32_t deltaY = (*_yTouch - (m_yPos + radiusToDetect)) *
                   (*_yTouch - (m_yPos + radiusToDetect));
  int32_t radiusQ = radiusToDetect * radiusToDetect;

  if ((deltaX < radiusQ) && (deltaY < radiusQ)) {
    detected = true;
    m_currentPos = (*_xTouch);
    m_currentPos = constrain(m_currentPos, m_minX, m_maxX);
    updateValue();
    m_shouldRedraw = true;
  }
  #else
  if(POINT_IN_RECT(*_xTouch, *_yTouch, m_minX, m_yPos, (m_config.width), m_height)) {
    m_myTime = millis();
    detected = true;
    m_currentPos = (*_xTouch);
    m_currentPos = constrain(m_currentPos, m_minX, m_maxX);
    updateValue();
    m_shouldRedraw = true;
  }
  #endif

  if (detected) {
    setPressed(true);  // Mark widget as pressed
    ESP_LOGD(TAG, "HSlider touched at (%d, %d), value: %d", *_xTouch, *_yTouch, m_value);
  }

  return detected;
}

/**
 * @brief Recupera a função callback associada ao slider.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o valor do slider mudar.
 */
functionCB_t HSlider::getCallbackFunc() { return m_callback; }


/**
 * @brief Valida a estrutura de configuração do slider.
 * @param config Configuração a ser validada.
 * @return True se válida, False caso contrário.
 * @details Verifica se todos os parâmetros essenciais estão dentro dos limites aceitáveis:
 *          - Largura deve ser maior que zero
 *          - Valor mínimo deve ser menor que o máximo
 *          - Raio deve ser maior que zero
 */
bool HSlider::validateConfig(const HSliderConfig& config) {
  // Validate basic parameters
  if (config.width <= 0) {
    ESP_LOGE(TAG, "Invalid width: %d", config.width);
    return false;
  }
  
  if (config.minValue >= config.maxValue) {
    ESP_LOGE(TAG, "Invalid range: min=%d, max=%d", config.minValue, config.maxValue);
    return false;
  }
  
  if (config.radius <= 0) {
    ESP_LOGE(TAG, "Invalid radius: %d", config.radius);
    return false;
  }
  
  return true;
}

/**
 * @brief Atualiza o valor do slider baseado na posição atual.
 * @details Mapeia a posição atual do controle para o valor correspondente
 *          dentro da faixa configurada usando a função map().
 */
void HSlider::updateValue() {
  m_value = map(m_currentPos, m_minX, m_maxX, m_config.minValue, m_config.maxValue);
}

/**
 * @brief Atualiza as dimensões e restrições do slider.
 * @details Calcula e define todas as dimensões necessárias para o funcionamento do slider:
 *          - Altura da trilha baseada no raio
 *          - Posição vertical central
 *          - Limites mínimo e máximo para movimento do controle
 *          - Raio interno para área de conteúdo
 */
void HSlider::updateDimensions() {
  m_height = m_config.radius * 2;
  m_centerV = m_yPos + m_config.radius;
  m_minX = m_xPos + m_config.radius;
  m_maxX = m_xPos + m_config.width - m_config.radius;
  m_contentRadius = m_config.radius - 4; // Inner radius for content
}

/**
 * @brief Inicializa o widget HSlider com cálculos padrão.
 * @details Chama updateDimensions() para calcular dimensões e marca o widget
 *          para redesenho na próxima atualização.
 */
void HSlider::start() {
  updateDimensions();
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "HSlider initialized at (%d, %d)", m_xPos, m_yPos);
}

// setEnabled method removed - use inherited methods from WidgetBase

/**
 * @brief Redesenha o controle deslizante e a porção preenchida na tela.
 * @details Este método é responsável por renderizar o slider na tela:
 *          - Verifica todas as condições necessárias para o redesenho
 *          - Limpa a posição anterior do controle
 *          - Desenha a barra de progresso baseada na direção do movimento
 *          - Renderiza o controle deslizante com cores apropriadas
 *          - Atualiza a posição anterior para próxima renderização
 *          Apenas redesenha se o slider está visível, inicializado, carregado, na tela atual
 *          e a flag m_shouldRedraw está configurada como true.
 */
void HSlider::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID
  //CHECK_DEBOUNCE_FAST_REDRAW_VOID
  CHECK_SHOULDREDRAW_VOID


  m_shouldRedraw = false;

  #if defined(USING_GRAPHIC_LIB)


  Serial.println("HSlider redraw");

  uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;

  // Clear previous position
  WidgetBase::objTFT->fillCircle(m_lastPos, m_centerV, m_contentRadius,
                                 m_config.backgroundColor); // slider background

  ESP_LOGD(TAG, "Redraw HSlider with value %i at %i", m_value, m_currentPos);

  uint8_t fillRadius = 2;

  // Draw progress bar
  if(m_lastPos > m_currentPos){

    WidgetBase::objTFT->fillRoundRect(m_currentPos, m_centerV - (fillRadius),
                                  m_lastPos - (m_currentPos), fillRadius * 2,
                                  fillRadius, m_config.backgroundColor);
  }else{
    WidgetBase::objTFT->fillRoundRect(m_minX, m_centerV - (fillRadius),
                                  m_currentPos - (m_minX), fillRadius * 2,
                                  fillRadius, m_config.pressedColor);
  }
  
  // Draw slider handle
  WidgetBase::objTFT->fillCircle(m_currentPos, m_centerV, m_contentRadius,
                                 lightBg); // slider handle
  WidgetBase::objTFT->fillCircle(m_currentPos, m_centerV,
                                 m_contentRadius * 0.75,
                                 m_config.pressedColor); // slider center

  m_lastPos = m_currentPos;

  if (m_config.subtitle)
  {
    m_config.subtitle->setTextInt(m_value);
  }

  #endif
}

/**
 * @brief Desenha o fundo do slider e elementos estáticos.
 * @details Cria a trilha e borda do slider:
 *          - Desenha o fundo da trilha com cantos arredondados
 *          - Adiciona borda com cor baseada no modo claro/escuro
 *          - Chama redraw() para renderizar o controle deslizante
 *          Chamado durante a configuração inicial e quando o slider precisa de redesenho completo.
 */
void HSlider::drawBackground() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_LOADED_VOID

  #if defined(USING_GRAPHIC_LIB)

  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_height, m_config.radius,
                                    m_config.backgroundColor);
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_height, m_config.radius,
                                    baseBorder);

  redraw();

  #endif
}

/**
 * @brief Configura o HSlider com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref HSliderConfig contendo os parâmetros de configuração do slider.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Valida a configuração usando validateConfig()
 *          - Atribui a estrutura de configuração
 *          - Chama start() para cálculos e inicialização
 *          - Define posição inicial do controle
 *          - Atualiza o valor inicial
 *          - Marca o widget como inicializado e carregado
 *          O slider não será exibido corretamente até que este método seja chamado.
 */
void HSlider::setup(const HSliderConfig &config) {
  // Validate TFT object
  CHECK_TFT_VOID

  // Validate configuration first
  if (!validateConfig(config)) {
    ESP_LOGE(TAG, "Invalid configuration provided");
    return;
  }

  // Assign the configuration structure
  m_config = config;
  m_callback = config.callback;

  start();

  m_currentPos = m_minX;
  m_lastPos = m_currentPos;

  updateValue();

  m_loaded = true;
  m_initialized = true;

  ESP_LOGD(TAG, "HSlider setup completed at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Recupera o valor atual do slider.
 * @return Valor atual do slider dentro da faixa configurada.
 * @details Retorna o valor mapeado baseado na posição atual do controle deslizante.
 */
int HSlider::getValue() const { return m_value; }

/**
 * @brief Define o valor do slider.
 * @param newValue Novo valor para o slider.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o valor.
 * @details Atualiza a posição do controle baseada no valor fornecido:
 *          - Restringe o valor dentro da faixa configurada
 *          - Mapeia o valor para a posição correspondente na trilha
 *          - Marca para redesenho
 *          - Executa callback se configurado e triggerCallback for true
 */
void HSlider::setValue(int newValue, bool triggerCallback) {
  CHECK_LOADED_VOID
  CHECK_INITIALIZED_VOID
  
  m_value = constrain(newValue, m_config.minValue, m_config.maxValue);
  
  // Update position based on value
  m_currentPos = map(m_value, m_config.minValue, m_config.maxValue, m_minX, m_maxX);
  
  m_shouldRedraw = true;
  
  if (m_callback != nullptr && triggerCallback) {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
  
  ESP_LOGD(TAG, "HSlider value set to %d", m_value);
}

/**
 * @brief Força o slider a atualizar, refrescando seu estado visual no próximo redesenho.
 * @details Define a flag m_shouldRedraw como true para garantir que o widget
 *          seja redesenhado na próxima oportunidade.
 */
void HSlider::forceUpdate() { 
  m_shouldRedraw = true; 
  ESP_LOGD(TAG, "HSlider force update requested");
}

/**
 * @brief Torna o slider visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void HSlider::show() {
  m_visible = true;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "HSlider shown at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Oculta o slider da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void HSlider::hide() {
  m_visible = false;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "HSlider hidden at (%d, %d)", m_xPos, m_yPos);
}
