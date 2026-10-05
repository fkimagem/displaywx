#include "wnumberbox.h"
#include "../../check_touch.h"

const char* NumberBox::TAG = "NumberBox";

/**
 * @brief Converte um valor float para uma representação em string.
 * @param f Valor float para converter.
 * @return Representação em string do valor float.
 * @details Nota: Esta função usa um buffer estático, então seu valor de retorno é válido
 *          apenas até a próxima chamada.
 */
void NumberBox::convertoToString(double f, char* buf, size_t bufSize) {
  //return String(f);
  if (buf && bufSize > 0) {
    snprintf(buf, bufSize, "%.*f", (int)m_config.decimalPlaces, f);
  }
}

/**
 * @brief Constrói um widget NumberBox em uma posição especificada em uma tela especificada.
 * @param _x Coordenada X para a posição do NumberBox.
 * @param _y Coordenada Y para a posição do NumberBox.
 * @param _screen Identificador da tela onde o NumberBox será exibido.
 * @details Inicializa o widget com valores padrão (padding 3) e configuração vazia.
 *          O NumberBox não será funcional até que setup() seja chamado.
 */
NumberBox::NumberBox(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_padding(3), m_shouldRedraw(true) {
      #if defined(USING_GRAPHIC_LIB)
      m_config = {.funcPtr = nullptr, .callback = nullptr, .font = nullptr, .startValue = 0, .width = 0, .height = 0, .letterColor = 0, .backgroundColor = 0, .decimalPlaces = 2};
      #endif
    }

/**
 * @brief Construtor padrão para o NumberBox.
 * @details Cria um NumberBox na posição (0,0) na tela 0.
 */
NumberBox::NumberBox() : WidgetBase(0, 0, 0), m_padding(3), m_shouldRedraw(true), m_config{} {}

/**
 * @brief Destrutor da classe NumberBox.
 * @details Limpa quaisquer recursos usados pelo NumberBox.
 */
NumberBox::~NumberBox() {
    cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo NumberBox.
 * @details NumberBox não usa alocação dinâmica de memória.
 *          CharString gerencia sua própria memória.
 */
void NumberBox::cleanupMemory() {
    ESP_LOGD(TAG, "NumberBox memory cleanup completed");
}

/**
 * @brief Detecta se o NumberBox foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do NumberBox, False caso contrário.
 * @details Quando tocado, ativa o modo de teclado virtual para entrada de valor.
 *          Valida visibilidade, tela atual, carregamento, debounce e uso de teclado.
 */
bool NumberBox::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_USINGKEYBOARD_BOOL

  
  

  bool inBounds = POINT_IN_RECT(*_xTouch, *_yTouch, m_xPos, m_yPos, m_config.width, m_config.height);

  if (inBounds) {
    m_myTime = millis();
    setPressed(true);  // Mark widget as pressed
    WidgetBase::usingKeyboard = true;
    ESP_LOGD(TAG, "Open keyboard");
    return true;
  }

  return false;
}

/**
 * @brief Recupera a função callback associada ao NumberBox.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o NumberBox for utilizado.
 */
functionCB_t NumberBox::getCallbackFunc() { return m_callback; }

/**
 * @brief Redesenha o NumberBox na tela, atualizando sua aparência.
 * @details Exibe o valor numérico atual com formatação e estilo apropriados.
 *          Apenas redesenha se o NumberBox estiver na tela atual e precisar atualização:
 *          - Valida TFT, visibilidade, tela atual, carregamento e flag de redesenho
 *          - Configura fonte (usando fonte configurada ou padrão)
 *          - Desenha retângulo de fundo com cor de fundo
 *          - Desenha borda com cor de letra
 *          - Exibe texto do valor com formatação para caber no espaço disponível
 */
void NumberBox::redraw() {
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  m_shouldRedraw = false;

  #if defined(USING_GRAPHIC_LIB)

  if (m_font) {
    WidgetBase::objTFT->setFont(m_font);
  } else {
    updateFont(FontType::NORMAL);
  }

  // uint16_t darkBg = WidgetBase::lightMode ? CFK_GREY3 : CFK_GREY11;
  // uint16_t lightBg = WidgetBase::lightMode ? CFK_GREY11 : CFK_GREY3;
  // uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  ESP_LOGD(TAG, "Redraw numberbox with value %s", m_value.getString());
  WidgetBase::objTFT->fillRect(m_xPos, m_yPos, m_config.width, m_config.height,
                               m_config.backgroundColor);
  WidgetBase::objTFT->drawRect(m_xPos, m_yPos, m_config.width, m_config.height, m_config.letterColor);
  WidgetBase::objTFT->setTextColor(m_config.letterColor);

  // TextBound_t area;
  // WidgetBase::objTFT->getTextBounds("M", m_xPos, m_yPos, &area.x, &area.y,
  // &area.width, &area.height); uint16_t qtdLetrasMax = m_width / area.width;
  // const char* conteudo = m_value.getFirstChars(qtdLetrasMax);
  // printText(conteudo, m_xPos + m_padding, m_yPos + m_height/2, ML_DATUM);
  char conteudo[256];

  int n = getFirstLettersForSpace(m_value.getString(), m_config.width * 0.9, m_config.height * 0.9, conteudo, sizeof(conteudo));
  if( n > 0){
    printText(conteudo, m_xPos + m_padding, m_yPos + m_config.height / 2, ML_DATUM);
  }

  // log_d("Draw %d letters from %s in space %d", qtdLetrasMax, conteudo,
  // m_width);
  

  updateFont(FontType::UNLOAD);

  #endif
}

/**
 * @brief Força o NumberBox a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void NumberBox::forceUpdate() { m_shouldRedraw = true; }

#if defined(USING_GRAPHIC_LIB)
/**
 * @brief Configura o widget NumberBox com dimensões, cores e valor inicial específicos.
 * @param _width Largura da caixa de número.
 * @param _height Altura da caixa de número.
 * @param _letterColor Cor para o texto exibindo o valor.
 * @param _backgroundColor Cor de fundo da caixa de número.
 * @param _startValue Valor numérico inicial para exibir.
 * @param _font Fonte para usar no texto.
 * @param _funcPtr Ponteiro de função para a função de carregamento da tela pai.
 * @param _cb Função callback para executar na interação.
 * @details Inicializa as propriedades do NumberBox e o marca como carregado quando completo:
 *          - Define função pai e callback
 *          - Configura largura, altura e fonte
 *          - Calcula altura automaticamente baseado na fonte se necessário
 *          - Define cores e valor inicial
 *          - Marca como carregado
 */
void NumberBox::setup(uint16_t _width, uint16_t _height, uint16_t _letterColor,
                      uint16_t _backgroundColor, float _startValue,
                      const GFXfont *_font, functionLoadScreen_t _funcPtr,
                      functionCB_t _cb) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGE(TAG, "Numberbox already intialized");
    return;
  }

  parentScreen = _funcPtr;
  m_callback = _cb;

  m_config.width = _width;
  m_config.height = _height;
  m_config.font = _font;

