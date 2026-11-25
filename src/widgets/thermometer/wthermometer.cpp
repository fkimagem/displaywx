#include "../../widgets/widgetsetup.h"
#if !defined(DFK_DISABLE_THERMOMETER)
#include "wthermometer.h"
#include "../../extras/dwx_safe.h"

const char* Thermometer::TAG = "Thermometer";

/**
 * @brief Construtor para a classe Thermometer.
 * @param _x Posição X do widget.
 * @param _y Posição Y do widget.
 * @param _screen Número da tela.
 * @details Inicializa o widget com configuração vazia.
 *          O termômetro não será funcional até que setup() seja chamado.
 */
Thermometer::Thermometer(uint16_t _x, uint16_t _y, uint8_t _screen) : WidgetBase(_x, _y, _screen), m_config{} {
  m_border = 5;
  ESP_LOGD(TAG, "Thermometer created at (%d, %d) on screen %d", _x, _y, _screen);
}

/**
 * @brief Destrutor da classe Thermometer.
 * @details Limpa quaisquer recursos usados pelo Thermometer.
 */
Thermometer::~Thermometer() {
    cleanupMemory();
}

/**
 * @brief Limpa memória usada pelo Thermometer.
 * @details Thermometer não usa alocação dinâmica de memória.
 *          m_config.subtitle é um ponteiro para objeto Label externo.
 */
void Thermometer::cleanupMemory() {
  WidgetStateGuard widgetStateGuard;
    ESP_LOGD(TAG, "Thermometer memory cleanup completed");
}


/** @brief Ordena os valores mínimo e máximo para o widget Thermometer
 * @details Ordena os valores mínimo e máximo para o widget Thermometer
 * @return void
 */
void Thermometer::sortValues()
{
  WidgetStateGuard widgetStateGuard;
  if (m_config.minValue > m_config.maxValue)
  {
    int temp = m_config.minValue;
    m_config.minValue = m_config.maxValue;
    m_config.maxValue = temp;
  }
}


/** @brief Define o valor mínimo e máximo para o widget Thermometer
 * @details Define o valor mínimo e máximo para o widget Thermometer
 * @param newMinValue Valor mínimo
 * @param newMaxValue Valor máximo
 * @return void
 */
void Thermometer::setScale(int newMinValue, int newMaxValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newMinValue == newMaxValue) return;
  m_config.minValue = newMinValue;
  m_config.maxValue = newMaxValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}


/** @brief Define o valor mínimo para o widget Thermometer
 * @details Define o valor mínimo para o widget Thermometer
 * @param newValue Valor mínimo
 * @return void
 */
void Thermometer::setMinValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newValue == m_config.maxValue) return;
  m_config.minValue = newValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}


/** @brief Define o valor máximo para o widget Thermometer
 * @details Define o valor máximo para o widget Thermometer
 * @param newValue Valor máximo
 * @return void
 */
void Thermometer::setMaxValue(int newValue)
{
  WidgetStateGuard widgetStateGuard;
  if (newValue == m_config.minValue) return;
  m_config.maxValue = newValue;
  sortValues();
  m_currentValue = constrain(m_currentValue, m_config.minValue, m_config.maxValue);
  m_changedScale = true;
  m_shouldRedraw = true;
}


/** @brief Recupera o valor mínimo para o widget Thermometer
 * @details Recupera o valor mínimo para o widget Thermometer
 * @return Valor mínimo
 */
int Thermometer::getMinValue()
{
  WidgetStateGuard widgetStateGuard;
  return m_config.minValue;
}


/** @brief Recupera o valor máximo para o widget Thermometer
 * @details Recupera o valor máximo para o widget Thermometer
 * @return Valor máximo
 */
int Thermometer::getMaxValue()
{
  WidgetStateGuard widgetStateGuard;
  return m_config.maxValue;
}


