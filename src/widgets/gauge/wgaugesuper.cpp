#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_GAUGE)
#include "wgaugesuper.h"
#include <new>
#include "../../extras/dwx_safe.h"
#include <esp_log.h>

const char* GaugeSuper::TAG = "GaugeSuper";

/**
 * @brief Construtor da classe GaugeSuper.
 * @param _x Coordenada X da posição central do gauge na tela.
 * @param _y Coordenada Y da posição central do gauge na tela.
 * @param _screen Identificador da tela onde o gauge será exibido.
 * @details Inicializa o gauge super com valores padrão: largura 0, faixa 0-100,
 *          cores padrão, sem título, sem intervalos. O gauge deve ser configurado
 *          com setup() antes de ser exibido. Gerencia memória dinamicamente para
 *          arrays de intervalos, cores e título.
 */
GaugeSuper::GaugeSuper(uint16_t _x, uint16_t _y, uint8_t _screen) 
    : WidgetBase(_x, _y, _screen), 
      // Internal state (in order of declaration)
      m_ltx(0), m_lastPointNeedle(), m_stripColor(CFK_WHITE), m_indexCurrentStrip(0), 
      m_divisor(1), m_isFirstDraw(true),
      // Dynamic arrays
      m_intervals(nullptr), m_colors(nullptr), m_title(nullptr),
      // Memory management flags
      m_intervalsAllocated(false), m_colorsAllocated(false), m_titleAllocated(false),
      // Calculated dimensions
      m_height(0), m_radius(0), m_currentValue(0), m_lastValue(0),
      // Drawing parameters
      m_stripWeight(16), m_maxAngle(40), m_offsetYAgulha(40), m_rotation(90), 
      m_distanceAgulhaArco(2), m_borderSize(5), m_availableWidth(0), m_availableHeight(0)
{
  // Initialize m_config with default values
  m_config = {.title = nullptr, .intervals = nullptr, .colors = nullptr,
              #if defined(USING_GRAPHIC_LIB)
              .fontFamily = nullptr,
              #endif
              .minValue = 0, .maxValue = 100, .width = 0, .height = 0, .borderColor = CFK_BLACK,
              .textColor = CFK_BLACK, .backgroundColor = CFK_WHITE, .titleColor = CFK_NAVY,
              .needleColor = CFK_RED, .markersColor = CFK_BLACK, .amountIntervals = 0, .showLabels = false};
  
  // Dynamic arrays already initialized in member initializer list
  
  ESP_LOGD(TAG, "GaugeSuper created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe GaugeSuper.
 * @details Registra o evento no log do ESP32, limpa toda a memória alocada dinamicamente
 *          usando cleanupMemory() e limpa o ponteiro da função callback.
 *          Todos os recursos são liberados automaticamente pela herança de @ref WidgetBase.
 */
GaugeSuper::~GaugeSuper()
{
  ESP_LOGD(TAG, "GaugeSuper destroyed at (%d, %d)", m_xPos, m_yPos);
  
  // Use centralized cleanup method
  cleanupMemory();
  
  // Clear callback
  if (m_callback != nullptr) {
    m_callback = nullptr;
  }
}

/**
 * @brief Detecta se o GaugeSuper foi tocado pelo usuário.
 * @param _xTouch Ponteiro para a coordenada X do toque na tela.
 * @param _yTouch Ponteiro para a coordenada Y do toque na tela.
 * @return Sempre retorna False, pois GaugeSuper não processa eventos de toque.
 * @details Este widget é apenas visual e não responde a interações de toque.
 *          Os parâmetros são marcados como UNUSED para evitar avisos do compilador.
 */
bool GaugeSuper::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  WidgetStateGuard widgetStateGuard;
  // GaugeSuper doesn't handle touch events, it's a display-only widget
  UNUSED(_xTouch);
  UNUSED(_yTouch);
  return false;
}

/**
 * @brief Recupera a função callback associada ao GaugeSuper.
 * @return Ponteiro para a função callback, ou nullptr se nenhuma foi definida.
 * @details A função callback é executada quando o estado do gauge muda.
 *          Para mais informações sobre callbacks, consulte @ref WidgetBase.
 */
functionCB_t GaugeSuper::getCallbackFunc()
{
  WidgetStateGuard widgetStateGuard;
  return m_callback;
}

/**
 * @brief Inicializa o widget GaugeSuper com cálculos padrão e restrições.
 * @details Calcula dimensões do gauge, limites de texto e define valores iniciais.
 *          Restringe o ângulo máximo entre 22 e 80 graus. Calcula o raio sugerido
 *          baseado na largura configurada e ajusta o offset da agulha automaticamente.
 *          Este método é chamado internamente por setup() após a configuração.
 */
void GaugeSuper::start()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  #if defined(USING_GRAPHIC_LIB)
  if (m_config.maxValue < m_config.minValue)
  {
    int temp = m_config.minValue;
    m_config.minValue = m_config.maxValue;
    m_config.maxValue = temp;
  }

  WidgetBase::objTFT->setFont(m_usedFont);

  // Calculate text dimensions for values
  char strMin[12], strMax[12];// Maximum of 12 characters for values
  snprintf(strMin, sizeof(strMin), "%d", m_config.minValue);
  snprintf(strMax, sizeof(strMax), "%d", m_config.maxValue);
  TextBound_t dimensionMin = getTextBounds(strMin, m_xPos, m_yPos);// Get text dimensions
  TextBound_t dimensionMax = getTextBounds(strMax, m_xPos, m_yPos);// Get text dimensions
  m_textBoundForValue = dimensionMax.width > dimensionMin.width ? dimensionMax : dimensionMin;
  m_textBoundForValue.width += getTextBounds("0", m_xPos, m_yPos).width;

  // Configure the title
  // Title visibility is determined by m_config.title != nullptr
  
  m_currentValue = m_config.minValue;// Define initial value
  m_lastValue = m_config.maxValue;// Define initial value

  
  //TextBound_t t;
  //t.height = 20;
  //t.width = m_showLabels ? 50 : 10;

  // https://pt.wikipedia.org/wiki/Segmento_circular

  if(!m_config.showLabels){
    m_textBoundForValue.width = 5;
    m_textBoundForValue.height = 5;
  }
  

  // Calculate gauge radius and angle
  int corda = (m_config.width - 2 * m_textBoundForValue.width) * 0.9;// Gauge width minus value text multiplied by 0.9 to have space for text
  int aberturaArcoTotal = 2 * m_maxAngle;// Calculate total arc angle
  int raioSugerido = corda / (2.0 * fastSin(aberturaArcoTotal / 2.0));// Calculate suggested radius
  int altura = raioSugerido * (1 - fastCos(aberturaArcoTotal / 2.0));
  m_radius = raioSugerido;
  ESP_LOGD(TAG, "Gauge radius %i\tsegment %i", m_radius, altura);
  UNUSED(altura);

  //m_offsetYAgulha = (40 + m_textBoundForValue.height + m_borderSize);
  int seno = fastSin((90 - m_maxAngle)) * m_radius;
  m_offsetYAgulha = seno - (m_borderSize + m_textBoundForValue.height * 2);
  ESP_LOGD(TAG, "Needle offset: %i", m_offsetYAgulha);
  m_rotation = (-(m_maxAngle + m_rotation));
  m_divisor = 1;

  // Configure gauge origin
  m_origem.x = m_xPos;
  m_origem.y = m_yPos + m_offsetYAgulha;

  // The 3 in borderSize is to have space for labels and top border
  // m_height = (m_radius - m_offsetYAgulha) + (3 * m_borderSize) + (m_textBoundForValue.height * 3);
  m_height = m_config.height;

  

  // Adjust maximum angle
  m_maxAngle = constrain(m_maxAngle, 22, 80);

  // Reset textbound value to draw the value
  m_textBoundForValue.x = 0;
  m_textBoundForValue.y = 0;
  m_textBoundForValue.width = 0;
  m_textBoundForValue.height = 0;

  // Calculate available dimensions
  m_availableWidth = m_config.width - (2 * m_borderSize);
  m_availableHeight = m_height - (2 * m_borderSize);

  m_shouldRedraw = true;
  
  // Configuration is now directly accessible through m_config
  #endif
}

