#include "wtextbox.h"
#include "../../displaywx.h"

const char* TextBox::TAG = "[TextBox]";

/**
 * @brief Construtor para a classe TextBox.
 * @param _x Coordenada X para a posição do TextBox.
 * @param _y Coordenada Y para a posição do TextBox.
 * @param _screen Identificador da tela onde o TextBox será exibido.
 * @details Inicializa o widget com valores padrão (padding 3) e configuração vazia.
 *          A caixa de texto não será funcional até que setup() seja chamado.
 */
TextBox::TextBox(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_padding(3), m_shouldRedraw(true) {}

/**
 * @brief Construtor padrão para o TextBox.
 * @details Cria um TextBox na posição (0,0) na tela 0.
 */
TextBox::TextBox() : WidgetBase(0, 0, 0) {}

/**
 * @brief Destrutor da classe TextBox.
 * @details Limpa ponteiros para fonte, função de tela pai e função callback.
 *          O objeto CharString gerencia sua própria limpeza de memória.
 */
TextBox::~TextBox() {
// A classe CharString cuida de sua própria memória
// A fonte é apenas uma referência, não precisamos liberar
#if defined(USING_GRAPHIC_LIB)
  m_font = nullptr;
#endif
  parentScreen = nullptr;
  m_callback = nullptr;
}

/**
 * @brief Detecta se o TextBox foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do TextBox, False caso contrário.
 * @details Quando tocado, ativa o modo de teclado virtual para entrada de texto.
 *          Valida visibilidade, uso de teclado, tela atual, carregamento, debounce, habilitado e bloqueado.
 */
bool TextBox::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) {
  CHECK_VISIBLE_BOOL
  CHECK_USINGKEYBOARD_BOOL
  CHECK_CURRENTSCREEN_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL
  CHECK_POINTER_TOUCH_NULL_BOOL

  bool inBounds = POINT_IN_RECT(*_xTouch, *_yTouch, m_xPos, m_yPos, m_width, m_height);
  if(inBounds) {
    m_myTime = millis();
  }

  // bool detectado = false;
  uint16_t xMax = m_xPos + m_width;
  uint16_t yMax = m_yPos + m_height;

  if ((*_xTouch > m_xPos) && (*_xTouch < xMax) && (*_yTouch > m_yPos) &&
      (*_yTouch < yMax) && WidgetBase::usingKeyboard == false) {
    setPressed(true);  // Mark widget as pressed
    WidgetBase::usingKeyboard = true;
    ESP_LOGD(TAG, "Open keyboard");

    return true;
  }

  return false;
}

/**
 * @brief Recupera a função callback associada ao TextBox.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o TextBox for utilizado.
 */
functionCB_t TextBox::getCallbackFunc() { return m_callback; }

/**
 * @brief Redesenha o TextBox na tela, atualizando sua aparência.
 * @details Exibe o valor de texto atual com fonte e estilo apropriados.
 *          Apenas redesenha se o TextBox estiver na tela atual e precisar atualização:
 *          - Valida visibilidade, TFT, tela atual, carregamento e flag de redesenho
 *          - Configura fonte (usando fonte configurada ou padrão)
 *          - Desenha retângulo de fundo com cor de fundo
 *          - Desenha borda com cor de letra
 *          - Exibe texto com formatação para caber no espaço disponível
 */
void TextBox::redraw() {
  CHECK_TFT_VOID
  if (!m_visible) {
    return;
  }
#if defined(DISP_DEFAULT)
  if (WidgetBase::currentScreen != m_screen || !m_loaded || !m_shouldRedraw) {
    return;
  }

  m_shouldRedraw = false;

  if (m_font) {
    WidgetBase::objTFT->setFont(m_font);
  } else {
    updateFont(FontType::NORMAL);
  }

  ESP_LOGD(TAG, "Redraw textbox with value %s", m_value.getString());

  // TextBound_t area;
  // WidgetBase::objTFT->getTextBounds("M", m_xPos, m_yPos, &area.x, &area.y,
  // &area.width, &area.height);

  WidgetBase::objTFT->fillRect(m_xPos, m_yPos, m_width, m_height,
                               m_backgroundColor);
  WidgetBase::objTFT->drawRect(m_xPos, m_yPos, m_width, m_height, m_letterColor);
  WidgetBase::objTFT->setTextColor(m_letterColor);

  // uint16_t qtdLetrasMax = m_width / area.width;
  // const char *conteudo = m_value.getFirstChars(qtdLetrasMax);
  char conteudo[256];
  int n = getFirstLettersForSpace(m_value.getString(), m_width * 0.9, m_height * 0.9, conteudo, sizeof(conteudo));
  if(n > 0){
    printText(conteudo, m_xPos + m_padding, m_yPos + m_height / 2, ML_DATUM);
  }
  // log_d("Draw %d letters from %s in space %d", qtdLetrasMax, conteudo,
  // m_width);
  

  updateFont(FontType::UNLOAD);
#endif
}