/**
 * @brief Detecta um toque no widget Thermometer.
 * @param _xTouch Ponteiro para a coordenada X do toque.
 * @param _yTouch Ponteiro para a coordenada Y do toque.
 * @return True se o toque é detectado, False caso contrário.
 * @details Método override que sempre retorna false. O Thermometer não tem detecção de toque ativa.
 */
bool Thermometer::detectTouch(uint16_t *_xTouch, uint16_t *_yTouch)
{
  WidgetStateGuard widgetStateGuard;
  return false;
}

/**
 * @brief Recupera a função callback para o widget Thermometer.
 * @return Ponteiro para a função callback.
 * @details Retorna o ponteiro para a função que será executada quando o Thermometer for utilizado.
 */
functionCB_t Thermometer::getCallbackFunc()
{
  WidgetStateGuard widgetStateGuard;
  return m_callback;
}

/**
 * @brief Define o valor do widget Thermometer.
 * @param newValue Novo valor para definir.
 * @details Atualiza o valor do termômetro:
 *          - Restringe valor usando constrain() dentro da faixa min/max
 *          - Marca para redesenho
 */
void Thermometer::setValue(float newValue)
{
  WidgetStateGuard widgetStateGuard;
  m_currentValue = constrain(newValue, m_config.minValue, m_config.maxValue);
  // Serial.println("ajusta currentValue: " + String(currentValue));
  m_shouldRedraw = true;
  // redraw();
}

/**
 * @brief Redesenha o widget Thermometer.
 * @details Atualiza a exibição do termômetro:
 *          - Valida visibilidade, TFT, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Calcula altura do preenchimento usando dwx_map_clamped() baseado no valor atual
 *          - Atualiza área preenchida (limpando se valor diminuiu, preenchendo se aumentou)
 *          - Atualiza Label com valor formatado se subtitle estiver configurado
 *          - Armazena último valor e reseta flag de redesenho
 */
void Thermometer::redraw()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID
  
  #if defined(USING_GRAPHIC_LIB)

  uint32_t heightFill = dwx_map_clamped(m_currentValue, m_config.minValue, m_config.maxValue, 0, m_fillArea.height);
  uint32_t heightErase = m_fillArea.height - heightFill;

  int startY = m_fillArea.y + heightErase;

  if(m_changedScale){
    WidgetBase::objTFT->fillRoundRect(m_fillArea.x, m_fillArea.y, m_fillArea.width, m_fillArea.height, 0, m_config.backgroundColor);     // area do widget
  }


  if(m_currentValue < m_lastValue && !m_changedScale){
    WidgetBase::objTFT->fillRoundRect(m_fillArea.x, m_fillArea.y, m_fillArea.width, heightErase, 0, m_config.backgroundColor);     // area do widget
  }else{
    WidgetBase::objTFT->fillRoundRect(m_fillArea.x, startY, m_fillArea.width, heightFill, 0, m_config.filledColor);     // area do widget
  }

  if(m_config.subtitle){
    m_config.subtitle->setTextFloat(m_currentValue, m_config.decimalPlaces);
  }

  if(m_changedScale){
    m_changedScale = false;
  }

  m_lastValue = m_currentValue;
  m_shouldRedraw = false;

  #endif
}

/**
 * @brief Inicia o widget Thermometer.
 * @details Garante que a altura do termômetro esteja dentro de intervalos válidos:
 *          - Restringe altura dentro de bounds válidos (20 até altura da tela)
 */
void Thermometer::start()
{
  WidgetStateGuard widgetStateGuard;
#if defined(USING_GRAPHIC_LIB)
  m_config.height = constrain(m_config.height, 20, WidgetBase::objTFT->height());
#endif
}

/**
 * @brief Força o widget Thermometer a atualizar.
 * @details Define a flag de atualização para disparar um redesenho no próximo ciclo.
 */
void Thermometer::forceUpdate()
{
  WidgetStateGuard widgetStateGuard;
  m_shouldRedraw = true;
}

