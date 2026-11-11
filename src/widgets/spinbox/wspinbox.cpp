#include "wspinbox.h"

const char* SpinBox::TAG = "SpinBox";



/**
 * @brief Diminui o valor atual pelo valor do passo.
 * @details Decrementa o valor atual garantindo que permaneça dentro da faixa definida:
 *          - Calcula novo valor (atual - passo)
 *          - Restringe valor usando constrain() entre minValue e maxValue
 */
void SpinBox::decreaseValue() {
  int temp = m_currentValue - m_config.step;
  m_currentValue = constrain(temp, m_config.minValue, m_config.maxValue);
}

/**
 * @brief Aumenta o valor atual pelo valor do passo.
 * @details Incrementa o valor atual garantindo que permaneça dentro da faixa definida:
 *          - Calcula novo valor (atual + passo)
 *          - Restringe valor usando constrain() entre minValue e maxValue
 */
void SpinBox::increaseValue() {
  int temp = m_currentValue + m_config.step;
  m_currentValue = constrain(temp, m_config.minValue, m_config.maxValue);
}

/**
 * @brief Construtor para a classe SpinBox.
 * @param _x Coordenada X para a posição do SpinBox.
 * @param _y Coordenada Y para a posição do SpinBox.
 * @param _screen Identificador da tela onde o SpinBox será exibido.
 * @details Inicializa o widget com valores padrão e configuração vazia.
 *          A caixa de spin não será funcional até que setup() seja chamado.
 */
SpinBox::SpinBox(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen){

      m_config = {.callback = nullptr, .minValue = 0, .maxValue = 0, .startValue = 0, .width = 0, .height = 0, .step = 0, .color = 0, .textColor = 0};
      m_offset = 10;
      ESP_LOGD(TAG, "SpinBox created at (%d, %d) on screen %d", _x, _y, _screen);
    }

/**
 * @brief Destrutor da classe SpinBox.
 * @details Limpa quaisquer recursos usados pelo SpinBox.
 */
SpinBox::~SpinBox() {
    cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo SpinBox.
 * @details SpinBox não usa alocação dinâmica de memória.
 */
void SpinBox::cleanupMemory() {
    ESP_LOGD(TAG, "SpinBox memory cleanup completed");
}

/**
 * @brief Detecta se o SpinBox foi tocado e manipula ações de incremento/decremento.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do SpinBox, False caso contrário.
 * @details Detecta toques nas áreas dos botões de decremento (esquerda) ou incremento (direita),
 *          e processa a mudança de valor conforme apropriado:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento e debounce
 *          - Define áreas de detecção para botão esquerdo (decremento) e direito (incremento)
 *          - Detecta toque na área de decremento: chama decreaseValue() e marca para redesenho
 *          - Detecta toque na área de incremento: chama increaseValue() e marca para redesenho
 */
bool SpinBox::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL

  uint16_t topY = m_yPos;
  // uint16_t bottomY = m_yPos + m_height;

  Rect_t boundsAreaDecrement = {.x = static_cast<uint16_t>(m_xPos), .y = topY, .width = (uint16_t)(m_config.width / 2), .height = m_config.height}; // {.x = m_xPos, .y = topY, .width = (uint16_t)(m_config.width / 2), .height = m_config.height};
  Rect_t boundsAreaIncrement = {.x = (uint16_t)(m_xPos + (m_config.width / 2)), .y = topY, .width = (uint16_t)(m_config.width / 2),
                                .height = m_config.height};

  bool detectLeft = POINT_IN_RECT(*_xTouch, *_yTouch, boundsAreaDecrement.x, boundsAreaDecrement.y, boundsAreaDecrement.width, boundsAreaDecrement.height);
  bool detectRight = POINT_IN_RECT(*_xTouch, *_yTouch, boundsAreaIncrement.x, boundsAreaIncrement.y, boundsAreaIncrement.width, boundsAreaIncrement.height);

  if(!detectLeft && !detectRight) {
    return false;
  }


  
  if(detectLeft) {
    decreaseValue();
  }else if(detectRight) {
    increaseValue();
  }

  setPressed(true);  // Mark widget as pressed
  m_myTime = millis();
  m_shouldRedraw = true;

  return true;
}

