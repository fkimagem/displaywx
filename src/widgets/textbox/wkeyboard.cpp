#include "wkeyboard.h"
#include "../../check_touch.h"

const char *WKeyboard::TAG = "[WKeyboard]";
/**
 * @brief Lowercase alphabet layout for the keyboard.
 *
 * The layout is organized in a grid with numbers on top and controls in strategic positions:
 * - Row 1: Numbers 1-0
 * - Row 2-4: Lowercase letters
 * - Row 5: Special characters and controls (DEL, CAP, CLR, OK)
 */
/*const char *WKeyboard::m_alphabet[WKeyboard::aRows][WKeyboard::aCols] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
    {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"},
    {"a", "s", "d", "f", "g", "h", "j", "k", "l", "DEL"},
    {"z", "x", "c", "v", "b", "n", "m", " ", "\0", "CAP"},
    {":", ",", "+", "-", "*", "/", "=", ".", "CLR", "OK"}};*/

uint16_t WKeyboard::m_backgroundColor = CFK_GREY3;
uint16_t WKeyboard::m_letterColor = CFK_BLACK;
uint16_t WKeyboard::m_keyColor = CFK_GREY13;

const Key_t WKeyboard::m_alphabet[AROWS][ACOLS] = {
    // Linha 0 - Números
    {{"1", PressedKeyType::NUMBER}, {"2", PressedKeyType::NUMBER}, {"3", PressedKeyType::NUMBER}, {"4", PressedKeyType::NUMBER}, {"5", PressedKeyType::NUMBER}, {"6", PressedKeyType::NUMBER}, {"7", PressedKeyType::NUMBER}, {"8", PressedKeyType::NUMBER}, {"9", PressedKeyType::NUMBER}, {"0", PressedKeyType::NUMBER}},

    // Linha 1 - QWERTY topo
    {{"q", PressedKeyType::LETTER}, {"w", PressedKeyType::LETTER}, {"e", PressedKeyType::LETTER}, {"r", PressedKeyType::LETTER}, {"t", PressedKeyType::LETTER}, {"y", PressedKeyType::LETTER}, {"u", PressedKeyType::LETTER}, {"i", PressedKeyType::LETTER}, {"o", PressedKeyType::LETTER}, {"p", PressedKeyType::LETTER}},

    // Linha 2 - QWERTY meio + DEL
    {{"a", PressedKeyType::LETTER}, {"s", PressedKeyType::LETTER}, {"d", PressedKeyType::LETTER}, {"f", PressedKeyType::LETTER}, {"g", PressedKeyType::LETTER}, {"h", PressedKeyType::LETTER}, {"j", PressedKeyType::LETTER}, {"k", PressedKeyType::LETTER}, {"l", PressedKeyType::LETTER}, {"DEL", PressedKeyType::DEL}},

    // Linha 3 - QWERTY base + espaço, nulo, caps
    {{"z", PressedKeyType::LETTER}, {"x", PressedKeyType::LETTER}, {"c", PressedKeyType::LETTER}, {"v", PressedKeyType::LETTER}, {"b", PressedKeyType::LETTER}, {"n", PressedKeyType::LETTER}, {"m", PressedKeyType::LETTER}, {" ", PressedKeyType::CONTROL}, {"\0", PressedKeyType::EMPTY}, {"CAP", PressedKeyType::CAPS}},

    // Linha 4 - Símbolos, limpar, ok
    {{":", PressedKeyType::ANOTHER}, {",", PressedKeyType::ANOTHER}, {"+", PressedKeyType::SIGN}, {"-", PressedKeyType::SIGN}, {"*", PressedKeyType::SIGN}, {"/", PressedKeyType::SIGN}, {"=", PressedKeyType::SIGN}, {".", PressedKeyType::ANOTHER}, {"CLR", PressedKeyType::CLR}, {"OK", PressedKeyType::RETURN}}};