/**
 * @brief Desenha o fundo do widget Thermometer.
 * @details Desenha o termômetro completo incluindo fundo, bordas, bulbo e marcações:
 *          - Valida TFT, visibilidade, tela atual, uso de teclado, carregamento e flag de redesenho
 *          - Calcula posições e dimensões do bulbo, área de vidro e área de preenchimento
 *          - Desenha retângulo de fundo com bordas arredondadas
 *          - Desenha tubo de vidro com bordas e fundo
 *          - Desenha bulbo com gradiente de cor para efeito de luz
 *          - Desenha marcações na área de preenchimento
 */
void Thermometer::drawBackground()
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  CHECK_VISIBLE_VOID
  CHECK_CURRENTSCREEN_VOID
  CHECK_USINGKEYBOARD_VOID
  CHECK_LOADED_VOID
  CHECK_SHOULDREDRAW_VOID

  #if defined(USING_GRAPHIC_LIB)
  int radius = (m_config.width / 2) - m_border;

  m_bulb.x = m_xPos + (m_config.width / 2);
  m_bulb.y = m_yPos + m_config.height - radius - m_border;
  m_bulb.radius = radius;

  int width = m_config.width / 3;

  m_glassArea.x = m_xPos + ((m_config.width - width) / 2);
  m_glassArea.y = m_yPos + m_border;
  m_glassArea.width = width;
  //m_glassArea.height = m_config.height - radius;
  m_glassArea.height = m_config.height - (radius + m_border);

  m_fillArea.x = m_xPos + ((m_config.width - width) / 2);
  m_fillArea.y = m_yPos + m_border + (width/2);
  m_fillArea.width = width;
  m_fillArea.height = m_config.height - (m_border * 2 + m_bulb.radius * 2 + (width/2));

  WidgetBase::objTFT->fillRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, 5, m_config.backgroundColor);     // area do widget
  WidgetBase::objTFT->drawRoundRect(m_xPos, m_yPos, m_config.width, m_config.height, 5, CFK_BLACK);     // area do widget


  WidgetBase::objTFT->drawRoundRect(m_glassArea.x -1, m_glassArea.y-1, m_glassArea.width+2, m_glassArea.height+2, m_glassArea.width/2, CFK_BLACK);     // borda vidro
  WidgetBase::objTFT->fillRoundRect(m_glassArea.x, m_glassArea.y, m_glassArea.width, m_glassArea.height, m_glassArea.width/2, m_config.backgroundColor);     // fundo vidro

  WidgetBase::objTFT->fillRoundRect(m_glassArea.x, m_bulb.y - m_bulb.radius, m_glassArea.width, m_bulb.radius, 0, m_config.filledColor);// base do vidro (para nao ficar sobrando espaço em branco)

  for(uint8_t i = 0; i < m_colorLightGradientSize; i++)
    {
      uint8_t radius = m_bulb.radius - (i * 4);
      if(radius > m_bulb.radius){
        continue;//Aborta o loop se o raio for maior que o raio do LED (overflow do tipo uint8_t)
      }
      WidgetBase::objTFT->fillCircle(m_bulb.x - (2 * i), m_bulb.y - (2 * i), radius, m_colorLightGradient[i]);
    }
    WidgetBase::objTFT->drawCircle(m_bulb.x, m_bulb.y, m_bulb.radius, CFK_BLACK); // desenha a borda do bulb
	
	int amountSpace = 10;
	float space = m_fillArea.height / (float)amountSpace;
	
	int offset = 2;
	int size = 3;
	for(auto i = 0; i <= amountSpace;i++){
		int y = m_fillArea.y + i * space;
    uint8_t thickness = i % 5 == 0 ? 2*size : size;
		WidgetBase::objTFT->drawFastHLine(m_fillArea.x - (size + offset), y, thickness, m_config.markColor);
	}
  #endif
}