/**
 * @brief Desenha o fundo do widget GaugeSuper.
 * @details Renderiza o fundo do gauge incluindo faixas coloridas, marcadores e rótulos.
 *          Este método deve ser chamado uma vez para configurar o fundo antes
 *          de usar setValue() para atualizar o valor. Desenha bordas, fundo,
 *          faixas coloridas baseadas nos intervalos, marcadores graduados e
 *          rótulos opcionais. Apenas desenha se o widget está na tela atual
 *          e adequadamente carregado.
 */
void GaugeSuper::drawBackground()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  #if defined(USING_GRAPHIC_LIB)
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  
  // Validate arrays before drawing
  if (m_config.amountIntervals > 0) {
    if (m_intervals == nullptr || m_colors == nullptr) {
      ESP_LOGE(TAG, "Cannot draw background: intervals or colors arrays are null");
      return;
    }
  }

  uint16_t baseBorder = WidgetBase::lightMode ? CFK_BLACK : CFK_WHITE;

  // updateFont(FontType::NORMAL);
  WidgetBase::objTFT->setFont(m_usedFont);

  ESP_LOGD(TAG, "Draw background GaugeSuper");

  m_indexCurrentStrip = 0;  // Index of first color to paint the strip background
  m_stripColor = CFK_WHITE; // the beginning of the strip is white
  m_ltx = 0;                // last x position of needle base
  m_lastPointNeedle.x = m_origem.x;
  m_lastPointNeedle.y = m_origem.y; // needle positions

  for (auto i = 0; i < m_borderSize; ++i)
  {
    WidgetBase::objTFT->drawRect((m_xPos - (m_config.width / 2)) + i, (m_yPos - (m_height)) + i, m_config.width - (2 * i), m_height - (2 * i), m_config.borderColor);
  }

  WidgetBase::objTFT->fillRect(m_xPos - (m_availableWidth / 2), m_yPos - (m_availableHeight + m_borderSize), m_availableWidth, m_availableHeight, m_config.backgroundColor);
  WidgetBase::objTFT->drawRect(m_xPos - (m_availableWidth / 2), m_yPos - (m_availableHeight + m_borderSize), m_availableWidth, m_availableHeight, baseBorder);

  // Draw colored strip with intervals less than 5 to be more precise
  for (int i = 0; i <= (2 * m_maxAngle); i += 1)
  {
    // Apply rotation
    int angulo = i + m_rotation;

    // Larger tick size
    int tl = 15;

    // Coordinates to draw the tick
    float sx = fastCos(angulo);
    float sy = fastSin(angulo);

    uint16_t x0 = sx * (m_radius + tl) + m_origem.x;
    uint16_t y0 = sy * (m_radius + tl) + m_origem.y;
    uint16_t x1 = sx * m_radius + m_origem.x;
    uint16_t y1 = sy * m_radius + m_origem.y;

    // Coordinates of next tick to fill space with desired color
    float sx2 = fastCos(angulo + 1);
    float sy2 = fastSin(angulo + 1);
    int x2 = sx2 * (m_radius + tl) + m_origem.x;
    int y2 = sy2 * (m_radius + tl) + m_origem.y;
    int x3 = sx2 * m_radius + m_origem.x;
    int y3 = sy2 * m_radius + m_origem.y;

    int vFaixa = dwx_map_clamped(i, 0, (2 * m_maxAngle), m_config.minValue, m_config.maxValue); // Transform the for loop from -50 to 50 into value between min and max to paint
    if (m_indexCurrentStrip < m_config.amountIntervals && vFaixa >= m_intervals[m_indexCurrentStrip])
    {
      m_stripColor = m_colors[m_indexCurrentStrip];
      m_indexCurrentStrip++;
    }
    if (i < 2 * m_maxAngle)
    {
      WidgetBase::objTFT->fillTriangle(x0, y0, x1, y1, x2, y2, m_stripColor);
      WidgetBase::objTFT->fillTriangle(x1, y1, x2, y2, x3, y3, m_stripColor);
    }
  }
  // End of colored arc drawing

  // Calculate and draw the multiplier
  WidgetBase::objTFT->setTextColor(m_config.textColor); // Text color

  if (isLabelsVisible())
  {

    // Reset values to draw texts
    m_indexCurrentStrip = 0; // Index of first interval to write the value

    // Loop to draw interval texts
    for (int i = 0; i <= (2 * m_maxAngle); i += 1)
    {
      int angulo = i + m_rotation;

      // Larger tick size
      int tl = 15;

      // Coordinates to draw the tick
      float sx = fastCos(angulo);
      float sy = fastSin(angulo);
      uint16_t x0 = sx * (m_radius + tl) + m_origem.x;
      uint16_t y0 = sy * (m_radius + tl) + m_origem.y;

      int vFaixa = dwx_map_clamped(i, 0, 2 * m_maxAngle, m_config.minValue, m_config.maxValue);
      if (m_indexCurrentStrip < m_config.amountIntervals && vFaixa >= m_intervals[m_indexCurrentStrip])
      {
        int aX = sx * (m_radius + tl + 2) + m_origem.x;
        int aY = sy * (m_radius + tl + 2) + m_origem.y;
        uint8_t alinhamento = TL_DATUM;
        if (i == m_maxAngle)
        {
          alinhamento = BC_DATUM;
        }
        else if (i < m_maxAngle)
        {
          alinhamento = BR_DATUM;
          aX += 5;
        }
        else
        {
          alinhamento = BL_DATUM;
        }
        char char_arr[20];
        snprintf(char_arr, sizeof(char_arr), "%d", m_intervals[m_indexCurrentStrip] / m_divisor);
        printText(char_arr, aX, aY, alinhamento);
        //printText(String(m_intervals[m_indexCurrentStrip] / m_divisor).c_str(), aX, aY, alinhamento);
        ESP_LOGD(TAG, "Writing \"%d\" at %d, %d", m_intervals[m_indexCurrentStrip] / m_divisor, aX, aY);
        m_indexCurrentStrip++;
      }
      if (i == 2 * m_maxAngle)
      {
        char maximumText[12];
        snprintf(maximumText, sizeof(maximumText), "%d", m_config.maxValue / m_divisor);
        printText(maximumText, x0, y0, BL_DATUM);

        ESP_LOGD(TAG, "Writing %d at %d, %d", m_config.maxValue / m_divisor, x0, y0);
      }
    }
  }

  // Draw ticks every 5 degrees
  for (int i = 0; i <= 2 * m_maxAngle; i += 5)
  {
    int angulo = i + m_rotation;
    // Larger tick size
    int tl = 15;

    // Coordinates to draw the tick
    float sx = fastCos(angulo);
    float sy = fastSin(angulo);
    uint16_t x0 = sx * (m_radius + tl) + m_origem.x;
    uint16_t y0 = sy * (m_radius + tl) + m_origem.y;
    uint16_t x1 = sx * m_radius + m_origem.x;
    uint16_t y1 = sy * m_radius + m_origem.y;

    // Smaller tick size
    if (i % 25 != 0)
      tl = 8;

    // Recalculate coordinates if tick changes size, in case angle is not multiple of 25
    x0 = sx * (m_radius + tl) + m_origem.x;
    y0 = sy * (m_radius + tl) + m_origem.y;
    x1 = sx * m_radius + m_origem.x;
    y1 = sy * m_radius + m_origem.y;

    // Draw the tick
    WidgetBase::objTFT->drawLine(x0, y0, x1, y1, m_config.markersColor);

    // Calculate positions to draw base arc
    sx = fastCos(angulo + 5);
    sy = fastSin(angulo + 5);
    x0 = sx * m_radius + m_origem.x;
    y0 = sy * m_radius + m_origem.y;

    // Draw the arc, don't draw the last part
    if (i < 2 * m_maxAngle)
      WidgetBase::objTFT->drawLine(x0, y0, x1, y1, m_config.markersColor);
  }

  m_isFirstDraw = true;

  #endif

  // WidgetBase::objTFT->fillCircle(m_origem.x, m_origem.y, 2, CFK_RED);
  // WidgetBase::objTFT->drawCircle(m_origem.x, m_origem.y, m_radius, CFK_RED);
}