/**
 * @brief Uppercase alphabet layout for the keyboard.
 *
 * The layout is organized in a grid with special characters on top and controls in strategic positions:
 * - Row 1: Special characters
 * - Row 2-4: Uppercase letters
 * - Row 5: Special characters and controls (DEL, CAP, CLR, OK)
 */
/*const char *WKeyboard::m_alphabetCap[WKeyboard::aRows][WKeyboard::aCols] = {
    {"!", "@", "#", "$", "%", "^", "&", "?", "(", ")"},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", "DEL"},
    {"Z", "X", "C", "V", "B", "N", "M", "{", "}", "CAP"},
    {"_", "'", ":", "<", ">", ";", "~", "[", "]", "OK"}};*/

const Key_t WKeyboard::m_alphabetCap[AROWS][ACOLS] = {
    // Linha 0 - Símbolos especiais
    {{"!", PressedKeyType::SIGN}, {"@", PressedKeyType::SIGN}, {"#", PressedKeyType::SIGN}, {"$", PressedKeyType::SIGN}, {"%", PressedKeyType::SIGN}, {"^", PressedKeyType::SIGN}, {"&", PressedKeyType::SIGN}, {"?", PressedKeyType::SIGN}, {"(", PressedKeyType::SIGN}, {")", PressedKeyType::SIGN}},

    // Linha 1 - QWERTY topo maiúsculo
    {{"Q", PressedKeyType::LETTER}, {"W", PressedKeyType::LETTER}, {"E", PressedKeyType::LETTER}, {"R", PressedKeyType::LETTER}, {"T", PressedKeyType::LETTER}, {"Y", PressedKeyType::LETTER}, {"U", PressedKeyType::LETTER}, {"I", PressedKeyType::LETTER}, {"O", PressedKeyType::LETTER}, {"P", PressedKeyType::LETTER}},

    // Linha 2 - QWERTY meio maiúsculo + DEL
    {{"A", PressedKeyType::LETTER}, {"S", PressedKeyType::LETTER}, {"D", PressedKeyType::LETTER}, {"F", PressedKeyType::LETTER}, {"G", PressedKeyType::LETTER}, {"H", PressedKeyType::LETTER}, {"J", PressedKeyType::LETTER}, {"K", PressedKeyType::LETTER}, {"L", PressedKeyType::LETTER}, {"DEL", PressedKeyType::DEL}},

    // Linha 3 - QWERTY base maiúsculo + chaves, caps
    {{"Z", PressedKeyType::LETTER}, {"X", PressedKeyType::LETTER}, {"C", PressedKeyType::LETTER}, {"V", PressedKeyType::LETTER}, {"B", PressedKeyType::LETTER}, {"N", PressedKeyType::LETTER}, {"M", PressedKeyType::LETTER}, {"{", PressedKeyType::SIGN}, {"}", PressedKeyType::SIGN}, {"CAP", PressedKeyType::CAPS}},

    // Linha 4 - Símbolos diversos, ok
    {{"_", PressedKeyType::SIGN}, {"'", PressedKeyType::SIGN}, {":", PressedKeyType::SIGN}, {"<", PressedKeyType::SIGN}, {">", PressedKeyType::SIGN}, {";", PressedKeyType::SIGN}, {"~", PressedKeyType::SIGN}, {"[", PressedKeyType::SIGN}, {"]", PressedKeyType::SIGN}, {"OK", PressedKeyType::RETURN}}};

/**
 * @brief Constrói um widget WKeyboard em uma posição especificada em uma tela especificada.
 * @param _x Coordenada X para a posição do WKeyboard.
 * @param _y Coordenada Y para a posição do WKeyboard.
 * @param _screen Identificador da tela onde o WKeyboard será exibido.
 * @details Inicializa o teclado com valores padrão.
 *          O teclado não será funcional até que setup() seja chamado.
 */
WKeyboard::WKeyboard(uint16_t _x, uint16_t _y, uint8_t _screen) : WidgetBase(_x, _y, _screen)
{
}