#if defined(DISP_DEFAULT)
  if (_font) {
    WidgetBase::objTFT->setFont(_font);
    TextBound_t area;
    WidgetBase::objTFT->getTextBounds("Mp", m_xPos, m_yPos, &area.x, &area.y,
                                      &area.width, &area.height);
    m_config.height = area.height + (m_padding * 2);
  }
#endif

  m_config.letterColor = _letterColor;
  m_config.backgroundColor = _backgroundColor;
  m_value.setStringDouble(_startValue, m_config.decimalPlaces);

  m_loaded = true;
}
#endif

/**
 * @brief Configura o NumberBox com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref NumberBoxConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia profundamente a configuração
 *          - Copia ponteiros de função (parentScreen e callback)
 *          - Define membros de configuração (largura, altura, cores, fonte)
 *          - Define valor inicial usando setString()
 *          - Marca o widget como carregado e inicializado
 *          O NumberBox não será exibido corretamente até que este método seja chamado.
 */
void NumberBox::setup(const NumberBoxConfig &config) {
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGW(TAG, "NumberBox already initialized");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();
  
  // Deep copy configuration
  m_config = config;
  
  // Copy function pointers (no deep copy needed for function pointers)
  parentScreen = config.funcPtr;
  m_callback = config.callback;
  
  // Set member variables from config
  m_config.width = config.width;
  m_config.height = config.height;
  m_config.letterColor = config.letterColor;
  m_config.backgroundColor = config.backgroundColor;
  m_config.decimalPlaces = config.decimalPlaces;
  
  #if defined(USING_GRAPHIC_LIB)
  m_config.font = config.font;
  m_font = config.font;
  #endif
  
  // Set initial value
  m_value.setStringDouble(config.startValue, config.decimalPlaces);
  
  m_loaded = true;
  ESP_LOGD(TAG, "NumberBox configured: %dx%d, value: %f", 
           m_config.width, m_config.height, config.startValue);
}

/**
 * @brief Define o valor numérico exibido pelo NumberBox.
 * @param str Valor numérico para definir.
 * @details Atualiza o valor exibido e marca o NumberBox para redesenho:
 *          - Converte float para string usando setString()
 *          - Marca para redesenho usando forceUpdate()
 */
void NumberBox::setValue(double str) {
  m_value.setStringDouble(str, m_config.decimalPlaces);
  ESP_LOGD(TAG, "Set value for numberbox: %s", m_value.getString());
  forceUpdate();
}

/**
 * @brief Recupera o valor numérico atual do NumberBox.
 * @return Valor numérico atual como double.
 * @details Converte a string interna para valor double usando toDouble().
 */
double NumberBox::getValue() { return m_value.toDouble(); }

/**
 * @brief Recupera o valor numérico atual como string.
 * @return Ponteiro para a representação em string do valor atual.
 * @details Retorna ponteiro para a string interna que representa o valor.
 */
const char *NumberBox::getValueChar() { return m_value.getString(); }

/**
 * @brief Torna o NumberBox visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void NumberBox::show() {
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o NumberBox da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void NumberBox::hide() {
  m_visible = false;
  m_shouldRedraw = true;
}