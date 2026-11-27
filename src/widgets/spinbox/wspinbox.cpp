#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_SPINBOX)
#include "wspinbox.h"

const char* SpinBox::TAG = "SpinBox";



/**
 * @brief Diminui o valor atual pelo valor do passo.
 * @details Decrementa o valor atual garantindo que permaneÃ§a dentro da faixa definida:
 *          - Calcula novo valor (atual - passo)
 *          - Restringe valor usando constrain() entre minValue e maxValue
 */
void SpinBox::decreaseValue() {
  WidgetStateGuard widgetStateGuard;
  int temp = m_currentValue - m_config.step;
  m_currentValue = constrain(temp, m_config.minValue, m_config.maxValue);
}

/**
 * @brief Aumenta o valor atual pelo valor do passo.
 * @details Incrementa o valor atual garantindo que permaneÃ§a dentro da faixa definida:
 *          - Calcula novo valor (atual + passo)
 *          - Restringe valor usando constrain() entre minValue e maxValue
 */
void SpinBox::increaseValue() {
  WidgetStateGuard widgetStateGuard;
  int temp = m_currentValue + m_config.step;
  m_currentValue = constrain(temp, m_config.minValue, m_config.maxValue);
}

/**
 * @brief Construtor para a classe SpinBox.
 * @param _x Coordenada X para a posiÃ§Ã£o do SpinBox.
 * @param _y Coordenada Y para a posiÃ§Ã£o do SpinBox.
 * @param _screen Identificador da tela onde o SpinBox serÃ¡ exibido.
 * @details Inicializa o widget com valores padrÃ£o e configuraÃ§Ã£o vazia.
 *          A caixa de spin nÃ£o serÃ¡ funcional atÃ© que setup() seja chamado.
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
 * @brief Limpa memÃ³ria usada pelo SpinBox.
 * @details SpinBox nÃ£o usa alocaÃ§Ã£o dinÃ¢mica de memÃ³ria.
 */
void SpinBox::cleanupMemory() {
  WidgetStateGuard widgetStateGuard;
    ESP_LOGD(TAG, "SpinBox memory cleanup completed");
}

/**
 * @brief Detecta se o SpinBox foi tocado e manipula aÃ§Ãµes de incremento/decremento.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque estÃ¡ dentro da Ã¡rea do SpinBox, False caso contrÃ¡rio.
 * @details Detecta toques nas Ã¡reas dos botÃµes de decremento (esquerda) ou incremento (direita),
 *          e processa a mudanÃ§a de valor conforme apropriado:
 *          - Valida visibilidade, uso de teclado, tela atual, carregamento e debounce
 *          - Define Ã¡reas de detecÃ§Ã£o para botÃ£o esquerdo (decremento) e direito (incremento)
 *          - Detecta toque na Ã¡rea de decremento: chama decreaseValue() e marca para redesenho
 *          - Detecta toque na Ã¡rea de incremento: chama increaseValue() e marca para redesenho
 */
bool SpinBox::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  WidgetStateGuard widgetStateGuard;
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
 * @brief Recupera a funÃ§Ã£o callback associada ao SpinBox.
 * @return Ponteiro para a funÃ§Ã£o callback.
 * @details Retorna o ponteiro para a funÃ§Ã£o que serÃ¡ executada quando o SpinBox for utilizado.
 */
functionCB_t SpinBox::getCallbackFunc() {
  WidgetStateGuard widgetStateGuard; return m_callback; }

/**
 * @brief Redesenha o SpinBox na tela, atualizando sua aparÃªncia.
 * @details Atualiza a exibiÃ§Ã£o do valor atual na caixa de spin,
 *          apenas redesenha se o SpinBox estiver na tela atual e precisar atualizaÃ§Ã£o:
 *          - Valida TFT, visibilidade, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Calcula dimensÃµes dos botÃµes e Ã¡rea disponÃ­vel para texto
 *          - Define cor do texto e desenha retÃ¢ngulo de fundo para valor
 *          - Usa a fonte RobotoBold configurada
 *          - Exibe valor atual centralizado na Ã¡rea de texto
 */
void SpinBox::redraw() {
  WidgetStateGuard widgetStateGuard;
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
  char valueText[12];
  snprintf(valueText, sizeof(valueText), "%d", m_currentValue);
  printText(valueText, m_xPos + m_config.width / 2,
            m_yPos + (m_config.height / 2) - 3, MC_DATUM, m_lastArea, m_config.color);
  updateFont(FontType::UNLOAD);

  #endif

}

