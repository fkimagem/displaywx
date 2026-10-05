#include "numpad.h"
#include "../../check_touch.h"

const char *Numpad::TAG = "Numpad";

uint16_t Numpad::m_backgroundColor = CFK_GREY3;
uint16_t Numpad::m_letterColor = CFK_BLACK;
uint16_t Numpad::m_keyColor = CFK_GREY13;

/**
 * @brief 2D array defining the characters displayed on the Numpad keys
 * Layout:
 * 7 8 9 +/-
 * 4 5 6 Del
 * 1 2 3 .
 * -- 0 ++ OK
 */
const Key_t Numpad::m_pad[NCOLS][NROWS] = {
    // Linha 0 - Números e +/-
    {{"7", PressedKeyType::NUMBER},
     {"8", PressedKeyType::NUMBER},
     {"9", PressedKeyType::NUMBER},
     {"+/-", PressedKeyType::INVERT_VALUE}},

    // Linha 1 - Números e Del
    {{"4", PressedKeyType::NUMBER},
     {"5", PressedKeyType::NUMBER},
     {"6", PressedKeyType::NUMBER},
     {"Del", PressedKeyType::DEL}},

    // Linha 2 - Números e ponto decimal
    {{"1", PressedKeyType::NUMBER},
     {"2", PressedKeyType::NUMBER},
     {"3", PressedKeyType::NUMBER},
     {".", PressedKeyType::POINT}},

    // Linha 3 - Controles especiais
    {{"--", PressedKeyType::DECREMENT},
     {"0", PressedKeyType::NUMBER},
     {"++", PressedKeyType::INCREMENT},
     {"OK", PressedKeyType::RETURN}}};

/**
 * @brief Constrói um widget Numpad em uma posição especificada em uma tela especificada.
 * @param _x Coordenada X para a posição do Numpad.
 * @param _y Coordenada Y para a posição do Numpad.
 * @param _screen Identificador da tela onde o Numpad será exibido.
 * @details Inicializa o widget com valores padrão e configuração vazia.
 *          O teclado não será funcional até que setup() seja chamado.
 */
Numpad::Numpad(uint16_t _x, uint16_t _y, uint8_t _screen)
    : WidgetBase(_x, _y, _screen), m_config{}, m_pressedKey{0, 0, false} {}

/**
 * @brief Construtor padrão para a classe Numpad.
 * @details Inicializa um Numpad na posição (0,0) na tela 0.
 */
Numpad::Numpad() : WidgetBase(0, 0, 0), m_config{}, m_pressedKey{0, 0, false} {}

/**
 * @brief Destrutor da classe Numpad.
 * @details Limpa quaisquer recursos usados pelo Numpad.
 */
Numpad::~Numpad()
{
  cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo Numpad.
 * @details Numpad não usa alocação dinâmica de memória.
 *          CharString gerencia sua própria memória.
 */
void Numpad::cleanupMemory()
{
  ESP_LOGD(TAG, "Numpad memory cleanup completed");
}

/**
 * @brief Detecta se o Numpad foi tocado.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque está dentro da área do Numpad, False caso contrário.
 * @details Método override que sempre retorna false. Use detectTouch() com pressedKey para detecção real.
 */
bool Numpad::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch) { return false; }

/**
 * @brief Recupera a função callback associada ao Numpad.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o teclado for utilizado.
 */
functionCB_t Numpad::getCallbackFunc() { return m_callback; }

/**
 * @brief Detecta se uma tecla específica do Numpad foi tocada e retorna o tipo de tecla.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @param pressedKey Ponteiro para armazenar o tipo de tecla pressionada.
 * @return True se uma tecla é detectada, False caso contrário.
 * @details Este método realiza múltiplas validações e detecta qual tecla foi pressionada:
 *          - Valida visibilidade, inicialização, carregamento e habilitado
 *          - Calcula índice da tecla baseado na posição do toque
 *          - Processa diferentes tipos de teclas (NUMBER, POINT, RETURN, DEL, INCREMENT, DECREMENT, INVERT_VALUE)
 *          - Atualiza conteúdo e redesenha preview quando necessário
 */