/**
 * @brief Construtor padrão para o WKeyboard.
 * @details Inicializa um teclado na posição (0,0) na tela 0.
 */
WKeyboard::WKeyboard() : WidgetBase(0, 0, 0)
{
}

/**
 * @brief Destrutor da classe WKeyboard.
 * @details Limpa quaisquer recursos usados pelo WKeyboard.
 */
WKeyboard::~WKeyboard()
{
}

/**
 * @brief Detects if the keyboard has been touched.
 * @param _xTouch Pointer to the X-coordinate of the touch.
 * @param _yTouch Pointer to the Y-coordinate of the touch.
 * @return True if the touch is within the keyboard area, otherwise false.
 *
 * This base implementation always returns false as the actual detection
 * is performed by the specialized detectTouch method.
 */
bool WKeyboard::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
    return false;
}

/**
 * @brief Recupera a função callback associada ao teclado.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o teclado for utilizado.
 */
functionCB_t WKeyboard::getCallbackFunc()
{
    return m_callback;
}

/**
 * @brief Detects touch events and identifies the type of key pressed.
 * @param _xTouch Pointer to the X-coordinate of the touch.
 * @param _yTouch Pointer to the Y-coordinate of the touch.
 * @param pressedKey Pointer to a variable to store the detected key type.
 * @return True if a key is detected and processed, otherwise false.
 *
 * This method handles the core keyboard interaction by detecting which key
 * was pressed and performing the appropriate action (adding/removing characters,
 * toggling caps lock, etc.).
 */
bool WKeyboard::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch, PressedKeyType *pressedKey)
{
#if defined(HAS_TOUCH)
    bool retorno = false;
    uint16_t xMax = m_xPos + m_availableWidth;
    uint16_t yMax = m_yPos + m_availableHeight;
    (*pressedKey) = PressedKeyType::NONE;

    if (millis() - m_myTime < TIMEOUT_CLICK)
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

    if ((*_xTouch > m_xPos) && (*_xTouch < xMax) && (*_yTouch > m_yPos) && (*_yTouch < yMax))
    {
        (*pressedKey) = PressedKeyType::LETTER;

        int16_t aux_xIndexClick = ((*_xTouch) - m_xPos) / (m_keyW + 2);
        int16_t aux_yIndexClick = ((*_yTouch) - m_yPos) / (m_keyH + 2);

        uint16_t xIndexClick = constrain(aux_xIndexClick, 0, WKeyboard::aCols - 1);
        uint16_t yIndexClick = constrain(aux_yIndexClick, 0, WKeyboard::aRows - 1);

        Key_t letter = m_capsLock ? (m_alphabetCap[yIndexClick][xIndexClick]) : (m_alphabet[yIndexClick][xIndexClick]);

        // if (yIndexClick == 3 && xIndexClick == 9)
        if (letter.type == PressedKeyType::CAPS)
        {
            m_capsLock = !m_capsLock;
            drawKeys(false, false);
            (*pressedKey) = PressedKeyType::CAPS;
            return true;
        }

        // const char *letter = (Keyboard::_alphabet[yIndexClick][xIndexClick]);

        if (letter.type == PressedKeyType::EMPTY)
        {
            ESP_LOGD(TAG, "Empty key. Changing to previous one.");
            xIndexClick--;
            letter = m_capsLock ? (m_alphabetCap[yIndexClick][xIndexClick]) : (m_alphabet[yIndexClick][xIndexClick]);
        }

        ESP_LOGD(TAG, "Index clicked: %d, %d = %s", xIndexClick, yIndexClick, letter.label);

        switch (letter.type)
        {
        case PressedKeyType::NONE:
            break;
        case PressedKeyType::LETTER:
            addLetter((char)letter.label[0]);
            break;
        case PressedKeyType::NUMBER:
            addLetter((char)letter.label[0]);
            break;
        case PressedKeyType::ANOTHER:
            addLetter((char)letter.label[0]);
            break;
        case PressedKeyType::CONTROL:
            addLetter((char)letter.label[0]);
            break;
        case PressedKeyType::SHIFT:
            break;
        case PressedKeyType::CAPS:
            m_capsLock = !m_capsLock;
            drawKeys(false, false);
            break;
        case PressedKeyType::DEL:
            removeLetter();
            break;
        case PressedKeyType::CLR:
            clear();
            break;
        case PressedKeyType::SIGN:
            addLetter((char)letter.label[0]);
            break;
        case PressedKeyType::RETURN:
            (*pressedKey) = PressedKeyType::RETURN;
            break;

        case PressedKeyType::INVERT_VALUE:
        case PressedKeyType::INCREMENT:
        case PressedKeyType::DECREMENT:
        default:
            break;
        }

        retorno = true;
    }

    return retorno;
#else
    return false;
#endif
}

