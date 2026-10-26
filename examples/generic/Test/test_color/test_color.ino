#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 480 // Largura do display
#define DISPLAY_H 320 // Altura do display
#define DISP_CS	10 // Pino CS do display
#define DISP_MISO	13 // Pino MISO do display
#define DISP_MOSI	11 // Pino MOSI do display
#define DISP_SCLK	12 // Pino SCL do display
#define DISP_RST	9 // Pino RST do display
#define DISP_DC	8 // Pino DC do display
#define DISP_FREQUENCY	27000000 // Frequencia do SPI do display
#define TC_CS	21 // Pino CS do touch
#define TC_SCL	12 // Pino SCL do touch
#define TC_MOSI	11 // Pino MOSI do touch
#define TC_MISO	13 // Pino MISO do touch
#define TOUCH_FREQUENCY	2500000 // Frequencia SPI do touch
#define TOUCH_INVERT_X	false // Flag que indica se o eixo X está invertido no touch
#define TOUCH_INVERT_Y	false // Flag que indica se o eixo Y está invertido no touch

// Os quatro parametros abaixo indicam o map do touch (ex: eixo vai de 0 a 480 e y de 0 a 320)
#define TOUCH_MAP_X0	0 // Pontos x0
#define TOUCH_MAP_X1	480
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	320
#define TOUCH_SWAP_XY	false // Flag que indica se os eixos estao trocados

// prototipo da função que configura os widgets
void loadWidgets();

// Prototipos das telas
void screen0();

#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>


#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);              // Se estiver usando um ESP32-S3, inicializa o barramento SPI no canal FSPI (mais rápido e dedicado)
#else
SPIClass spi_shared(HSPI);              // Caso contrário (ESP32 convencional), usa o canal HSPI para comunicação SPI compartilhada
#endif

Arduino_DataBus *bus = nullptr;         // Ponteiro para o barramento de dados do display (será instanciado posteriormente com as configurações corretas)
Arduino_GFX *tft = nullptr;             // Ponteiro para o objeto gráfico principal do display (gerencia desenho, cores, texto, etc.)
uint8_t rotationScreen = 1;             // Define a rotação da tela (0 a 3); valor 1 corresponde a 90° (paisagem padrão para 480x320)

DisplayWX myDisplay;                    // Instancia do objeto principal da biblioteca `displaywx`, que gerencia widgets, eventos e o desenho na tela


void setup()
{
    Serial.begin(115200); // Inicializa a comunicação serial para debug, com baudrate 115200

    // Inicializa o barramento SPI compartilhado com os pinos definidos para o display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);

    // Cria um objeto de barramento SPI de hardware com os pinos do display
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);

    // Cria o objeto da tela com controlador ILI9488
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, false);

    // Inicializa o display com a frequência de SPI especificada
    tft->begin(DISP_FREQUENCY);

    // Define o objeto global do display dentro da classe base dos widgets (para que todos possam desenhar)
    WidgetBase::objTFT = tft;

    // Escolha seu touch - Não esqueça de atualizar seu user_setup.h
    //myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, TC_INT, TC_RST);
    //myDisplay.startTouchFT6236U(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, -1, -1);
    //myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, TC_INT, TC_RST);
    //myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TC_CS, &spi_shared, tft, TOUCH_FREQUENCY, DISP_FREQUENCY, DISP_CS);
    //myDisplay.startTouchXPT2046(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SCL, TC_MOSI, TC_MISO, TC_CS, tft, TOUCH_FREQUENCY, DISP_FREQUENCY, DISP_CS);

    // myDisplay.recalibrate(); // (opcional) re-calibração do touch se necessário
    myDisplay.checkCalibration(); // Verifica se o touch está calibrado, pode carregar dados salvos
    myDisplay.enableTouchLog(); // (opcional) ativa log de toque na serial para debug

    loadWidgets(); // Função que configura todos os widgets (botões, gráficos, spinboxes, etc.)
    WidgetBase::loadScreen = screen0; // Define qual tela será carregada inicialmente ao iniciar a interface gráfica
    myDisplay.createTask(); // Cria a tarefa principal da interface gráfica (usa FreeRTOS para lidar com toque e atualização de tela)
}

void loop()
{
    delay(100);
}
              
void screen0()
{

    tft->fillScreen(CFK_BLACK); // Preenche toda a tela com a cor cinza escuro (CFK_GREY3) como fundo
    WidgetBase::backgroundColor = CFK_BLACK; // Define a cor de fundo padrão para os widgets desenhados nesta tela

    uint16_t c_red = 0xf800;
    uint16_t c_green = 0x07e0;
    uint16_t c_blue = 0x001f;
    uint16_t c_yellow = 0xffe0;
    uint16_t c_cyan = 0x07ff;
    uint16_t c_magenta = 0xf81f;
    uint16_t c_white = 0xffff;
    uint16_t c_black = 0x0000;


    tft->fillCircle(20, 20, 10, c_red);
    tft->fillCircle(60, 20, 10, c_green);
    tft->fillCircle(80, 20, 10, c_blue);
    tft->fillCircle(140, 20, 10, c_yellow);
    tft->fillCircle(180, 20, 10, c_cyan);
    tft->fillCircle(220, 20, 10, c_magenta);
    tft->fillCircle(260, 20, 10, c_white);
    tft->fillCircle(300, 20, 10, c_black);

    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets()
{
}