bool Numpad::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch,
                         PressedKeyType *pressedKey)
{

  CHECK_VISIBLE_BOOL
  CHECK_INITIALIZED_BOOL
  CHECK_LOADED_BOOL
  CHECK_DEBOUNCE_CLICK_BOOL
  CHECK_ENABLED_BOOL
  CHECK_LOCKED_BOOL
  CHECK_POINTER_TOUCH_NULL_BOOL
  bool retorno = false;
  uint16_t xMax = m_xPos + m_availableWidth;
  uint16_t yMax = m_yPos + m_availableHeight;
  (*pressedKey) = PressedKeyType::NONE;

  if (!m_field)
  {
    return false;
  }

  m_myTime = millis();

  bool inBoundsBackKey = POINT_IN_RECT(*_xTouch, *_yTouch, m_backKeyPos.x, m_backKeyPos.y, m_keyW / 2, m_keyH / 2);
  if (inBoundsBackKey)
  {
    (*pressedKey) = PressedKeyType::ESC;
    m_content.setString(m_previousContent, true);
    return true;
  }

  bool inBounds = POINT_IN_RECT(*_xTouch, *_yTouch, m_xPos, m_yPos, m_availableWidth, m_availableHeight);
  if (inBounds)
  {
    m_myTime = millis();
  }

  if ((*_xTouch > m_xPos) && (*_xTouch < xMax) && (*_yTouch > m_yPos) &&
      (*_yTouch < yMax))
  {
    (*pressedKey) = PressedKeyType::NUMBER;

    int16_t aux_xIndexClick = ((*_xTouch) - m_xPos) / (m_keyW + 2);
    int16_t aux_yIndexClick = ((*_yTouch) - m_yPos) / (m_keyH + 2);

    uint16_t xIndexClick = constrain(aux_xIndexClick, 0, Numpad::aCols - 1);
    uint16_t yIndexClick = constrain(aux_yIndexClick, 0, Numpad::aRows - 1);

    const Key_t letter = m_pad[yIndexClick][xIndexClick];

    // Verifica se é tecla vazia antes de marcar como pressionada
    if (letter.type == PressedKeyType::EMPTY || letter.label[0] == '\0')
    {
      ESP_LOGD(TAG, "Empty key. None action.");
      return false;
    }

    // Marca tecla como pressionada e atualiza visual
    pressKey(yIndexClick, xIndexClick);

    if (letter.type == PressedKeyType::INVERT_VALUE)
    {
      ESP_LOGD(TAG, "Invert value");

      double v = m_content.toDouble() * -1;
      m_content.setStringDouble(v, m_field->getConfig().decimalPlaces);

      drawKeys(false, true);
      // Libera tecla após processar (teclas especiais retornam imediatamente)
      releaseKey();
      (*pressedKey) = PressedKeyType::INVERT_VALUE;
      return true;
    }
    if (letter.type == PressedKeyType::DECREMENT)
    {
      ESP_LOGD(TAG, "Decrement value");

      double v = m_content.toDouble();
      v--;
      m_content.setStringDouble(v, m_field->getConfig().decimalPlaces);

      drawKeys(false, true);
      // Libera tecla após processar (teclas especiais retornam imediatamente)
      releaseKey();
      (*pressedKey) = PressedKeyType::DECREMENT;
      return true;
    }
    if (letter.type == PressedKeyType::INCREMENT)
    {
      ESP_LOGD(TAG, "Increment value");

      double v = m_content.toDouble();
      v++;
      m_content.setStringDouble(v, m_field->getConfig().decimalPlaces);

      drawKeys(false, true);
      // Libera tecla após processar (teclas especiais retornam imediatamente)
      releaseKey();
      (*pressedKey) = PressedKeyType::INCREMENT;
      return true;
    }

    ESP_LOGD(TAG, "Index clicked: %d, %d = %s", xIndexClick, yIndexClick, letter.label);

    switch (letter.type)
    {
    case PressedKeyType::NUMBER:
      ESP_LOGD(TAG, "Is number");
      addLetter((char)letter.label[0]);
      break;
    case PressedKeyType::POINT:
      addLetter((char)letter.label[0]);
      break;
    case PressedKeyType::RETURN:
      (*pressedKey) = PressedKeyType::RETURN;
      break;
    case PressedKeyType::DEL:
      ESP_LOGD(TAG, "Is Delete");
      removeLetter();
      break;
    default:
      ESP_LOGD(TAG, "Another type: %d", letter.type);
      break;
    }

    retorno = true;
  }

  return retorno;
}