/**
 * @brief Configura o widget TextBox com parâmetros específicos de uma estrutura de configuração.
 * @param config A estrutura de configuração contendo todos os parâmetros de setup.
 * @details Este método delega para o método setup() com parâmetros individuais.
 */
void TextBox::setup(const TextBoxConfig &config) {
  CHECK_TFT_VOID
#if defined(USING_GRAPHIC_LIB)
  setup(config.width, config.height, config.letterColor, config.backgroundColor,
        config.startValue, config.font, config.funcPtr, config.cb);
#endif
}

/**
 * @brief Simula um clique no TextBox.
 * @details Abre o teclado de letras. O callback configurado só roda quando a entrada é confirmada.
 */
void TextBox::onClick()
{
  if (!m_loaded)
  {
    ESP_LOGE(TAG, "TextBox widget not loaded");
    return;
  }
#if defined(DFK_TEXTBOX)
  if (DisplayWX::instance == nullptr)
  {
    ESP_LOGE(TAG, "DisplayWX instance missing");
    return;
  }
  ESP_LOGD(TAG, "TextBox clicked");
  DisplayWX::instance->openTextBox(this);
#endif
}

/**
 * @brief Configura o widget TextBox com dimensões, cores e valor inicial específicos.
 * @param _width Largura da caixa de texto.
 * @param _height Altura da caixa de texto.
 * @param _letterColor Cor para o texto exibindo o valor.
 * @param _backgroundColor Cor de fundo da caixa de texto.
 * @param _startValue Valor de texto inicial para exibir.
 * @param _font Fonte para usar no texto.
 * @param _funcPtr Ponteiro de função para a função de carregamento da tela pai.
 * @param _cb Função callback para executar na interação.
 * @details Inicializa as propriedades do TextBox e o marca como carregado quando completo.
 *          Ajusta altura baseado em métricas de fonte se uma fonte for especificada.
 */
#if defined(USING_GRAPHIC_LIB)
void TextBox::setup(uint16_t _width, uint16_t _height, uint16_t _letterColor,
                    uint16_t _backgroundColor, const char *_startValue,
                    const GFXfont *_font, functionLoadScreen_t _funcPtr,
                    functionCB_t _cb) {
  if (!WidgetBase::objTFT) {
    ESP_LOGE(TAG, "TFT not defined on WidgetBase");
    return;
  }
  parentScreen = _funcPtr;
  m_width = _width;
  m_height = _height;

#if defined(DISP_DEFAULT)
  if (_font) {
    WidgetBase::objTFT->setFont(_font);
    TextBound_t area;
    WidgetBase::objTFT->getTextBounds("Mp", m_xPos, m_yPos, &area.x, &area.y,
                                      &area.width, &area.height);
    m_height = area.height + (m_padding * 2);
  }
#endif

  m_letterColor = _letterColor;
  m_backgroundColor = _backgroundColor;
  m_value.setString(_startValue, true);
  m_font = _font;
  m_callback = _cb;

  m_loaded = true;
}
#endif

/**
 * @brief Define o valor de texto exibido pelo TextBox.
 * @param str Valor de texto para definir.
 * @details Atualiza o texto exibido e marca o TextBox para redesenho:
 *          - Define string de valor usando setString()
 *          - Marca para redesenho usando forceUpdate()
 */
void TextBox::setValue(const char *str) {
  m_value.setString(str, true);
  ESP_LOGD(TAG, "Set value for textbox: %s", str);
  forceUpdate();
}

/**
 * @brief Força o TextBox a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void TextBox::forceUpdate() { m_shouldRedraw = true; }

/**
 * @brief Recupera o valor de texto atual do TextBox.
 * @return Ponteiro para a representação em string do valor atual.
 * @details Retorna ponteiro para a string interna que representa o valor.
 */
const char *TextBox::getValue() { return m_value.getString(); }

/**
 * @brief Torna o TextBox visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void TextBox::show() {
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o TextBox da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void TextBox::hide() {
  m_visible = false;
  m_shouldRedraw = true;
}