/**
 * @brief Recupera a função callback associada ao SpinBox.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o SpinBox for utilizado.
 */
functionCB_t SpinBox::getCallbackFunc() { return m_callback; }

/**
 * @brief Redesenha o SpinBox na tela, atualizando sua aparência.
 * @details Atualiza a exibição do valor atual na caixa de spin,
 *          apenas redesenha se o SpinBox estiver na tela atual e precisar atualização:
 *          - Valida TFT, visibilidade, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Calcula dimensões dos botões e área disponível para texto
 *          - Define cor do texto e desenha retângulo de fundo para valor
 *          - Seleciona melhor fonte RobotoBold para o texto
 *          - Exibe valor atual centralizado na área de texto
 */
void SpinBox::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  #if defined(USING_GRAPHIC_LIB)

  //uint16_t btnW = m_config.height - (2 * m_offset);
  //uint16_t btnH = m_config.height - (2 * m_offset);

  //uint16_t availableW = m_config.width - (2 * m_offset);
  //uint16_t availableH = m_config.height - (2 * m_offset);

  WidgetBase::objTFT->setTextColor(m_config.textColor);

  //WidgetBase::objTFT->fillRoundRect(m_xPos + (2 * m_offset) + btnW, m_yPos + m_offset,m_config.width - (4 * m_offset + 2 * btnW), btnH, m_radius, m_config.color);
  WidgetBase::objTFT->fillRect(m_textAreaSize.x, m_textAreaSize.y, m_textAreaSize.width, m_textAreaSize.height, m_config.color);


  //uint16_t offsetFont = 10;
  //WidgetBase::objTFT->setFont(getBestRobotoBold( availableW - offsetFont, availableH - offsetFont, String(m_currentValue).c_str()));
  WidgetBase::objTFT->setFont(m_font);
  printText(String(m_currentValue).c_str(), m_xPos + m_config.width / 2,
            m_yPos + (m_config.height / 2) - 3, MC_DATUM, m_lastArea, m_config.color);
  updateFont(FontType::UNLOAD);

  #endif

}

/**
 * @brief Desenha o fundo do SpinBox, incluindo elementos estáticos.
 * @details Cria o container visual, botões de mais e menos para o SpinBox.
 *          Este método é tipicamente chamado uma vez durante setup ao invés de em cada redesenho:
 *          - Desenha retângulo principal com bordas arredondadas
 *          - Desenha borda do container
 *          - Desenha dois botões com cores clarificadas (lados esquerdo e direito)
 *          - Desenha bordas dos botões
 *          - Desenha sinais de menos (-) no botão esquerdo e mais (+) no botão direito
 */
void SpinBox::drawBackground() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID

  #if defined(USING_GRAPHIC_LIB)

  // Fundo
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_radius,m_config.color);
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_radius,CFK_BLACK);

  uint16_t corBotao = lighten565(m_config.color, 0.5);//WidgetBase::lightenColor565(m_config.color, 4);

  // botao left
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius, corBotao);
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius,CFK_BLACK);

  // botao right
  WidgetBase::objTFT->fillRoundRect(m_xPos + m_config.width - m_buttonSize.width, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius, corBotao);
  WidgetBase::objTFT->drawRoundRect(m_xPos + m_config.width - m_buttonSize.width, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius,CFK_BLACK);

  int centerX_left = m_xPos + (m_buttonSize.width / 2);
  int centerY_left = m_yPos + (m_buttonSize.height / 2);

  int centerX_right = m_xPos + m_config.width - (m_buttonSize.width / 2);
  int centerY_right = m_yPos + (m_buttonSize.height / 2);

  int signalSize = m_buttonSize.width / 2;
  int signalHalfWeight = 2;

  // Sinal menos
  WidgetBase::objTFT->fillRect(centerX_left - (signalSize / 2),
                               centerY_left - signalHalfWeight, signalSize, signalHalfWeight * 2,
                               m_config.textColor);

  // Sinal mais
  WidgetBase::objTFT->fillRect(centerX_right - (signalSize / 2),
                               centerY_right - signalHalfWeight,
                               signalSize,
                               signalHalfWeight * 2,
                               m_config.textColor);
  WidgetBase::objTFT->fillRect(centerX_right - signalHalfWeight,
                               centerY_right - (signalSize / 2),
                               signalHalfWeight * 2,
                               signalSize,
                               m_config.textColor);

  m_shouldRedraw = true;

  #endif


  // WidgetBase::objTFT->fillCircle(btnRight.x, btnRight.y, 4, CFK_YELLOW);
}