void Numpad::drawBackKey(uint16_t x, uint16_t y)
{
  CHECK_TFT_VOID
  CHECK_LOADED_VOID

#if defined(USING_GRAPHIC_LIB)
  WidgetBase::objTFT->setFont(&RobotoBold8pt7b);
  TextBound_t textBounds = WidgetBase::getTextBounds("ESC", 100, 100);
  uint16_t keyPadding = 3;

  uint16_t keyW = textBounds.width + keyPadding * 2;
  uint16_t keyH = textBounds.height + keyPadding * 2;

  WidgetBase::objTFT->fillRoundRect(x, y, keyW, keyH, 4, Numpad::m_keyColor);
  WidgetBase::objTFT->drawRoundRect(x, y, keyW, keyH, 4, Numpad::m_letterColor);
  uint16_t xCenter = x + keyW / 2;
  uint16_t yCenter = y + keyH / 2;

  printText("ESC", xCenter, yCenter, MC_DATUM);
#endif
}

/**
 * @brief Redesenha o Numpad na tela, atualizando sua aparência.
 * @param fullScreen Se true, redesenha toda a tela.
 * @param onlyContent Se true, redesenha apenas a área de conteúdo.
 * @details Desenha o teclado numérico completo:
 *          - Preenche fundo se fullScreen for true
 *          - Desenha área de preview com valor atual
 *          - Desenha todas as teclas se onlyContent for false
 *          - Usa fontes e cores configuradas
 *          - Registra tempo de execução para debug
 */
void Numpad::drawKeys(bool fullScreen, bool onlyContent)
{
  CHECK_TFT_VOID
  CHECK_LOADED_VOID
  //uint32_t startMillis = millis();

#if defined(USING_GRAPHIC_LIB)

  if (fullScreen)
  {
    WidgetBase::objTFT->fillScreen(Numpad::m_backgroundColor);
  }

  WidgetBase::objTFT->fillRect(m_pontoPreview.x, m_pontoPreview.y,
                               m_pontoPreview.width, m_pontoPreview.height,
                               Numpad::m_backgroundColor);
  WidgetBase::objTFT->drawRect(m_pontoPreview.x, m_pontoPreview.y,
                               m_pontoPreview.width, m_pontoPreview.height,
                               Numpad::m_keyColor);

  WidgetBase::objTFT->setTextColor(Numpad::m_keyColor);

  WidgetBase::objTFT->setFont(m_fontPreview);
  char conteudo[256];
  int n = getLastLettersForSpace(m_content.getString(), m_pontoPreview.width * 0.9, m_pontoPreview.height * 0.9, conteudo, sizeof(conteudo));
  if(n > 0){
  printText(conteudo, m_pontoPreview.x + 2,
            m_pontoPreview.y + (m_pontoPreview.height / 2), ML_DATUM,
            m_lastArea, Numpad::m_backgroundColor);
  }

  WidgetBase::objTFT->setTextColor(Numpad::m_letterColor);
  drawBackKey(m_backKeyPos.x, m_backKeyPos.y);

  WidgetBase::objTFT->setFont(m_fontKeys);

  if (!onlyContent)
  {
    WidgetBase::objTFT->fillRect(m_xPos, m_yPos, m_availableWidth,
                                 m_availableHeight, Numpad::m_backgroundColor);
    for (auto row = 0; row < Numpad::aRows; ++row)
    {
      for (auto col = 0; col < Numpad::aCols; ++col)
      {
        const Key_t letter = m_pad[row][col];

        if (letter.type != PressedKeyType::EMPTY)
        {
          uint16_t keyScale = 1;

          // uint16_t xCenter = xPos + (((keyScale * (m_keyW + 2))) * col) +
          // ((keyScale * (m_keyW + 2)) / 2); uint16_t yCenter = m_yPos + ((m_keyH
          // + 2) * row) + (m_keyH / 2);
          const int key_width = m_keyW * keyScale + (2 * (keyScale - 1));
          const int key_height = m_keyH;
          const int key_x = m_xPos + ((m_keyW + 2) * col);
          const int key_y = m_yPos + ((m_keyH + 2) * row);
          const int key_round = 4;

          WidgetBase::objTFT->fillRoundRect(key_x, key_y, key_width, key_height,
                                            key_round, Numpad::m_keyColor);
          WidgetBase::objTFT->drawRoundRect(key_x, key_y, key_width, key_height,
                                            key_round, Numpad::m_letterColor);

          uint16_t xCenter = key_x + key_width / 2;
          uint16_t yCenter = key_y + key_height / 2;

          printText(letter.label, xCenter, yCenter, MC_DATUM);
        }
      }
    }
  }

  //uint32_t endMillis = millis();
  //ESP_LOGD(TAG, "Numpad::redraw: %i ms", endMillis - startMillis);

#endif
}