/**
 * @brief Desenha o fundo do SpinBox, incluindo elementos estÃ¡ticos.
 * @details Cria o container visual, botÃµes de mais e menos para o SpinBox.
 *          Este mÃ©todo Ã© tipicamente chamado uma vez durante setup ao invÃ©s de em cada redesenho:
 *          - Desenha retÃ¢ngulo principal com bordas arredondadas
 *          - Desenha borda do container
 *          - Desenha dois botÃµes com cores clarificadas (lados esquerdo e direito)
 *          - Desenha bordas dos botÃµes
 *          - Desenha sinais de menos (-) no botÃ£o esquerdo e mais (+) no botÃ£o direito
 */
void SpinBox::drawBackground() {
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID

  #if defined(USING_GRAPHIC_LIB)

  // Fundo
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_radius,m_config.color);
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, m_radius,m_config.textColor);

  uint16_t corBotao = lighten565(m_config.color, 0.5);//WidgetBase::lightenColor565(m_config.color, 4);

  // botao left
  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius, corBotao);
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius,m_config.textColor);

  // botao right
  WidgetBase::objTFT->fillRoundRect(m_xPos + m_config.width - m_buttonSize.width, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius, corBotao);
  WidgetBase::objTFT->drawRoundRect(m_xPos + m_config.width - m_buttonSize.width, m_yPos, m_buttonSize.width, m_buttonSize.height, m_radius,m_config.textColor);

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
 * @brief Configura o SpinBox com Ð¿ÐµÑ€Ð¸Ð¼ÐµÑ‚Ñ€Ð¾Ð² definidos em uma estrutura de configuraÃ§Ã£o.
 * @param config Estrutura @ref SpinBoxConfig contendo os parÃ¢metros de configuraÃ§Ã£o.
 * @details Este mÃ©todo deve ser chamado apÃ³s criar o objeto para configurÃ¡-lo adequadamente:
 *          - Limpa memÃ³ria existente usando cleanupMemory()
 *          - Copia profundamente a configuraÃ§Ã£o
 *          - Valida e corrige se minValue > maxValue (troca valores)
 *          - Restringe startValue dentro da faixa vÃ¡lida
 *          - Define callback
 *          - Marca o widget como carregado
 *          A caixa de spin nÃ£o serÃ¡ exibida corretamente atÃ© que este mÃ©todo seja chamado.
 */
void SpinBox::setup(const SpinBoxConfig &config) {
  WidgetStateGuard widgetStateGuard;
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


#if defined(USING_GRAPHIC_LIB)
  m_font = m_config.font ? m_config.font : &RobotoBold8pt7b;
#endif

  m_currentValue = constrain(m_config.startValue, m_config.minValue, m_config.maxValue);
  m_callback = config.callback;
  m_loaded = true;
}

/**
 * @brief Recupera o valor atual do SpinBox.
 * @return Valor numÃ©rico atual.
 * @details Retorna o valor armazenado em m_currentValue.
 */
int SpinBox::getValue() {
  WidgetStateGuard widgetStateGuard; return m_currentValue; }

/**
 * @brief Define o valor atual do SpinBox.
 * @param _value Novo valor numÃ©rico para definir.
 * @param triggerCallback True para disparar a callback (padrÃ£o), False para apenas atualizar o valor.
 * @details Atualiza o valor do SpinBox, garante que estÃ¡ dentro da faixa definida,
 *          e marca o widget para redesenho. Dispara o callback se solicitado:
 *          - Valida se widget estÃ¡ carregado
 *          - Restringe valor usando constrain() dentro da faixa min/max
 *          - Marca para redesenho
 *          - Dispara callback se configurado e triggerCallback for true
 */
void SpinBox::setValue(int _value, bool triggerCallback) {
  WidgetStateGuard widgetStateGuard;
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
 * @brief Torna o SpinBox visÃ­vel na tela.
 * @details Define o widget como visÃ­vel e marca para redesenho.
 */
void SpinBox::show() {
  WidgetStateGuard widgetStateGuard;
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o SpinBox da tela.
 * @details Define o widget como invisÃ­vel e marca para redesenho.
 */
void SpinBox::hide() {
  WidgetStateGuard widgetStateGuard;
  m_visible = false;
  m_shouldRedraw = true;
}

/**
 * @brief ForÃ§a o SpinBox a atualizar.
 * @details Define a flag de atualizaÃ§Ã£o para disparar um redesenho no prÃ³ximo ciclo.
 */
void SpinBox::forceUpdate() {
  WidgetStateGuard widgetStateGuard; m_shouldRedraw = true; }
#endif // DFK_DISABLE_SPINBOX