/**
 * @brief Define o valor atual do widget GaugeSuper.
 * @param newValue Novo valor a ser exibido no gauge.
 * @details Este método permite atualizar o valor do gauge:
 *          - Restringe o valor entre minValue e maxValue usando constrain()
 *          - Atualiza apenas se o valor mudou para evitar redesenhos desnecessários
 *          - Marca o widget para redesenho se o valor mudou
 *          - Registra o evento no log do ESP32
 *          O valor será mapeado proporcionalmente para o ângulo da agulha na próxima chamada de redraw().
 */
void GaugeSuper::setValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_INITIALIZED_VOID
  
  m_currentValue = constrain(newValue, m_config.minValue, m_config.maxValue);

  if (m_lastValue != m_currentValue)
  {
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "Set GaugeSuper value to %d", m_currentValue);
  }
  else
  {
    ESP_LOGD(TAG, "Set GaugeSuper value to %d but not updated", m_currentValue);
  }

  // redraw();
}

/**
 * @brief Redesenha o widget GaugeSuper na tela, atualizando a posição da agulha.
 * @details Este método é responsável por renderizar o gauge na tela:
 *          - Verifica todas as condições necessárias para o redesenho
 *          - Mapeia o valor atual para ângulos usando a função dwx_map_clamped()
 *          - Apaga a agulha anterior desenhando sobre ela com a cor de fundo
 *          - Desenha a nova agulha na posição calculada
 *          - Usa funções trigonométricas otimizadas (fastCos, fastSin, fastTan)
 *          - Desenha título e rótulos se habilitados
 *          - Aplica debounce para evitar redesenhos excessivos
 *          Apenas redesenha se o gauge está visível, inicializado, carregado, na tela atual
 *          e a flag m_shouldRedraw está configurada como true.
 */
