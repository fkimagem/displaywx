#include "widgetbase.h"
#include <esp_log.h>

//#define DEBUG_TEXT_BOUND

const char* WidgetBase::TAG = "WidgetBase";

bool WidgetBase::usingKeyboard = false;
uint8_t WidgetBase::currentScreen = 0;
bool WidgetBase::showingLog = false;
bool WidgetBase::lightMode = true;
uint16_t WidgetBase::backgroundColor = 0xffff;
#if defined(USING_GRAPHIC_LIB)
const GFXfont *WidgetBase::fontNormal = nullptr;
const GFXfont *WidgetBase::fontBold = nullptr;
#endif
QueueHandle_t WidgetBase::xFilaCallback;

#if defined(DISP_DEFAULT)
Arduino_GFX *WidgetBase::objTFT = nullptr;
#elif defined(DISP_PCD8544)
Adafruit_PCD8544 *WidgetBase::objTFT = nullptr;
#elif defined(DISP_SSD1306)
Adafruit_SSD1306 *WidgetBase::objTFT = nullptr;
#elif defined(DISP_U8G2)
U8G2 *WidgetBase::objTFT = nullptr;
#else
#error "You need to define the display type in user_setup.h"
#endif

uint16_t WidgetBase::screenWidth = 480;
uint16_t WidgetBase::screenHeight = 240;
// unsigned long WidgetBase::sharedTime = 0;
functionLoadScreen_t WidgetBase::loadScreen = nullptr;
#if defined(DFK_SD)
fs::SDFS *WidgetBase::mySD = nullptr;
#endif

/**
 * @brief Lightens a color by a given factor.
 * @param color The color to lighten.
 * @param factor The factor to lighten the color by. (0.0 to 5.0)
 * @return The lightened color.
 */
/*uint16_t WidgetBase::lightenColor565(unsigned short color, float factor)
{
    factor = constrain(factor, 0.0, 5.0);
    factor /= 20.0;

    // Extrair os componentes de cor
    unsigned short red   = (color >> 11) & 0x1F; // 5 bits para vermelho
    unsigned short green = (color >> 5) & 0x3F;  // 6 bits para verde
    unsigned short blue  = color & 0x1F;         // 5 bits para azul

    // Clarear os componentes de cor
    red   = _min(static_cast<unsigned short>(red * (1 + factor)), 0x1F);
    green = _min(static_cast<unsigned short>(green * (1 + factor)), 0x3F);
    blue  = _min(static_cast<unsigned short>(blue * (1 + factor)), 0x1F);

    // Recombinar os componentes em uma cor 565
    return (red << 11) | (green << 5) | blue;
}*/

/*uint16_t WidgetBase::lightenToWhite565(uint16_t color, float factor)
{
    // limita factor entre 0.0 (original) e 1.0 (branco puro)
    if (factor < 0.0f) factor = 0.0f;
    if (factor > 1.0f) factor = 1.0f;

    // extrair componentes
    uint8_t r = (color >> 11) & 0x1F; // 5 bits
    uint8_t g = (color >> 5) & 0x3F;  // 6 bits
    uint8_t b = color & 0x1F;         // 5 bits

    // máximos
    const uint8_t Rmax = 0x1F;
    const uint8_t Gmax = 0x3F;
    const uint8_t Bmax = 0x1F;

    // para clarear, aproximamos cada componente do máximo
    r = r + (uint8_t)((Rmax - r) * factor);
    g = g + (uint8_t)((Gmax - g) * factor);
    b = b + (uint8_t)((Bmax - b) * factor);

    // recombinar
    return (r << 11) | (g << 5) | b;
}*/

/**
 * @brief Blends two colors with a given factor.
 * @param color1 The first color to blend.
 * @param color2 The second color to blend.
 * @param factor The factor to blend the colors by.
 * @return The blended color.
 */