/**
 * @brief Adiciona um caractere à entrada atual do Numpad.
 * @param c Caractere para adicionar.
 * @details Adiciona caractere ao conteúdo e redesenha preview:
 *          - Previne múltiplos pontos decimais
 *          - Verifica comprimento máximo
 *          - Redesenha área de preview após adicionar
 */
void Numpad::addLetter(char c)
{
  if (c == '.' && m_content.containsChar('.'))
  {
    ESP_LOGE(TAG, "Value of numberbox aleady has . character");
    return;
  }

  if (m_content.addChar(c))
  {
    drawKeys(false, true);
  }
  else
  {
    ESP_LOGE(TAG, "numberbox has reached maximum lenght. The max lenght is %d",
             MAX_LENGTH_CSTR);
  }
}

/**
 * @brief Remove o último caractere da entrada atual do Numpad.
 * @details Remove caractere do conteúdo e redesenha preview.
 */
void Numpad::removeLetter()
{
  if (!m_content.removeLastChar())
  {
    ESP_LOGD(TAG, "Char not removed");
  }
  drawKeys(false, true);
}

/**
 * @brief Configura as configurações do Numpad.
 * @return True se a configuração foi bem-sucedida, False caso contrário.
 * @details Inicializa o layout e aparência do Numpad:
 *          - Calcula dimensões baseadas na tela
 *          - Calcula tamanho de teclas baseado em linhas/colunas
 *          - Define posição e tamanho da área de preview
 *          - Seleciona melhor fonte para teclas
 *          - Marca widget como carregado e inicializado
 */
bool Numpad::setup()
{
  CHECK_TFT_BOOL
  if (m_loaded)
  {
    ESP_LOGW(TAG, "Keyboard already configured");
    return false;
  }
#if defined(DISP_DEFAULT)
  m_screenW = WidgetBase::objTFT->width();
  m_screenH = WidgetBase::objTFT->height();
#endif

  m_availableWidth = m_screenW * 0.9;
  m_availableHeight = (m_screenH * 0.75);

  m_keyW = (m_availableWidth / Numpad::aCols) - 2;
  m_keyH = (m_availableHeight / Numpad::aRows) - 2;

  m_pontoPreview.x = m_screenW / 4;
  m_pontoPreview.y = 1;
  m_pontoPreview.height = (m_screenH * 0.2);
  m_pontoPreview.width = m_screenW / 2;

  m_xPos = (m_screenW - m_availableWidth) / 2;
  m_yPos = m_screenH - m_availableHeight;

  float percentUtilArea = 0.9;
#if defined(USING_GRAPHIC_LIB)
  m_fontKeys = const_cast<GFXfont *>(getBestRobotoBold(
      m_keyW * percentUtilArea, m_keyH * percentUtilArea, "CAP"));
  m_fontPreview = m_fontKeys; // const_cast<GFXfont*>(getBestRobotoBold(m_pontoPreview.width
                              // * percentUtilArea, m_pontoPreview.height *
                              // percentUtilArea, "M"));
#endif
  m_loaded = true;
  m_initialized = true;
  return true;
}

/**
 * @brief Configura o Numpad com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref NumpadConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Limpa memória existente
 *          - Atribui configuração e cores estáticas
 *          - Configura fontes
 *          - Inicializa layout e dimensões
 *          - Marca o widget como carregado e inicializado
 *          O teclado não será exibido corretamente até que este método seja chamado.
 */