/**
 * @brief Configura o widget Thermometer.
 * @param _width Largura do widget.
 * @param _height Altura do widget.
 * @param _filledColor Cor para a porção preenchida do termômetro.
 * @param _vmin Valor mínimo da faixa.
 * @param _vmax Valor máximo da faixa.
 * @details Configura o termômetro com parâmetros básicos:
 *          - Valida se widget já está carregado
 *          - Calcula gradiente de cor para efeito de luz no bulbo
 *          - Define valor inicial como minValue
 *          - Marca para redesenho e chama start() para validar dimensões
 *          - Marca como carregado
 */
void Thermometer::setup(uint16_t _width, uint16_t _height, uint16_t _filledColor, int _vmin, int _vmax)
{
  WidgetStateGuard widgetStateGuard;
  if (m_loaded || _vmin >= _vmax || _width < 12 || _height < 24 ||
      _height <= _width + _width / 6u ||
      uint32_t(m_xPos) + _width > INT16_MAX || uint32_t(m_yPos) + _height > INT16_MAX) return;
#if defined(USING_GRAPHIC_LIB)
  if (!WidgetBase::objTFT || _height > WidgetBase::objTFT->height()) return;
#endif
  m_config.width = _width;
  m_config.height = _height;
  m_config.filledColor = _filledColor;
  m_config.minValue = _vmin;
  m_config.maxValue = _vmax;
  CHECK_TFT_VOID
  if (m_loaded)
  {
    ESP_LOGD(TAG, "Thermometer widget already configured");
    return;
  }
  //m_width = _width;
  //m_height = _height;
 // m_filledColor = _filledColor;
  for(uint8_t i = 0; i < m_colorLightGradientSize; i++)
  {
    //m_colorLightGradient[i] = WidgetBase::lightenToWhite565(m_config.filledColor, 0.08*i);
    m_colorLightGradient[i] = lighten565(m_config.filledColor, 0.1*i);
  }
  //m_vmin = _vmin;
  //m_vmax = _vmax;
  m_currentValue = m_config.minValue;
  m_shouldRedraw = true;
  start();
  m_loaded = true;
}

/**
 * @brief Configura o Thermometer com parâmetros definidos em uma estrutura de configuração.
 * @param config Estrutura @ref ThermometerConfig contendo os parâmetros de configuração.
 * @details Este método deve ser chamado após criar o objeto para configurá-lo adequadamente:
 *          - Limpa memória existente usando cleanupMemory()
 *          - Copia configuração
 *          - Configura subtitle com unit se fornecido
 *          - Chama método setup() com parâmetros individuais
 *          - Marca o widget como carregado e inicializado
 *          O termômetro não será exibido corretamente até que este método seja chamado.
 */
void Thermometer::setup(const ThermometerConfig& config)
{
  WidgetStateGuard widgetStateGuard;
  CHECK_TFT_VOID
  if (m_loaded) {
    ESP_LOGW(TAG, "Thermometer already initialized");
    return;
  }

  // Clean up any existing memory before setting new config
  cleanupMemory();
  
  // Deep copy configuration
  if (config.minValue >= config.maxValue || config.width < 12 || config.height < 24) return;
  m_config = config;
  
  // Set up subtitle if provided
  if(m_config.subtitle){
    m_config.subtitle->setSuffix(m_config.unit);
  }
  
  // Call the original setup method with config parameters
  setup(config.width, config.height, config.filledColor, config.minValue, config.maxValue);
  
  ESP_LOGD(TAG, "Thermometer configured: %dx%d, range=[%d,%d], unit='%s'", 
           config.width, config.height, config.minValue, config.maxValue, config.unit);
}

/**
 * @brief Torna o Thermometer visível na tela.
 * @details Define o widget como visível e marca para redesenho.
 */
void Thermometer::show()
{
  WidgetStateGuard widgetStateGuard;
    m_visible = true;
    m_shouldRedraw = true;
}

/**
 * @brief Oculta o Thermometer da tela.
 * @details Define o widget como invisível e marca para redesenho.
 */
void Thermometer::hide()
{
  WidgetStateGuard widgetStateGuard;
    m_visible = false;
    m_shouldRedraw = true;
}

#endif // DFK_DISABLE_THERMOMETER
