#include "wcheckbox.h"
#include <Arduino.h>
#include <esp_log.h>

const char *CheckBox::TAG = "CheckBox";

/**
 * @brief Construtor da classe CheckBox.
 * @param _x Coordenada X da posição do checkbox na tela.
 * @param _y Coordenada Y da posição do checkbox na tela.
 * @param _screen Identificador da tela onde o checkbox será exibido.
 * @details Inicializa o checkbox com valores padrão: status desmarcado, borda de 2 pixels,
 *          cantos arredondados com raio de 5 pixels. O checkbox deve ser configurado com setup()
 *          antes de ser exibido.
 */
CheckBox::CheckBox(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_status(false), m_borderWidth(2), m_borderRadius(5){
  
  // Initialize with default config
  m_config = {.callback = nullptr, .weight = CheckBoxWeight::LIGHT, .size = 0, .checkedColor = 0, .uncheckedColor = 0};

  m_topRightPoint = {.x = 0, .y = 0};
  m_bottomCenterPoint = {.x = 0, .y = 0};
  m_middleLeftPoint = {.x = 0, .y = 0};

  ESP_LOGD(TAG, "CheckBox created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe CheckBox.
 * @details Limpa o ponteiro da função callback e registra o evento no log do ESP32.
 *          Todos os recursos são liberados automaticamente pela herança de @ref WidgetBase.
 */
CheckBox::~CheckBox() { 
  m_callback = nullptr; 
  ESP_LOGD(TAG, "CheckBox destroyed at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Força uma atualização imediata do CheckBox.
 * @details Define a flag m_shouldRedraw para true, forçando o redesenho do checkbox
 *          no próximo ciclo de atualização da tela. Útil para garantir que mudanças
 *          no estado ou configuração sejam visíveis imediatamente.
 */
void CheckBox::forceUpdate() { 
  m_shouldRedraw = true; 
  ESP_LOGD(TAG, "CheckBox force update requested");
}

/**
 * @brief Detecta se o CheckBox foi tocado pelo usuário.
 * @param _xTouch Ponteiro para a coordenada X do toque na tela.
 * @param _yTouch Ponteiro para a coordenada Y do toque na tela.
 * @return True se o toque está dentro da área do CheckBox, False caso contrário.
 * @details Este método realiza múltiplas validações antes de processar o toque:
 *          - Verifica se o widget está visível, inicializado, carregado e habilitado
 *          - Valida que o teclado virtual não está ativo
 *          - Verifica que o checkbox está na tela atual
 *          - Aplica debounce para evitar múltiplos cliques
 *          - Verifica se o widget não está bloqueado
 *          - Usa tolerância de 2 pixels para melhorar a experiência do usuário
 *          Se todas as validações passarem e o toque estiver dentro da área do checkbox,
 *          o estado é alternado e o widget é marcado para redesenho.
 */
bool CheckBox::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
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

  // Check if touch is within bounds (with some tolerance for better UX)
  const uint8_t touchTolerance = 2;
  int xDetect = m_xPos - touchTolerance;
  int yDetect = m_yPos - touchTolerance;
  int widthDetect = m_config.size + touchTolerance;
  int heightDetect = m_config.size + touchTolerance;

  bool inBouds = POINT_IN_RECT(*_xTouch, *_yTouch, xDetect, yDetect, widthDetect, heightDetect);

  if (inBouds) {
    m_myTime = millis();
    setPressed(true);  // Mark widget as pressed
    changeState();
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "CheckBox touched at (%d, %d), new status: %s", 
                  *_xTouch, *_yTouch, m_status ? "checked" : "unchecked");
    return true;
  }
  
  return false;
}

/**
 * @brief Recupera a função callback associada ao CheckBox.
 * @return Ponteiro para a função callback, ou nullptr se nenhuma foi definida.
 * @details A função callback é executada quando o estado do checkbox muda.
 *          Para mais informações sobre callbacks, consulte @ref WidgetBase.
 */
functionCB_t CheckBox::getCallbackFunc() { return m_callback; }

// getEnabled() and setEnabled() are inherited from WidgetBase

/**
 * @brief Alterna o estado atual do CheckBox entre marcado e desmarcado.
 * @details Inverte o valor do atributo m_status. Este método é chamado internamente
 *          por detectTouch() quando o checkbox é tocado pelo usuário.
 */
void CheckBox::changeState() { m_status = !m_status; }

/**
 * @brief Redesenha o CheckBox na tela, atualizando sua aparência baseada em seu estado.
 * @details Este método é responsável por renderizar o checkbox na tela:
 *          - Verifica todas as condições necessárias para o redesenho
 *          - Desenha um retângulo arredondado com a cor apropriada (marcado ou desmarcado)
 *          - Desenha a borda do checkbox se m_borderWidth > 0
 *          - Desenha a marca de verificação (checkmark) se o checkbox está marcado
 *          - Usa a biblioteca @ref Arduino_GFX_Library para o desenho
 *          Apenas redesenha se o checkbox está visível, inicializado, carregado, na tela atual
 *          e a flag m_shouldRedraw está configurada como true.
 */
void CheckBox::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_INITIALIZED_VOID
  CHECK_LOADED_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_DEBOUNCE_REDRAW_VOID
  CHECK_SHOULDREDRAW_VOID

#if defined(DISP_DEFAULT)
  m_shouldRedraw = false;

  ESP_LOGD(TAG, "Redrawing checkbox at (%d,%d) size %d, status: %s", 
               m_xPos, m_yPos, m_config.size, m_status ? "checked" : "unchecked");

  // Get appropriate colors
  uint16_t bgColor = m_status ? m_config.checkedColor : getUncheckedColor();
  
  // Draw main checkbox area
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.size, m_config.size, m_borderRadius, bgColor);
  
  // Draw border if configured
  if (m_borderWidth > 0) {
    drawBorder();
  }

  // Draw checkmark if checked
  if (m_status) {
    drawCheckmark();
  }
#endif
}


/**
 * @brief Configura o CheckBox com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref CheckBoxConfig contendo os parâmetros de configuração do checkbox.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Valida o tamanho (10-100 pixels) e ajusta valores fora da faixa recomendada
 *          - Copia todas as configurações para m_config
 *          - Calcula as coordenadas dos pontos para desenho da marca de verificação
 *          - Define a função callback e marca o widget como inicializado e carregado
 *          - O checkbox não será exibido corretamente até que este método seja chamado
 */
void CheckBox::setup(const CheckBoxConfig &config) {
  // Validate TFT object
  CHECK_TFT_VOID

  // Validate configuration parameters
  if (config.size < 10 || config.size > 100) {
    ESP_LOGW(TAG, "CheckBox size %d is outside recommended range (10-100), using default", config.size);
    m_config.size = (config.size < 10) ? 10 : 100;
  } else {
    m_config.size = config.size;
  }
  
  // Copy configuration (no deep copy needed as struct has no pointers)
  m_config.checkedColor = config.checkedColor;
  m_config.uncheckedColor = config.uncheckedColor;
  m_config.callback = config.callback;
  m_config.weight = config.weight;

  uint8_t offsetCheckMark = m_borderRadius;
  m_topRightPoint = {.x = (uint16_t)(m_xPos + m_config.size - offsetCheckMark), .y = (uint16_t)(m_yPos + offsetCheckMark)};
  m_bottomCenterPoint = {.x = (uint16_t)(m_xPos + m_config.size / 2), .y = (uint16_t)(m_yPos + (m_config.size * 0.666 ))};
  m_middleLeftPoint = {.x = (uint16_t)(m_xPos + offsetCheckMark), .y = (uint16_t)(m_yPos + m_config.size / 2)};


  
  // Set callback and mark as initialized
  m_callback = m_config.callback;
  m_initialized = true;
  m_loaded = true;
  
  ESP_LOGD(TAG, "CheckBox configured: size=%d, checkedColor=0x%04X, callback=%s", 
               m_config.size, m_config.checkedColor, (m_callback != nullptr) ? "set" : "null");
}

/**
 * @brief Recupera o status atual do CheckBox.
 * @return True se o checkbox está marcado, False se está desmarcado.
 */
bool CheckBox::getStatus() const { return m_status; }

/**
 * @brief Define o estado atual do checkbox.
 * @param status True para marcar o checkbox, False para desmarcar.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o estado.
 * @details Este método permite mudar programaticamente o estado do checkbox:
 *          - Atualiza o atributo m_status
 *          - Marca o widget para redesenho
 *          - Adiciona o callback na fila se uma função foi definida e triggerCallback for true
 *          - Registra o evento no log do ESP32
 *          Útil para sincronizar o estado visual com valores de variáveis ou configurações.
 */
void CheckBox::setStatus(bool status, bool triggerCallback) {
  CHECK_LOADED_VOID
  
  m_status = status;
  m_shouldRedraw = true;
  
  if (m_callback != nullptr && triggerCallback) {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
  
  ESP_LOGD(TAG, "CheckBox status set to: %s", status ? "checked" : "unchecked");
}

/**
 * @brief Torna o checkbox visível na tela.
 * @details Define m_visible como true e marca para redesenho. O checkbox será
 *          desenhado na próxima chamada de redraw() se estiver na tela atual.
 */
void CheckBox::show() {
  m_visible = true;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "CheckBox shown at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Oculta o checkbox da tela.
 * @details Define m_visible como false e marca para redesenho. O checkbox não será
 *          mais desenhado até que show() seja chamado novamente.
 */
void CheckBox::hide() {
  m_visible = false;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "CheckBox hidden at (%d, %d)", m_xPos, m_yPos);
}


/**
 * @brief Obtém a cor para o estado desmarcado do checkbox.
 * @return A cor em formato RGB565.
 * @details Se uncheckedColor foi configurado como 0 (automático), retorna uma cor baseada no modo:
 *          - Modo claro (lightMode): retorna CFK_GREY11 (cinza claro)
 *          - Modo escuro (!lightMode): retorna CFK_GREY3 (cinza escuro)
 *          Caso contrário, retorna o valor configurado em m_config.uncheckedColor.
 */
uint16_t CheckBox::getUncheckedColor() {
  if (m_config.uncheckedColor != 0) {
    return m_config.uncheckedColor;
  }
  // Auto-calculate based on light/dark mode
  return WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
}

/**
 * @brief Obtém a cor da borda do checkbox.
 * @return A cor da borda em formato RGB565.
 * @details Retorna a mesma cor do estado marcado (m_config.checkedColor).
 *          A borda usa a cor do checkbox marcado para manter a consistência visual.
 */
uint16_t CheckBox::getBorderColor() {
  return m_config.checkedColor;
}

/**
 * @brief Desenha o símbolo de marca de verificação (checkmark).
 * @details Desenha uma marca em formato de "V" usando linhas brancas. A espessura
 *          da marca varia conforme o peso configurado (LIGHT, MEDIUM, HEAVY):
 *          - LIGHT: uma linha simples
 *          - MEDIUM: três linhas (linha central + 1 pixel acima e abaixo)
 *          - HEAVY: cinco linhas (linha central + 2 pixels acima e abaixo)
 *          Usa os pontos calculados em setup() para criar o desenho.
 */
void CheckBox::drawCheckmark() {
#if defined(DISP_DEFAULT)
  uint8_t weight = static_cast<uint8_t>(m_config.weight);

  if(weight >= (int)CheckBoxWeight::HEAVY) {
    // Draw lines +2 and -2 y position from original lines
    WidgetBase::objTFT->drawLine(m_topRightPoint.x, m_topRightPoint.y + 2, m_bottomCenterPoint.x, m_bottomCenterPoint.y + 2, CFK_WHITE);
    WidgetBase::objTFT->drawLine(m_bottomCenterPoint.x, m_bottomCenterPoint.y + 2, m_middleLeftPoint.x, m_middleLeftPoint.y + 2, CFK_WHITE);
  }

  if(weight >= (int)CheckBoxWeight::MEDIUM) {
    // Draw lines +1 and -1 y position from original lines
    WidgetBase::objTFT->drawLine(m_topRightPoint.x, m_topRightPoint.y + 1, m_bottomCenterPoint.x, m_bottomCenterPoint.y + 1, CFK_WHITE);
    WidgetBase::objTFT->drawLine(m_bottomCenterPoint.x, m_bottomCenterPoint.y + 1, m_middleLeftPoint.x, m_middleLeftPoint.y + 1, CFK_WHITE);
  }
  
  WidgetBase::objTFT->drawLine(m_topRightPoint.x, m_topRightPoint.y, m_bottomCenterPoint.x, m_bottomCenterPoint.y, CFK_WHITE);
  WidgetBase::objTFT->drawLine(m_bottomCenterPoint.x, m_bottomCenterPoint.y, m_middleLeftPoint.x, m_middleLeftPoint.y, CFK_WHITE);  
#endif
}

/**
 * @brief Desenha a borda do checkbox.
 * @details Desenha múltiplas bordas concêntricas ao redor do checkbox para criar
 *          a aparência de uma borda mais larga. O número de bordas é controlado por
 *          m_borderWidth. Usa a cor retornada por getBorderColor().
 */
void CheckBox::drawBorder() {
  #if defined(DISP_DEFAULT)
  uint16_t borderColor = getBorderColor();
  
  // Draw border around the checkbox
  for (uint8_t i = 0; i < m_borderWidth; i++) {
    WidgetBase::objTFT->drawRoundRect(m_xPos + i, m_yPos + i, 
                                      m_config.size - 2*i, m_config.size - 2*i, 
                                      m_borderRadius, borderColor);
  }
  #endif
}

// isInitialized() is inherited from WidgetBase

/**
 * @brief Define um novo tamanho para o checkbox.
 * @param newSize O novo tamanho em pixels (largura e altura serão iguais).
 * @details Valida que o tamanho está na faixa recomendada (10-100 pixels).
 *          Se estiver fora da faixa, gera um aviso no log mas aceita o valor.
 *          Marca o widget para redesenho após a alteração.
 */
void CheckBox::setSize(uint16_t newSize) {
  CHECK_LOADED_VOID
  
  if (newSize < 10 || newSize > 100) {
    ESP_LOGW(TAG, "CheckBox size %d is outside recommended range (10-100)", newSize);
    return;
  }
  
  m_config.size = newSize;
  m_shouldRedraw = true;
  
  ESP_LOGD(TAG, "CheckBox size changed to: %d", newSize);
}

/**
 * @brief Obtém o tamanho atual do checkbox.
 * @return O tamanho atual em pixels.
 */
uint16_t CheckBox::getSize() const {
  return m_config.size;
}

/**
 * @brief Define as cores do checkbox.
 * @param checkedColor Cor RGB565 exibida quando o checkbox está marcado.
 * @param uncheckedColor Cor RGB565 exibida quando o checkbox está desmarcado. Use 0 para automático.
 * @details Atualiza as cores do checkbox e marca para redesenho. Se uncheckedColor for 0,
 *          a cor será calculada automaticamente baseada no modo claro/escuro em tempo de execução.
 */
void CheckBox::setColors(uint16_t checkedColor, uint16_t uncheckedColor) {
  CHECK_LOADED_VOID
  
  m_config.checkedColor = checkedColor;
  m_config.uncheckedColor = uncheckedColor;
  m_shouldRedraw = true;
  
  ESP_LOGD(TAG, "CheckBox colors updated: checked=0x%04X, unchecked=0x%04X", 
               checkedColor, uncheckedColor);
}