/**
 * @brief Configura o SpinBox com периметров definidos em uma estrutura de configuração.
 * @param config Estrutura @ref SpinBoxConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia profundamente a configuração
 *          - Valida e corrige se minValue > maxValue (troca valores)
 *          - Restringe startValue dentro da faixa válida
 *          - Define callback
 *          - Marca o widget como carregado
 *          A caixa de spin não será exibida corretamente até que este método seja chamado.
 */
void SpinBox::setup(const SpinBoxConfig &config) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGW(TAG, "SpinBox already initialized");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();
  
  // Deep copy configuration
  m_config = config;
  
  // Set member variables from config
  if (m_config.minValue > m_config.maxValue) {
    int temp = m_config.minValue;
    m_config.minValue = m_config.maxValue;
    m_config.maxValue = temp;
  }


  m_buttonSize.x = 0;
  m_buttonSize.y = 0;
  m_buttonSize.height = m_config.height;
  m_buttonSize.width = m_config.height;

  m_textAreaSize.x = m_xPos + m_buttonSize.width + (m_offset / 2);
  m_textAreaSize.y = m_yPos + (m_offset / 2);
  m_textAreaSize.height = m_config.height - m_offset;
  m_textAreaSize.width = m_config.width - (2 * m_buttonSize.width) - (m_offset);


  int charCountMinValue = String(m_config.minValue).length();
  int charCountMaxValue = String(m_config.maxValue).length();
  
  #if defined(USING_GRAPHIC_LIB)
  if (charCountMinValue > charCountMaxValue) {
    m_font = getBestRobotoBold(m_textAreaSize.width, m_textAreaSize.height, String(m_config.minValue).c_str());
  } else {
    m_font = getBestRobotoBold(m_textAreaSize.width, m_textAreaSize.height, String(m_config.maxValue).c_str());
  }
  #endif


  m_currentValue = constrain(m_config.startValue, m_config.minValue, m_config.maxValue);
  m_callback = config.callback;
  m_loaded = true;
}

/**
 * @brief Recupera o valor atual do SpinBox.
 * @return Valor numérico atual.
 * @details Retorna o valor armazenado em m_currentValue.
 */
int SpinBox::getValue() { return m_currentValue; }

/**
 * @brief Define o valor atual do SpinBox.
 * @param _value Novo valor numérico para definir.
 * @param triggerCallback True para disparar a callback (padrão), False para apenas atualizar o valor.
 * @details Atualiza o valor do SpinBox, garante que está dentro da faixa definida,
 *          e marca o widget para redesenho. Dispara o callback se solicitado:
 *          - Valida se widget está carregado
 *          - Restringe valor usando constrain() dentro da faixa min/max
 *          - Marca para redesenho
 *          - Dispara callback se configurado e triggerCallback for true
 */
void SpinBox::setValue(int _value, bool triggerCallback) {
  if (!m_loaded) {
    ESP_LOGE(TAG, "Spinbox widget not loaded");
    return;
  }

  m_currentValue = constrain(_value, m_config.minValue, m_config.maxValue);
  m_shouldRedraw = true;

  if (m_callback != nullptr && triggerCallback) {
    WidgetBase::addCallback(m_callback, WidgetBase::CallbackOrigin::SELF);
  }
}

/**
 * @brief Torna o SpinBox visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void SpinBox::show() {
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o SpinBox da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void SpinBox::hide() {
  m_visible = false;
  m_shouldRedraw = true;
}

/**
 * @brief Força o SpinBox a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void SpinBox::forceUpdate() { m_shouldRedraw = true; }