void WKeyboard::drawBackKey(uint16_t x, uint16_t y)
{
    CHECK_TFT_VOID
    CHECK_LOADED_VOID

#if defined(USING_GRAPHIC_LIB)
    WidgetBase::objTFT->setFont(&RobotoBold8pt7b);
    TextBound_t textBounds = WidgetBase::getTextBounds("ESC", 100, 100);
    uint16_t keyPadding = 3;

    uint16_t keyW = textBounds.width + keyPadding * 2;
    uint16_t keyH = textBounds.height + keyPadding * 2;

    WidgetBase::objTFT->fillRoundRect(x, y, keyW, keyH, 4, WKeyboard::m_keyColor);
    WidgetBase::objTFT->drawRoundRect(x, y, keyW, keyH, 4, WKeyboard::m_letterColor);
    uint16_t xCenter = x + keyW / 2;
    uint16_t yCenter = y + keyH / 2;

    printText("ESC", xCenter, yCenter, MC_DATUM);

#endif
}

/**
 * @brief Redesenha o teclado na tela.
 * @param fullScreen Se true, redesenha a tela inteira; caso contrário, apenas o teclado.
 * @param onlyContent Se true, redesenha apenas a área de conteúdo sem bordas ou fundo.
 * @details Este método manipula o desenho do layout do teclado incluindo teclas, área de exibição
 *          de conteúdo e manipulação dos diferentes estados (caps lock ligado/desligado).
 */