void Numpad::setup(const NumpadConfig &config)
{
  CHECK_TFT_VOID
  if (m_loaded)
  {
    ESP_LOGW(TAG, "Numpad already configured");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();

  // Deep copy configuration
  m_config = config;

  // Update static colors from config
  m_backgroundColor = config.backgroundColor;
  m_letterColor = config.letterColor;
  m_keyColor = config.keyColor;

#if defined(USING_GRAPHIC_LIB)
  m_fontKeys = const_cast<GFXfont *>(config.fontKeys);
  m_fontPreview = const_cast<GFXfont *>(config.fontPreview);
#endif

// Initialize layout without calling setup() to avoid recursion
#if defined(DISP_DEFAULT)
  m_screenW = WidgetBase::objTFT->width();
  m_screenH = WidgetBase::objTFT->height();
#endif

  m_availableWidth = m_screenW * 0.9;
  m_availableHeight = (m_screenH * 0.75);

  m_keyW = (m_availableWidth / Numpad::aCols) - 2;
  m_keyH = (m_availableHeight / Numpad::aRows) - 2;

  m_pontoPreview.x = m_screenW / 4;
  m_pontoPreview.y = 1;
  m_pontoPreview.height = (m_screenH * 0.2);
  m_pontoPreview.width = m_screenW / 2;

  m_xPos = (m_screenW - m_availableWidth) / 2;
  m_yPos = m_screenH - m_availableHeight;

  m_loaded = true;
  m_initialized = true;
  ESP_LOGD(TAG, "Numpad configured with custom settings");
}

/**
 * @brief Abre o Numpad associado a um NumberBox específico.
 * @param _field Ponteiro para o NumberBox que receberá a entrada do Numpad.
 * @details Abre o teclado e inicializa entrada:
 *          - Define timeout para evitar detecção imediata de tecla
 *          - Associa NumberBox ao teclado
 *          - Carrega valor atual do NumberBox
 *          - Habilita flag de teclado ativo
 *          - Desenha teclado completo na tela
 */
void Numpad::open(NumberBox *_field)
{
  m_myTime = millis() + (TIMEOUT_CLICK *
                         3); // Espera o tempo de 3 clicks para iniciar a
                             // detecção, evitando apertar tecla assim que abre.
  m_field = _field;
  WidgetBase::usingKeyboard = true;
  m_content.setString(m_field->getValueChar(), true);
  strncpy(m_previousContent, m_content.getString(), MAX_LENGTH_CSTR);
  m_previousContent[MAX_LENGTH_CSTR] = '\0';

#if defined(DISP_DEFAULT)
  WidgetBase::objTFT->setFont(&RobotoBold10pt7b);
#endif

  drawKeys(true, false);
}

/**
 * @brief Fecha o Numpad e aplica o valor digitado.
 * @details Fecha o teclado e atualiza o NumberBox:
 *          - Desabilita flag de teclado ativo
 *          - Aplica valor digitado ao NumberBox usando setValue()
 *          - Limpa fonte da tela
 */
void Numpad::close()
{
  // Libera tecla pressionada antes de fechar
  releaseKey();

#if defined(DISP_DEFAULT)
  WidgetBase::setFontNull();
#endif
  ESP_LOGD(TAG, "Value of content is: %s", m_content.getString());
  ESP_LOGD(TAG, "Value of content is: %f", m_content.toDouble());

  m_field->setValue(m_content.toDouble());

  WidgetBase::usingKeyboard = false;
}

/**
 * @brief Insere um caractere no conteúdo de entrada atual.
 * @param c Caractere para inserir.
 * @details Insere caractere se o teclado estiver aberto:
 *          - Verifica se usingKeyboard está ativo
 *          - Chama addLetter() para adicionar o caractere
 *          - Retorna erro se teclado não estiver aberto
 */
void Numpad::insertChar(char c)
{
  if (WidgetBase::usingKeyboard)
  {
    addLetter(c);
  }
  else
  {
    ESP_LOGE(TAG, "Cant add char. Keyboard is not open");
  }
}

/**
 * @brief Torna o Numpad visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void Numpad::show()
{
  m_visible = true;
  m_shouldRedraw = true;
}

/**
 * @brief Oculta o Numpad da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void Numpad::hide()
{
  m_visible = false;
  m_shouldRedraw = true;
}

/**
 * @brief Força o Numpad a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void Numpad::forceUpdate() { m_shouldRedraw = true; }

/**
 * @brief Redesenha o Numpad na tela.
 * @details Chama drawKeys() para redesenhar o teclado completo.
 */
void Numpad::redraw()
{
  drawKeys(false, false);
}

/**
 * @brief Desenha uma única tecla do Numpad
 * @param row Linha da tecla (0-3)
 * @param col Coluna da tecla (0-3)
 * @param isPressed Se true, desenha com borda destacada (efeito pressionado)
 * @details Redesenha apenas a tecla especificada com feedback visual quando pressionada
 */
void Numpad::drawSingleKey(uint16_t row, uint16_t col, bool isPressed)
{
  CHECK_TFT_VOID
  CHECK_LOADED_VOID

#if defined(USING_GRAPHIC_LIB)

  const Key_t letter = m_pad[row][col];

  if (letter.type == PressedKeyType::EMPTY)
  {
    return; // Não desenha teclas vazias
  }

  uint16_t keyScale = 1;
  const int key_width = m_keyW * keyScale + (2 * (keyScale - 1));
  const int key_height = m_keyH;
  const int key_x = m_xPos + ((m_keyW + 2) * col);
  const int key_y = m_yPos + ((m_keyH + 2) * row);
  const int key_round = 4;

  // Desenha fundo da tecla
  WidgetBase::objTFT->fillRoundRect(key_x, key_y, key_width, key_height,
                                    key_round, Numpad::m_keyColor);

  // Desenha borda (destaque quando pressionada)
  uint16_t borderColor = isPressed ? CFK_WHITE : Numpad::m_letterColor;
  uint8_t borderWeight = isPressed ? 5 : 1;

  for (uint8_t i = 0; i < borderWeight; i++)
  {
    WidgetBase::objTFT->drawRoundRect(key_x + i, key_y + i,
                                      key_width - 2 * i, key_height - 2 * i,
                                      key_round, borderColor);
  }

  // Desenha texto da tecla
  WidgetBase::objTFT->setFont(m_fontKeys);
  WidgetBase::objTFT->setTextColor(Numpad::m_letterColor);
  uint16_t xCenter = key_x + key_width / 2;
  uint16_t yCenter = key_y + key_height / 2;
  printText(letter.label, xCenter, yCenter, MC_DATUM);

#endif
}

/**
 * @brief Marca uma tecla como pressionada e atualiza visual
 * @param row Linha da tecla
 * @param col Coluna da tecla
 * @details Libera tecla anterior (se houver) e marca nova tecla como pressionada
 */
void Numpad::pressKey(uint16_t row, uint16_t col)
{
  // Libera tecla anterior se houver
  if (m_pressedKey.isPressed)
  {
    drawSingleKey(m_pressedKey.row, m_pressedKey.col, false);
  }

  // Marca nova tecla como pressionada
  m_pressedKey.isPressed = true;
  m_pressedKey.row = row;
  m_pressedKey.col = col;

  // Desenha tecla com efeito pressionado
  drawSingleKey(row, col, true);

  ESP_LOGD(TAG, "Key pressed at row=%d, col=%d", row, col);
}

/**
 * @brief Libera a tecla pressionada e atualiza visual
 * @details Remove efeito visual de pressionado da tecla atual
 */
void Numpad::releaseKey()
{
  if (m_pressedKey.isPressed)
  {
    drawSingleKey(m_pressedKey.row, m_pressedKey.col, false);
    m_pressedKey.isPressed = false;
    ESP_LOGD(TAG, "Key released at row=%d, col=%d", m_pressedKey.row, m_pressedKey.col);
  }
}

/**
 * @brief Verifica se coordenada de touch está dentro de uma tecla
 * @param xTouch Coordenada X do touch
 * @param yTouch Coordenada Y do touch
 * @param keyState Estado da tecla a verificar
 * @return True se touch está dentro da tecla, false caso contrário
 */
bool Numpad::isPointInKey(uint16_t xTouch, uint16_t yTouch, const KeyState &keyState)
{
  if (!keyState.isPressed)
  {
    return false;
  }

  const int key_x = m_xPos + ((m_keyW + 2) * keyState.col);
  const int key_y = m_yPos + ((m_keyH + 2) * keyState.row);
  const int key_width = m_keyW;
  const int key_height = m_keyH;

  return POINT_IN_RECT(xTouch, yTouch, key_x, key_y, key_width, key_height);
}