void GaugeSuper::redraw()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  #if defined(USING_GRAPHIC_LIB)
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID
  CHECK_DEBOUNCE_REDRAW_VOID

  ESP_LOGD(TAG, "Redrawing GaugeSuper");
  // updateFont(FontType::NORMAL);
  WidgetBase::objTFT->setFont(m_usedFont);

  m_myTime = millis();
  m_lastValue = m_currentValue;

  // Draw here


  // Since the drawing is rotated -90 and drawing angles are -50 and 50.
  // int diff10 = (maxAngle + 10) - 90;

  int sdeg = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, 0, 2 * m_maxAngle); // Map input values min and max with extrapolation of 10 to angle with extrapolation of 10
  int angulo = sdeg + m_rotation;

  // Calculate needle components according to angle

  float sx = fastCos(angulo);
  float sy = fastSin(angulo);

  // Use tangent to calculate X position where needle should start since origin.y is below graph limit
  // The -90 is to simulate that total opening angle is rotated 90 degrees for correct tangent calculation (from -90 to 90)
  float tx = fastTan(angulo - 90);

  // Erase old needle
  if (!m_isFirstDraw)
  {
    WidgetBase::objTFT->drawLine(m_origem.x - 1 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x - 1, m_lastPointNeedle.y, m_config.backgroundColor); // -1 is to not draw on top of thin border line
    WidgetBase::objTFT->drawLine(m_origem.x + 0 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x + 0, m_lastPointNeedle.y, m_config.backgroundColor); // -1 is to not draw on top of thin border line
    WidgetBase::objTFT->drawLine(m_origem.x + 1 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x + 1, m_lastPointNeedle.y, m_config.backgroundColor); // -1 is to not draw on top of thin border line
  }

  WidgetBase::objTFT->setTextColor(m_config.textColor);
  if (isLabelsVisible())
  {
    // WidgetBase::printText()
    //uint16_t auxX = m_xPos - (m_availableWidth / 2) + 1;
    //uint16_t auxY = m_yPos - m_borderSize;
    //TextBound_t tb_value = getTextBounds(buf, auxX, auxY);
    //printText(buf, auxX, auxY, BL_DATUM, m_textBoundForValue, m_bkColor); //Mostrar valor do gauge
  }
  if (isTitleVisible())
  {
    // Redraw texts
    // updateFont(FontType::BOLD);
    WidgetBase::objTFT->setTextColor(m_config.titleColor);
    WidgetBase::objTFT->setFont(m_usedFont);

    //TextBound_t tb_title = getTextBounds(m_title, m_xPos, m_yPos - (m_borderSize * 2));
    printText(m_title, m_xPos, m_yPos - (m_borderSize * 2), BC_DATUM);
    updateFont(FontType::UNLOAD);
  }
  // store line values to erase later
  m_ltx = tx;
  m_lastPointNeedle.x = sx * (m_radius - m_distanceAgulhaArco) + m_origem.x; //-2 is the distance between needle end and arc
  m_lastPointNeedle.y = sy * (m_radius - m_distanceAgulhaArco) + m_origem.y;

  // Draw new line
  // Draw 3 lines to increase thickness

  WidgetBase::objTFT->drawLine(m_origem.x - 1 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x - 1, m_lastPointNeedle.y, m_config.needleColor);     // -1 is to not draw on top of thin border line
  WidgetBase::objTFT->drawLine(m_origem.x + 0 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x + 0, m_lastPointNeedle.y, m_config.needleColor); // -1 is to not draw on top of thin border line
  WidgetBase::objTFT->drawLine(m_origem.x + 1 + round(m_ltx * m_offsetYAgulha), m_origem.y - m_offsetYAgulha - m_borderSize - 2, m_lastPointNeedle.x + 1, m_lastPointNeedle.y, m_config.needleColor);     // -1 is to not draw on top of thin border line

  m_shouldRedraw = false;
  m_isFirstDraw = false;
  updateFont(FontType::UNLOAD);
  #endif
}