void WKeyboard::drawKeys(bool fullScreen, bool onlyContent)
{
#if defined(USING_GRAPHIC_LIB)
    if (!m_loaded)
    {
        ESP_LOGW(TAG, "Keyboard not loaded");
        return;
    }
    //uint32_t startMillis = millis();
    if (fullScreen)
    {
        WidgetBase::objTFT->fillScreen(WKeyboard::m_backgroundColor);
    }

    WidgetBase::objTFT->fillRect(m_pontoPreview.x, m_pontoPreview.y, m_pontoPreview.width, m_pontoPreview.height, WKeyboard::m_backgroundColor);
    WidgetBase::objTFT->drawRect(m_pontoPreview.x, m_pontoPreview.y, m_pontoPreview.width, m_pontoPreview.height, WKeyboard::m_keyColor);

    WidgetBase::objTFT->setTextColor(WKeyboard::m_keyColor);

    WidgetBase::objTFT->setFont(m_fontPreview);
    char conteudo[256];

    int n = getLastLettersForSpace(m_content.getString(), m_pontoPreview.width * 0.9, m_pontoPreview.height * 0.9, conteudo, sizeof(conteudo));
    if(n > 0){
        printText(conteudo, m_pontoPreview.x + 2, m_pontoPreview.y + (m_pontoPreview.height / 2), ML_DATUM, lastArea, WKeyboard::m_backgroundColor);
    }

    WidgetBase::objTFT->setTextColor(WKeyboard::m_letterColor);
    drawBackKey(m_backKeyPos.x, m_backKeyPos.y);

    WidgetBase::objTFT->setFont(m_fontKeys);
    if (!onlyContent)
    {
        WidgetBase::objTFT->fillRect(m_xPos, m_yPos, m_availableWidth, m_availableHeight, WKeyboard::m_backgroundColor);
        // const float percentUtilArea = 0.9;

        for (auto row = 0; row < WKeyboard::aRows; ++row)
        {
            for (auto col = 0; col < WKeyboard::aCols; ++col)
            {
                Key_t letter = m_capsLock ? (m_alphabetCap[row][col]) : (m_alphabet[row][col]);

                if (letter.type != PressedKeyType::EMPTY)
                {
                    uint16_t keyScale = 1;
                    if (col + 1 < WKeyboard::aCols)
                    { // Tem proxima coluna
                        Key_t nextLetter = m_capsLock ? (m_alphabetCap[row][col + 1]) : (m_alphabet[row][col + 1]);
                        if (nextLetter.type == PressedKeyType::EMPTY)
                        {
                            keyScale++;
                        }
                    }
                    const int key_width = m_keyW * keyScale + (2 * (keyScale - 1));
                    const int key_height = m_keyH;
                    const int key_x = m_xPos + ((m_keyW + 2) * col);
                    const int key_y = m_yPos + ((m_keyH + 2) * row);
                    const int key_round = 4;
                    uint16_t xCenter = key_x + key_width / 2;
                    uint16_t yCenter = key_y + key_height / 2;

                    WidgetBase::objTFT->fillRoundRect(key_x, key_y, key_width, key_height, key_round, WKeyboard::m_keyColor);
                    WidgetBase::objTFT->drawRoundRect(key_x, key_y, key_width, key_height, key_round, WKeyboard::m_letterColor);

                    printText(letter.label, xCenter, yCenter, MC_DATUM);
                }
            }
        }
    }

    

    //uint32_t endMillis = millis();
    //ESP_LOGD(TAG, "WKeyboard::redraw: %i ms", endMillis - startMillis);
#endif
}

/**
 * @brief Adiciona um caractere à entrada atual do teclado.
 * @param c Caractere para adicionar.
 * @details Adiciona um caractere à string de conteúdo atual e redesenha a área de conteúdo
 *          para mostrar a atualização.
 */
void WKeyboard::addLetter(char c)
{

    if (m_content.addChar(c))
    {
        drawKeys(false, true);
    }
    else
    {
        ESP_LOGE(TAG, "textbox has reached maximum lenght. The max lenght is %d", MAX_LENGTH_CSTR);
    }
}

/**
 * @brief Remove o último caractere da entrada atual do teclado.
 * @details Deleta o último caractere da string de conteúdo e redesenha a área de conteúdo
 *          para mostrar a atualização.
 */
void WKeyboard::removeLetter()
{
    // content.remove(content.length() - 1);
    m_content.removeLastChar();
    drawKeys(false, true);
}

/**
 * @brief Configura o teclado, configurando parâmetros necessários.
 * @return True se a configuração foi bem-sucedida, False caso contrário.
 * @details Inicializa o layout do teclado, calcula dimensões das teclas e configura
 *          a área de exibição baseada nas dimensões da tela.
 */