/*uint16_t WidgetBase::blendColors(uint16_t color1, uint16_t color2, float factor) {
    uint8_t r1, g1, b1;
    uint8_t r2, g2, b2;
    
    // Extrai as cores RGB de ambas as cores no formato RGB565
    WidgetBase::extract565toRGB(color1, r1, g1, b1);
    WidgetBase::extract565toRGB(color2, r2, g2, b2);
    
    // Calcula a cor intermediária com base no fator
    uint8_t r = r1 * (1.0 - factor) + r2 * factor;
    uint8_t g = g1 * (1.0 - factor) + g2 * factor;
    uint8_t b = b1 * (1.0 - factor) + b2 * factor;
    
    // Converte a cor intermediária para o formato RGB565 e retorna
    return convertToRGB565(r, g, b);
}
*/
/**
 * @brief Extracts the RGB components from a color in RGB565 format.
 * @param color565 The color in RGB565 format.
 * @param r Reference to store the red component.
 * @param g Reference to store the green component.
 * @param b Reference to store the blue component.
 */
void WidgetBase::extract565toRGB(uint16_t color565, uint8_t &r, uint8_t &g, uint8_t &b) {
    r = (color565 >> 11) & 0x1F; // Extrai o componente vermelho (5 bits)
    g = (color565 >> 5) & 0x3F;  // Extrai o componente verde (6 bits)
    b = color565 & 0x1F;         // Extrai o componente azul (5 bits)
    
    // Normaliza para 8 bits
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
}

/**
 * @brief Converts RGB components to RGB565 format.
 * @param r The red component.
 * @param g The green component.
 * @param b The blue component.
 * @return The color in RGB565 format.
 */
/*uint16_t WidgetBase::convertToRGB565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}*/


/**
 * @brief Darkens a color by a given factor.
 * @param color The color to darken.
 * @param factor The factor to darken the color by.
 * @return The darkened color.
 */
/*uint16_t WidgetBase::darkenColor565(unsigned short color, float factor)
{
    factor = constrain(factor, 0.0, 5.0);
    factor /= 20.0;
    // Extrair os componentes de cor
    unsigned short red   = (color >> 11) & 0x1F; // 5 bits para vermelho
    unsigned short green = (color >> 5) & 0x3F;  // 6 bits para verde
    unsigned short blue  = color & 0x1F;         // 5 bits para azul

    // Escurecer os componentes de cor
    red   = _max(static_cast<unsigned short>(red * (1 - factor)), 0x0);
    green = _max(static_cast<unsigned short>(green * (1 - factor)), 0x0);
    blue  = _max(static_cast<unsigned short>(blue * (1 - factor)), 0x0);

    // Recombinar os componentes em uma cor 565
    return (red << 11) | (green << 5) | blue;
}*/

/**
 * @brief Constructor for the WidgetBase class.
 * @param _x X position of the widget.
 * @param _y Y position of the widget.
 * @param _screen Screen number.
 */