/**
 * @brief Força uma atualização imediata do GaugeSuper.
 * @details Define a flag m_shouldRedraw para true, forçando o redesenho do gauge
 *          no próximo ciclo de atualização da tela. Útil para garantir que mudanças
 *          no valor sejam visíveis imediatamente.
 */
void GaugeSuper::forceUpdate()
{
  WidgetStateGuard widgetStateGuard;
  m_shouldRedraw = true;
  ESP_LOGD(TAG, "GaugeSuper force update requested");
}


/**
 * @brief Configura o GaugeSuper com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref GaugeConfig contendo os parâmetros de configuração do gauge.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Valida a configuração usando validateConfig()
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia título para buffer interno com validação de tamanho
 *          - Copia arrays de intervalos e cores para buffers internos
 *          - Chama start() para cálculos e inicialização
 *          - Marca o widget como inicializado e carregado
 *          - O gauge não será exibido corretamente até que este método seja chamado
 */
void GaugeSuper::setup(const GaugeConfig& config)
{
  WidgetStateGuard widgetStateGuard;
  #if defined(USING_GRAPHIC_LIB)
  // Validate TFT object
  CHECK_TFT_VOID

  // Validate configuration first
  if (!validateConfig(config)) {
    ESP_LOGE(TAG, "Invalid configuration provided");
    return;
  }

  // Clean up any existing memory
  cleanupMemory();

  // Assign the configuration structure
  m_config = config;
  m_usedFont = config.fontFamily;
  
  // Copy title to internal buffer if provided
  if (config.title != nullptr) {
    size_t titleLen = strlen(config.title);
    
    // Validate title length (max 20 characters)
    if (titleLen > MAX_TITLE_LENGTH) {
      ESP_LOGW(TAG, "Title length (%d) exceeds maximum limit (%d). Truncating to %d characters", 
               titleLen, MAX_TITLE_LENGTH, MAX_TITLE_LENGTH);
      titleLen = MAX_TITLE_LENGTH;
    }
    
    // Clean up existing title before allocating new one
    if (m_titleAllocated && m_title != nullptr) {
      delete[] m_title;
      m_title = nullptr;
      m_titleAllocated = false;
    }
    
    m_title = new (std::nothrow) char[titleLen + 1];
    if (m_title != nullptr) {
      strncpy(m_title, config.title, titleLen);
      m_title[titleLen] = '\0'; // Ensure null termination
      m_titleAllocated = true;
    } else {
      ESP_LOGE(TAG, "Failed to allocate memory for title");
      return;
    }
  }
  
  // Copy intervals and colors to internal arrays if provided
  if (config.intervals != nullptr && config.colors != nullptr && config.amountIntervals > 0) {
    // Clamp amountIntervals to MAX_SERIES limit
    uint8_t clampedAmount = (config.amountIntervals > MAX_SERIES) ? MAX_SERIES : config.amountIntervals;
    
    // Clean up existing arrays before allocating new ones
    if (m_intervalsAllocated && m_intervals != nullptr) {
      delete[] m_intervals;
      m_intervals = nullptr;
      m_intervalsAllocated = false;
    }
    if (m_colorsAllocated && m_colors != nullptr) {
      delete[] m_colors;
      m_colors = nullptr;
      m_colorsAllocated = false;
    }
    
    // Allocate new memory
    m_intervals = new (std::nothrow) int[clampedAmount];
    m_colors = new (std::nothrow) uint16_t[clampedAmount];
    
    if (m_intervals != nullptr && m_colors != nullptr) {
      // Copy data
      for (uint8_t i = 0; i < clampedAmount; i++) {
        m_intervals[i] = config.intervals[i];
        m_colors[i] = config.colors[i];
      }
      m_intervalsAllocated = true;
      m_colorsAllocated = true;
    } else {
      ESP_LOGE(TAG, "Failed to allocate memory for intervals/colors");
      // Clean up partial allocations
      if (m_intervals != nullptr) {
        delete[] m_intervals;
        m_intervals = nullptr;
      }
      if (m_colors != nullptr) {
        delete[] m_colors;
        m_colors = nullptr;
      }
      cleanupMemory();
      return;
    }
  }
  // Initialize gauge
  start();

  m_loaded = true;
  m_initialized = true;
  
  ESP_LOGD(TAG, "GaugeSuper setup completed at (%d, %d)", m_xPos, m_yPos);
  #endif
}