bool WKeyboard::setup()
{
    CHECK_TFT_BOOL
#if defined(USING_GRAPHIC_LIB)
    if (!WidgetBase::objTFT)
    {
        ESP_LOGE(TAG, "TFT not defined on WidgetBase");
        return false;
    }
    if (m_loaded)
    {
        ESP_LOGW(TAG, "Keyboard already configured");
        return false;
    }

#if defined(DISP_DEFAULT)
    m_screenW = WidgetBase::objTFT->width();
    m_screenH = WidgetBase::objTFT->height();
#endif

    // log_d("Screen width: %d", screenW);
    // log_d("Screen height: %d", screenH);

    m_availableWidth = m_screenW * 0.9;
    m_availableHeight = m_screenH * 0.75;

    m_keyW = (m_availableWidth / WKeyboard::aCols) - 2;
    m_keyH = (m_availableHeight / WKeyboard::aRows) - 2;

    // log_d("Available width: %d", availableWidth);
    // log_d("Available height: %d", availableHeight);

    // log_d("Key width: %d", _keyW);
    // log_d("Key height: %d", _keyH);

    ESP_LOGD(TAG, "Keys: %i, %i", m_keyW, m_keyH);

    m_pontoPreview.x = m_screenW / 4;
    m_pontoPreview.y = 1;
    m_pontoPreview.height = (m_screenH * 0.2);
    m_pontoPreview.width = m_screenW / 2;

    m_xPos = (m_screenW - m_availableWidth) / 2;
    m_yPos = m_screenH - m_availableHeight;

    float percentUtilArea = 0.9;
#if defined(USING_GRAPHIC_LIB)
    m_fontKeys = const_cast<GFXfont *>(getBestRobotoBold(m_keyW * percentUtilArea, m_keyH * percentUtilArea, "CAP"));
    m_fontPreview = m_fontKeys; // const_cast<GFXfont*>(getBestRobotoBold(m_pontoPreview.width * percentUtilArea, m_pontoPreview.height * percentUtilArea, "M"));
#endif
    m_capsLock = false;
    m_loaded = true;
#endif
    return true;
}

/**
 * @brief Limpa o conteúdo de entrada atual.
 * @details Redefine a string de conteúdo para vazia e redesenha a área de conteúdo.
 */
void WKeyboard::clear()
{
    m_content.setString("", true);
    drawKeys(false, true);
}

/**
 * @brief Abre o teclado para um campo TextBox específico.
 * @param _field Ponteiro para o campo TextBox a ser editado.
 * @details Prepara o teclado para entrada definindo as flags apropriadas e inicializando
 *          o conteúdo com o valor atual do TextBox.
 */
void WKeyboard::open(TextBox *_field)
{
    CHECK_TFT_VOID
    m_myTime = millis() + (TIMEOUT_CLICK * 3); // Espera o tempo de 3 clicks para iniciar a detecção, evitando apertar tecla assim que abre.
    m_field = _field;
    WidgetBase::usingKeyboard = true;
    m_content.setString(m_field->getValue(), true);
    strncpy(m_previousContent, m_content.getString(), MAX_LENGTH_CSTR);
    m_previousContent[MAX_LENGTH_CSTR] = '\0';
#if defined(DISP_DEFAULT)
    if (m_keyH > 20)
    {
        WidgetBase::objTFT->setFont(&RobotoBold10pt7b);
    }
    else
    {
        WidgetBase::objTFT->setFont(&RobotoBold5pt7b);
    }

#endif

    drawKeys(true, false);
}

/**
 * @brief Fecha o teclado e aplica o conteúdo ao campo associado.
 * @details Atualiza o TextBox com o conteúdo editado e limpa recursos.
 */
void WKeyboard::close()
{

    updateFont(FontType::UNLOAD);

    m_field->setValue(m_content.getString());

    // delete field;
    // field = nullptr;
    WidgetBase::usingKeyboard = false;
}

/**
 * @brief Insere um caractere no conteúdo de entrada atual.
 * @param c Caractere para inserir.
 * @details Adiciona um caractere ao conteúdo se o teclado estiver aberto no momento.
 */
void WKeyboard::insertChar(char c)
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
 * @brief Torna o WKeyboard visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void WKeyboard::show()
{
    m_visible = true;
    m_shouldRedraw = true;
}

/**
 * @brief Oculta o WKeyboard da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void WKeyboard::hide()
{
    m_visible = false;
    m_shouldRedraw = true;
}

/**
 * @brief Redesenha o WKeyboard na tela.
 * @details Chama drawKeys() para redesenhar o teclado completo.
 */
void WKeyboard::redraw()
{
    drawKeys(false, false);
}

/**
 * @brief Força o WKeyboard a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void WKeyboard::forceUpdate() { m_shouldRedraw = true; }