WidgetBase::WidgetBase(uint16_t _x, uint16_t _y, uint8_t _screen) 
    : m_visible(true)
    , m_xPos(_x)
    , m_yPos(_y)
    , m_screen(_screen)
    , m_loaded(false)
    , m_enabled(true)
    , m_initialized(false)
    , m_shouldRedraw(false)
    , m_isPressed(false)
    , m_locked(false)
    , m_myTime(0)
    , m_callback(nullptr)
{
    ESP_LOGD(TAG, "WidgetBase created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destructor for the WidgetBase class.
 */
WidgetBase::~WidgetBase()
{
    ESP_LOGD(TAG, "WidgetBase destroyed at (%d, %d)", m_xPos, m_yPos);
    
    // Limpa o callback se existir
    if (m_callback != nullptr) {
        m_callback = nullptr;
    }
    
    // Reseta o estado do widget
    m_loaded = false;
    m_initialized = false;
    m_shouldRedraw = false;
    m_isPressed = false;
}

// Pure virtual methods - must be implemented by derived classes
// detectTouch, getCallbackFunc, show, hide, setup, forceUpdate, redraw

bool WidgetBase::showingMyScreen()
{
    return WidgetBase::currentScreen == m_screen;
}

/**
 * @brief Sets the pressed state of the widget
 * @param pressed True if widget is being pressed, false otherwise
 */
void WidgetBase::setPressed(bool pressed) {
    m_isPressed = pressed;
    if (pressed) {
        ESP_LOGD(TAG, "Widget marked as pressed at (%d, %d)", m_xPos, m_yPos);
    } else {
        ESP_LOGD(TAG, "Widget marked as not pressed at (%d, %d)", m_xPos, m_yPos);
    }
}

/**
 * @brief Default implementation of onRelease
 * @details This method is called when touch is released or leaves the widget area.
 *          Default implementation sets m_isPressed to false.
 *          Derived widgets should call WidgetBase::onRelease() first if overriding,
 *          then set m_shouldRedraw = true if visual update is needed.
 */
void WidgetBase::onRelease() {
    // Default implementation: reset pressed state
    m_isPressed = false;
    ESP_LOGD(TAG, "Widget released at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Adds a callback to the callback queue
 * @param callback Callback to be added
 * @param origin Origin of the callback
 */
void WidgetBase::addCallback(functionCB_t callback, CallbackOrigin origin){
    static uint32_t droppedCallbacks = 0;

    if (callback == nullptr) {
        ESP_LOGE(TAG, "Callback is nullptr. Not adding to queue callback.");
        return;
    }

    if (WidgetBase::xFilaCallback == nullptr) {
        ESP_LOGE(TAG, "Can't add callback. Queue is not created");
        return;
    }

    if (xQueueSend(WidgetBase::xFilaCallback, &callback, 0) == pdPASS) {
        ESP_LOGD(TAG, "Callback added to queue. Origin: %d", (int)origin);
        return;
    }

    droppedCallbacks++;
    if ((droppedCallbacks % 32) == 0) {
        ESP_LOGW(TAG, "Callback queue full, dropped %u", static_cast<unsigned>(droppedCallbacks));
    }
}


#if defined(USING_GRAPHIC_LIB)
int WidgetBase::getLastLettersForSpace(const char* textoCompleto, uint16_t width, uint16_t height,
                                       char* out, size_t outSize)
{
  if (!out || outSize == 0) return -1;
  out[0] = '\0';

  if (!objTFT || !textoCompleto) return 0;
  if (*textoCompleto == '\0') return 0;

  // Calcula o comprimento da string
  size_t len = 0;
  while (textoCompleto[len] != '\0' && len < outSize - 1) {
    len++;
  }

  int16_t x1, y1;
  uint16_t w, h;

  // Testa do comprimento total até 1, pegando do final
  for (size_t testLen = len; testLen > 0; --testLen) {
    size_t startPos = len - testLen;

    if (testLen >= outSize) continue;

    for (size_t i = 0; i < testLen; ++i) {
      out[i] = textoCompleto[startPos + i];
    }
    out[testLen] = '\0';

    objTFT->getTextBounds(out, 0, 0, &x1, &y1, &w, &h);

    if (w <= width && h <= height) {
      return (int)testLen;
    }
  }

  // Nem um caractere coube
  out[0] = '\0';
  return 0;
}

int WidgetBase::getFirstLettersForSpace(const char* textoCompleto, uint16_t width, uint16_t height,
                                        char* out, size_t outSize)
{
  if (!out || outSize == 0) return -1;
  out[0] = '\0';

  if (!objTFT || !textoCompleto) return 0;
  if (*textoCompleto == '\0') return 0;

  // Calcula o comprimento da string
  size_t len = 0;
  while (textoCompleto[len] != '\0' && len < outSize - 1) {
    len++;
  }

  int16_t x1, y1;
  uint16_t w, h;

  // Testa do comprimento total até 1, pegando do início
  for (size_t testLen = len; testLen > 0; --testLen) {
    if (testLen >= outSize) continue;

    for (size_t i = 0; i < testLen; ++i) {
      out[i] = textoCompleto[i];
    }
    out[testLen] = '\0';

    objTFT->getTextBounds(out, 0, 0, &x1, &y1, &w, &h);

    if (w <= width && h <= height) {
      return (int)testLen;
    }
  }

  // Nem um caractere coube
  out[0] = '\0';
  return 0;
}

/**
 * @brief Retrieves the best fitting font for a given area.
 * @param text Text to be displayed.
 * @param width Available width for text.
 * @param height Available height for text.
 * @param fonts Array of available fonts.
 * @param fontCount Number of fonts in the array.
 * @return Pointer to the best fitting font, or nullptr if none fit.
 */
const GFXfont* WidgetBase::getBestFontForArea(const char* text, uint16_t width, uint16_t height, const GFXfont* const fonts[], size_t fontCount)
{
  if (!objTFT || !fonts || fontCount == 0) {
    return nullptr;
  }

  // Se texto for nulo, considere como string vazia (largura/altura 0) -> primeira fonte serve.
  if (!text) {
    return fonts[0];
  }

  // Se string vazia, cabe em qualquer fonte -> retorne a primeira para evitar medições desnecessárias.
  if (*text == '\0') {
    return fonts[0];
  }

  // Varíaveis reutilizadas para evitar re-declarações em loop.
  int16_t x1, y1;
  uint16_t w, h;

  // Tente cada fonte, na ordem.
  for (size_t i = 0; i < fontCount; ++i) {
    const GFXfont* f = fonts[i];
    if (!f) {
      continue; // ignora entradas nulas
    }

    objTFT->setFont(f);
    // Posição de referência pode ser 0,0 pois o que importa é w e h
    objTFT->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    // Verifica se cabe.
    if (w <= width && h <= height) {
      return f; // primeira que couber
    }
  }

  return nullptr; // nenhuma coube
}

/**
 * @brief Retrieves the best fitting Roboto Bold font for a given area.
 * @param availableWidth Available width for text.
 * @param availableHeight Available height for text.
 * @param texto Text string to be displayed.
 * @return Pointer to the best fitting font.
 */
const GFXfont *WidgetBase::getBestRobotoBold(uint16_t availableWidth, uint16_t availableHeight, const char* texto)
{
    const uint8_t amountFonts = 18;
    // Array de fontes disponíveis, ordenadas do maior para o menor
    static const GFXfont* const fontes[amountFonts] = {
        &RobotoBold50pt7b,
        &RobotoBold40pt7b,
        &RobotoBold30pt7b,
        &RobotoBold20pt7b,
        &RobotoBold15pt7b,
        &RobotoBold14pt7b,
        &RobotoBold13pt7b,
        &RobotoBold12pt7b,
        &RobotoBold11pt7b,
        &RobotoBold10pt7b,
        &RobotoBold9pt7b,
        &RobotoBold8pt7b,
        &RobotoBold7pt7b,
        &RobotoBold6pt7b,
        &RobotoBold5pt7b,
        &RobotoBold4pt7b,
        &RobotoBold3pt7b,
        &RobotoBold2pt7b
    };

    return this->getBestFontForArea( texto, availableWidth, availableHeight, fontes, amountFonts);
}

/**
 * @brief Updates the font according to the specified font type.
 * @param _f Font type to set (NORMAL, BOLD).
 */
void WidgetBase::updateFont(FontType _f)
{
    WidgetBase::objTFT->setTextSize(1);
    switch (_f)
    {
    case FontType::NORMAL:
    {
#if defined(USING_GRAPHIC_LIB)
        WidgetBase::objTFT->setFont(WidgetBase::fontNormal);
#endif
    }
    break;
    case FontType::BOLD:
    { 
#if defined(USING_GRAPHIC_LIB)
        WidgetBase::objTFT->setFont(WidgetBase::fontBold);
#endif
    }
    break;

    default:
    {
#if defined(USING_GRAPHIC_LIB)
        WidgetBase::setFontNull();
#endif
    }
    break;
    }
}



/**
 * @brief Prints text on the screen.
 * @param _texto Text to print.
 * @param _x X position of the text.
 * @param _y Y position of the text.
 * @param _datum Datum of the text.
 */
void WidgetBase::printText(const char* _texto, uint16_t _x, uint16_t _y, uint8_t _datum){
    //int ox = _x, oy = _y;
    WidgetBase::recalculateTextPosition(_texto, &_x, &_y, _datum);

    TextBound_t areaAux;
    objTFT->getTextBounds(_texto, _x, _y, &areaAux.x, &areaAux.y, &areaAux.width, &areaAux.height);

    objTFT->setCursor(_x, _y);
    objTFT->print(_texto);

    #ifdef DEBUG_TEXT_BOUND
        objTFT->drawRect(areaAux.x, areaAux.y, areaAux.width, areaAux.height, CFK_FUCHSIA);
        objTFT->fillCircle(_x, _y, 3, CFK_RED);
    #endif

}

/**
 * @brief Prints text on the screen.
 * @param _texto Text to print.
 * @param _x X position of the text.
 * @param _y Y position of the text.
 * @param _datum Datum of the text.
 * @param lastTextBoud Reference to store the last text bounds.
 * @param _colorPadding Color of the padding.
 */
void WidgetBase::printText(const char* _texto, uint16_t _x, uint16_t _y, uint8_t _datum, TextBound_t &lastTextBoud, uint16_t _colorPadding){
    objTFT->setTextWrap(false);
    objTFT->fillRect(lastTextBoud.x, lastTextBoud.y, lastTextBoud.width, lastTextBoud.height, _colorPadding);
    //int ox = _x, oy = _y;
    WidgetBase::recalculateTextPosition(_texto, &_x, &_y, _datum);

    TextBound_t areaAux;
    objTFT->getTextBounds(_texto, _x, _y, &areaAux.x, &areaAux.y, &areaAux.width, &areaAux.height);

    lastTextBoud.x = areaAux.x;
    lastTextBoud.y = areaAux.y;
    lastTextBoud.width = areaAux.width;
    lastTextBoud.height = areaAux.height;

    objTFT->setCursor(_x, _y);
    objTFT->print(_texto);

    #ifdef DEBUG_TEXT_BOUND
        objTFT->drawRect(lastTextBoud.x, lastTextBoud.y, lastTextBoud.width, lastTextBoud.height, CFK_DEEPPINK);
        objTFT->fillCircle(_x, _y, 3, CFK_RED);
        //log_d("Wrote on screen: %s at %d, %d", _texto, _x, _y);
    #endif

}

/**
 * @brief Recalculates the position of text on the screen.
 * @param _texto Text to recalculate position for.
 * @param _x Pointer to the X position of the text.
 * @param _y Pointer to the Y position of the text.
 * @param _datum Datum of the text.
 */
void WidgetBase::recalculateTextPosition(const char *_texto, uint16_t *_x, uint16_t *_y, uint8_t _datum)
{
    //static const int16_t OFFSET_PADDING_Y = 6;
    
    // Obtém as dimensões do texto
    /*TextBound_t area;
    objTFT->getTextBounds(_texto, 0, 0, &area.x, &area.y, &area.width, &area.height);

    // Calcula os offsets baseados no datum
    int16_t xOffset = 0;
    int16_t yOffset = 0;

    // Calcula o offset horizontal
    switch (_datum) {
        case TC_DATUM:
        case MC_DATUM:
        case BC_DATUM:
            xOffset = -(area.width / 2);
            break;
        case TR_DATUM:
        case MR_DATUM:
        case BR_DATUM:
            xOffset = -area.width;
            break;
        default: // TL_DATUM, ML_DATUM, BL_DATUM
            xOffset = 0;
            break;
    }

    // Calcula o offset vertical
    switch (_datum) {
        case TL_DATUM:
        case TC_DATUM:
        case TR_DATUM:
            yOffset = area.height;
            break;
        case ML_DATUM:
        case MC_DATUM:
        case MR_DATUM:
            yOffset = area.height / 2;
            break;
        case BL_DATUM:
        case BC_DATUM:
        case BR_DATUM:
            yOffset = -OFFSET_PADDING_Y;
            break;
        default:
            yOffset = 0;
            break;
    }

    // Aplica os offsets
    *_x += xOffset;
    *_y += yOffset;*/
    // Pega a área real ocupada pelo texto
    TextBound_t a;
    objTFT->getTextBounds(_texto, 0, 0, &a.x, &a.y, &a.width, &a.height);

    int16_t xo = *_x; 
    int16_t yo = *_y;

    // Ajuste horizontal
    switch (_datum) {
        case TL_DATUM: case ML_DATUM: case BL_DATUM: case L_BASELINE:
            xo = *_x - a.x;
            break;

        case TC_DATUM: case MC_DATUM: case BC_DATUM: case C_BASELINE:
            xo = *_x - (a.x + a.width / 2);
            break;

        case TR_DATUM: case MR_DATUM: case BR_DATUM: case R_BASELINE:
            xo = *_x - (a.x + a.width);
            break;
    }

    // Ajuste vertical
    switch (_datum) {
        case TL_DATUM: case TC_DATUM: case TR_DATUM:
            yo = *_y - a.y;
            break;

        case ML_DATUM: case MC_DATUM: case MR_DATUM:
            yo = *_y - (a.y + a.height / 2);
            break;

        case BL_DATUM: case BC_DATUM: case BR_DATUM:
            yo = *_y - (a.y + a.height);
            break;

        case L_BASELINE: case C_BASELINE: case R_BASELINE:
            // baseline já está em a.y
            yo = *_y - 0;  
            break;
    }

    *_x = xo;
    *_y = yo;
}

/**
 * @brief Gets the text bounds for a given string.
 * @param str The string to get the bounds for.
 * @param x The X position of the text.
 * @param y The Y position of the text.
 * @return The text bounds.
 */
TextBound_t WidgetBase::getTextBounds(const char *str, int16_t x, int16_t y)
{
    TextBound_t retorno;
    WidgetBase::objTFT->getTextBounds(str, x, y, &retorno.x, &retorno.y, &retorno.width, &retorno.height);
    return retorno;
}

/**
 * @brief Draws a rotated image on the screen.
 * @param image The image to draw.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param angle The angle to rotate the image.
 * @param pivotX The X position of the pivot.
 * @param pivotY The Y position of the pivot.
 * @param drawX The X position to draw the image.
 * @param drawY The Y position to draw the image.
 */
void WidgetBase::drawRotatedImageOptimized(uint16_t *image, int16_t width, int16_t height, float angle, int16_t pivotX, int16_t pivotY, int16_t drawX, int16_t drawY)
{
    log_d("Drawing image with  %i x %i and angle %f at pos %i x %i", width, height, angle, drawX, drawY);
    // Converte o ângulo para radianos
    float radians = angle * DEG_TO_RAD;

    // Pré-calcula o seno e o cosseno do ângulo
    float cosA = cos(radians);
    float sinA = sin(radians);

    // Pré-calcula limites de varredura
    int16_t minX = -pivotX;
    int16_t maxX = width - pivotX;
    int16_t minY = -pivotY;
    int16_t maxY = height - pivotY;

    // Desenha a imagem rotacionada
    objTFT->startWrite();  // Começa a transação de escrita para otimizar a comunicação com o display
    for (int16_t y = minY; y < maxY; y++) {
        for (int16_t x = minX; x < maxX; x++) {
            // Calcula as coordenadas rotacionadas
            int16_t rotatedX = pivotX + (x * cosA - y * sinA);
            int16_t rotatedY = pivotY + (x * sinA + y * cosA);

            // Verifica se o pixel rotacionado está dentro dos limites da tela
            if (rotatedX >= 0 && rotatedX < objTFT->width() && rotatedY >= 0 && rotatedY < objTFT->height()) {
                // Desenha o pixel rotacionado no display
                uint16_t color = image[(y + pivotY) * width + (x + pivotX)]; // Obtém a cor original no formato RGB565
                objTFT->writePixel(drawX + rotatedX, drawY + rotatedY, color); // Desenha na nova posição
            }
        }
    }
    objTFT->endWrite();  // Termina a transação de escrita
}

/**
 * @brief Shows the origin of the widget.
 * @param color The color of the origin.
 */
void WidgetBase::showOrigin(uint16_t color){
#if defined(HELPERS)
    uint8_t tam = 5;
    objTFT->drawFastVLine(m_xPos, m_yPos-tam, 2*tam, color);
    objTFT->drawFastHLine(m_xPos-tam, m_yPos, 2*tam, color);
    objTFT->fillCircle(m_xPos, m_yPos, 2, color);
#endif
}

// New state management methods
bool WidgetBase::isInitialized() const {
    return m_initialized;
}

bool WidgetBase::isEnabled() const {
    return m_enabled;
}

void WidgetBase::setEnabled(bool enabled) {
    m_enabled = enabled;
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "Widget %s at (%d, %d)", enabled ? "enabled" : "disabled", m_xPos, m_yPos);
}

bool WidgetBase::isValidState() const {
    return m_initialized && m_loaded && (objTFT != nullptr);
}

bool WidgetBase::validateParameters() const {
    return (m_xPos < screenWidth) && (m_yPos < screenHeight);// && (m_screen >= 0);
}

void WidgetBase::lock() {
    m_locked = true;
    ESP_LOGD(TAG, "Widget locked at (%d, %d)", m_xPos, m_yPos);
}

void WidgetBase::unlock() {
    m_locked = false;
    ESP_LOGD(TAG, "Widget unlocked at (%d, %d)", m_xPos, m_yPos);
}

bool WidgetBase::isLocked() const {
    return m_locked;
}

// Pure virtual methods - must be implemented by derived classes
// setup, forceUpdate, redraw
#endif