/**
 * @brief Torna o gauge super visível na tela.
 * @details Define m_visible como true e marca para redesenho. O gauge será
 *          desenhado na próxima chamada de redraw() se estiver na tela atual.
 */
void GaugeSuper::show()
{
  WidgetStateGuard widgetStateGuard;
    m_visible = true;
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "GaugeSuper shown at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Oculta o gauge super da tela.
 * @details Define m_visible como false e marca para redesenho. O gauge não será
 *          mais desenhado até que show() seja chamado novamente.
 */
void GaugeSuper::hide()
{
  WidgetStateGuard widgetStateGuard;
    m_visible = false;
    m_shouldRedraw = true;
    ESP_LOGD(TAG, "GaugeSuper hidden at (%d, %d)", m_xPos, m_yPos);
}

/**
 * @brief Verifica se o título deve ser visível.
 * @return True se o título é visível, False caso contrário.
 * @details Verifica se o título foi definido e não está vazio.
 */
bool GaugeSuper::isTitleVisible() const
{
  WidgetStateGuard widgetStateGuard;
  return (m_title != nullptr && strlen(m_title) > 0);
}

/**
 * @brief Verifica se os rótulos devem ser visíveis.
 * @return True se os rótulos são visíveis, False caso contrário.
 * @details Verifica a flag showLabels da configuração.
 */
bool GaugeSuper::isLabelsVisible() const
{
  WidgetStateGuard widgetStateGuard;
  return m_config.showLabels;
}

/**
 * @brief Método centralizado de limpeza de memória.
 * @details Desaloca com segurança toda a memória dinâmica e reseta flags de alocação.
 *          Usado pelo destrutor e setup() para evitar vazamentos de memória.
 *          Verifica flags de alocação antes de desalocar para segurança.
 */
void GaugeSuper::cleanupMemory()
{
  WidgetStateGuard widgetStateGuard;
  m_loaded = false;
  m_initialized = false;
  // Clean up intervals array
  if (m_intervalsAllocated && m_intervals != nullptr) {
    delete[] m_intervals;
    m_intervals = nullptr;
    m_intervalsAllocated = false;
  }

  // Clean up colors array
  if (m_colorsAllocated && m_colors != nullptr) {
    delete[] m_colors;
    m_colors = nullptr;
    m_colorsAllocated = false;
  }

  // Clean up title string
  if (m_titleAllocated && m_title != nullptr) {
    delete[] m_title;
    m_title = nullptr;
    m_titleAllocated = false;
  }
  
  // Reset all pointers to nullptr for safety
  m_intervals = nullptr;
  m_colors = nullptr;
  m_title = nullptr;
}

// Color and value access methods removed - now using m_config directly

/**
 * @brief Valida a estrutura de configuração.
 * @param config Configuração a ser validada.
 * @return True se válida, False caso contrário.
 * @details Valida parâmetros básicos como largura, faixa de valores,
 *          número de intervalos e arrays de intervalos/cores.
 *          Registra erros no log do ESP32 para facilitar depuração.
 */
bool GaugeSuper::validateConfig(const GaugeConfig& config)
{
  WidgetStateGuard widgetStateGuard;
  // Validate basic parameters
  if (config.width <= 0) {
    ESP_LOGE(TAG, "Invalid width: %d", config.width);
    return false;
  }
  
  if (config.minValue >= config.maxValue) {
    ESP_LOGE(TAG, "Invalid range: min=%d, max=%d", config.minValue, config.maxValue);
    return false;
  }
  
  if (config.amountIntervals > MAX_SERIES) {
    ESP_LOGW(TAG, "Too many intervals: %d (max: %d)", config.amountIntervals, MAX_SERIES);
    return false;
  }
  
  // Validate intervals and colors arrays
  if (config.amountIntervals > 0) {
    if (config.intervals == nullptr || config.colors == nullptr) {
      ESP_LOGE(TAG, "Intervals or colors array is null");
      return false;
    }
  }
  
  for (size_t i = 0; i < config.amountIntervals; ++i) {
    if (config.intervals[i] < config.minValue || config.intervals[i] > config.maxValue ||
        (i && config.intervals[i] <= config.intervals[i - 1])) {
      ESP_LOGE(TAG, "Intervals must be increasing and within the gauge range");
      return false;
    }
  }
  return true;
}
#endif // DFK_DISABLE_GAUGE
