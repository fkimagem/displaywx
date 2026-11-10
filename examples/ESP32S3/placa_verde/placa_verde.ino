// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. [manual] = escrita depois (GPIOs, lerGPIOs, task do buzzer).
// [legenda] MCU ESP32S3, painel ST7796 SPI 480x320, toque FT6336U, sem camera; isIPS true no construtor SPI.
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Em ESP32-S2/S3 o alias VSPI aponta para FSPI, usado pelo SPIClass abaixo.
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
// [EdgeWX] Define VSPI como FSPI nestes alvos.
#define VSPI FSPI
#endif
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] SPI.h para o barramento FSPI do ST7796.
#include <SPI.h>
// [EdgeWX] Arduino_GFX: Arduino_HWSPI, Arduino_ST7796 e primitivas de desenho.
#include <Arduino_GFX_Library.h>
// [EdgeWX] HAL DisplayWX: widgets, toque FT6336U e a tarefa de UI.
#include <displaywx.h>


// [EdgeWX] Ficha da placa escolhida no EdgeWX. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32S3
    * Display: ST7796
    * Touch: FT6336U
    * Camera: None
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e usado neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; aqui ninguem le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura util do painel apos rotacao 1: 480 px.
const int DISPLAY_W = 480;
// [EdgeWX] Altura util do painel apos rotacao 1: 320 px.
const int DISPLAY_H = 320;
// [EdgeWX] Clock SPI do ST7796 em 27 MHz, passado a tft->begin.
const int DISP_FREQUENCY = 27000000;
// [EdgeWX] Canto minimo X do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto minimo Y do mapa de toque.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto maximo X do mapa; 320 casa com a altura nativa do painel.
const int TOUCH_MAP_X1 = 320;
// [EdgeWX] Canto maximo Y do mapa; 480 casa com a largura nativa do painel.
const int TOUCH_MAP_Y1 = 480;
// [EdgeWX] true: troca X e Y do FT6336U para alinhar com rotationScreen 1.
const bool TOUCH_SWAP_XY = true;
// [EdgeWX] false: nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] true: espelha o eixo Y do FT6336U nesta montagem.
const bool TOUCH_INVERT_Y = true;
// [EdgeWX] GPIO MOSI do ST7796 no ESP32-S3.
const int DISP_MOSI = 11;
// [EdgeWX] MISO desligado (-1); o painel e write-only neste desenho.
const int DISP_MISO = -1;
// [EdgeWX] GPIO SCLK do SPI do display.
const int DISP_SCLK = 12;
// [EdgeWX] Chip select do ST7796.
const int DISP_CS = 10;
// [EdgeWX] Data/Command do ST7796.
const int DISP_DC = 8;
// [EdgeWX] Reset do painel ST7796.
const int DISP_RST = 9;
// [EdgeWX] SCL I2C do FT6336U.
const int TOUCH_SCL = 13;
// [EdgeWX] SDA I2C do FT6336U.
const int TOUCH_SDA = 14;
// [EdgeWX] Interrupcao do FT6336U.
const int TOUCH_INT = 18;
// [EdgeWX] Reset do FT6336U.
const int TOUCH_RST = 17;
// [EdgeWX] Rotacao 1 (paisagem). O comentario em ingles e do gerador.
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] true: painel IPS; entra no construtor SPI. O typo "Come" e do gerador.
const bool isIPS = true; // Come display can use this as bigEndian flag

// [manual] Bloco dos GPIOs da placa verde (buzzer, botoes e potenciometro).
// Pinos GPIOs
// [manual] GPIO do buzzer ativo em nivel alto.
const int pinBuzzer = 39;

// [manual] Botao fisico verde; lido com INPUT_PULLUP (pressionado = LOW).
const int pinBotaoVerde = 47;
// [manual] Botao fisico amarelo; INPUT_PULLUP.
const int pinBotaoAmarelo = 48;
// [manual] Botao fisico vermelho; INPUT_PULLUP.
const int pinBotaoVermelho = 35;

// [manual] Entrada analogica do potenciometro (ADC 0..4095).
const int pinPotenciometro = 7;

// [manual] Flag pedida pelo TextButton; a task do buzzer consome e zera.
volatile bool pedirApitoBuzzer = false;

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Unica tela desenhada no editor. Indice 0.
void screen0();
// [EdgeWX] Aplica GaugeConfig, LedConfig e TextButtonConfig.
void loadWidgets();
// [manual] Configura pinMode dos GPIOs e forca o buzzer em LOW.
void iniciarGPIOs();
// [manual] Le botoes/potenciometro e atualiza widgets da tela.
void lerGPIOs();
// [manual] Task FreeRTOS que apita o buzzer por 500 ms quando a flag sobe.
void taskBuzzer(void *pvParameters);

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do TextButton textbutton1 (Buzzer).
void textbutton1_cb();

// [EdgeWX] Comentario do gerador abrindo o SPI global.
// Create global SPI object
// [EdgeWX] SPIClass no host FSPI do ESP32-S3, compartilhado com o HWSPI.
SPIClass spiPrimary(FSPI);
// [EdgeWX] Ponteiro do barramento Arduino_HWSPI. Preenchido no setup.
Arduino_DataBus *bus = nullptr;
// [EdgeWX] Ponteiro do Arduino_ST7796. screen0 e os widgets desenham nele.
Arduino_GFX *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque e a tarefa de UI.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] GaugeSuper em (240, 300), tela 0: ponteiro analogico do potenciometro.
GaugeSuper gauge(240, 300, 0);
// [EdgeWX] Quantidade de gauges passada a setGauge.
const uint8_t qtdGauge = 1;
// [EdgeWX] Array de ponteiros exigido por myDisplay.setGauge.
GaugeSuper *arrayGauge[qtdGauge] = {&gauge};
// [EdgeWX] Quantidade de intervalos de cor do gauge 0.
const uint8_t qtdIntervalG0 = 1;
// [EdgeWX] Limites dos intervalos; um unico marco em 0.
int range0[qtdIntervalG0] = {0};
// [EdgeWX] Cor do unico intervalo do gauge: branco.
uint16_t colors0[qtdIntervalG0] = {CFK_WHITE};
// [manual] Led verde reposicionado em (121, 101), tela 0.
Led ledtelaverde(121, 101, 0);
// [manual] Led amarelo reposicionado em (161, 101), tela 0.
Led ledtelaamarelo(161, 101, 0);
// [manual] Led vermelho reposicionado em (201, 101), tela 0.
Led ledtelavermelho(201, 101, 0);
// [EdgeWX] Tres leds registrados em setLed.
const uint8_t qtdLed = 3;
// [EdgeWX] Array na ordem verde, amarelo, vermelho.
Led *arrayLed[qtdLed] = {&ledtelaverde, &ledtelaamarelo, &ledtelavermelho};
// [manual] TextButton "Buzzer" reposicionado em (340, 80), tela 0.
TextButton textbutton1(340, 80, 0);
// [EdgeWX] Um TextButton neste export.
const uint8_t qtdTextButton = 1;
// [EdgeWX] Array de ponteiros para setTextButton.
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};

// [EdgeWX] Assinatura do setup do Arduino. Inicializa SPI, ST7796, FT6336U e a UI.
void setup(){

    // [EdgeWX] Abre a serial a 115200 para o log dos callbacks.
    Serial.begin(115200);
    
    // [manual] Prepara leds, buzzer, botoes e potenciometro antes da UI.
    iniciarGPIOs();
    // [manual] Sobe a task que escuta pedirApitoBuzzer e toca o buzzer.
    xTaskCreate(taskBuzzer, "taskBuzzer", 2048, NULL, 1, NULL);
    // [EdgeWX] Comentario do gerador abrindo o begin do SPI.
    // Start SPI object for display
    // [EdgeWX] Sobe spiPrimary com SCLK, MISO e MOSI do ST7796.
    spiPrimary.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    // [EdgeWX] Cria o Arduino_HWSPI com DC, CS e os pinos SPI, ligado a spiPrimary.
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spiPrimary);
    // [EdgeWX] Instancia o ST7796: reset, rotacao 1, IPS, geometria nativa 320x480.
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    // [EdgeWX] Inicia o painel a 27 MHz.
    tft->begin(DISP_FREQUENCY);
    // [EdgeWX] Entrega o Arduino_GFX aos widgets. O comentario em ingles e do gerador.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Comentario do gerador abrindo o bloco do FT6336U.
    // Setup touch
    // [EdgeWX] Grava o retangulo de calibracao 0,320 x 0,480.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Aplica o espelhamento X/Y definido nas constantes.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Troca os eixos do toque (TOUCH_SWAP_XY true).
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o FT6336U na I2C com SDA, SCL, INT e RST deste desenho.
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Log de toque deixado comentado pelo gerador neste export.
    //myDisplay.enableTouchLog();
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo "individualy", e do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra screen0 como tela carregada pela tarefa de UI.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. true = watchdog ligado; 3 = timeout em segundos.
    myDisplay.createTask(true, 3); // Initialize the task to read touch and draw
    // [EdgeWX] Registra a task atual no watchdog; createTask(true) deixou o WDT ativo.
    esp_task_wdt_add(NULL);
}

// [EdgeWX] Assinatura do loop do Arduino. Agora so atualiza widgets via lerGPIOs.
void loop(){

    // [EdgeWX] Toma o mutex de desenho antes de alterar widgets.
    myDisplay.startCustomDraw();
    // [manual] Le botoes/potenciometro e espelha nos widgets Led e Gauge.
    lerGPIOs();
    // [EdgeWX] Devolve o mutex. A tarefa DisplayWX redesenha o que mudou.
    myDisplay.finishCustomDraw();
    // [EdgeWX] Alimenta o watchdog da task. O comentario em ingles e do gerador.
    esp_task_wdt_reset(); // Reset watchdog
    // [manual] Folga curta para nao saturar o loop nem atrasar a leitura dos botoes.
    delay(50);
}

// [EdgeWX] Desenho estatico da tela 0. A tarefa chama isto ao carregar a tela; widgets entram no final.
void screen0(){

    // [EdgeWX] Pinta o fundo CFK_COLOR015 e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_COLOR015);
    // [EdgeWX] Rotulo "Botoes" na faixa superior da tela.
    myDisplay.printText("Botoes", 28, 93, TL_DATUM, CFK_BLACK, CFK_COLOR015, &RobotoRegular10pt7b);
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e 0.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Comentario fixo do gerador, inclusive o typo "widgtes".
// Configure each widgtes to be used
// [manual] Preenche as structs na ordem: gauge, leds, TextButton (toggles removidos).
void loadWidgets(){

    // [EdgeWX] Propriedades do GaugeSuper copiadas do editor.
    GaugeConfig configGauge0 = {
            // [EdgeWX] Titulo exibido no gauge.
            .title = "Potenciometro",
            // [EdgeWX] Vetor de limites dos intervalos.
            .intervals = range0,
            // [EdgeWX] Vetor de cores dos intervalos.
            .colors = colors0,
            // [EdgeWX] Fonte dos numeros: Roboto Regular 8 pt.
            .fontFamily = &RobotoRegular8pt7b,
            // [EdgeWX] Valor minimo do ponteiro.
            .minValue = 0,
            // [EdgeWX] Valor maximo 4095, tipico de ADC de 12 bits.
            .maxValue = 4095,
            // [EdgeWX] Largura do widget em pixels.
            .width = 269,
            // [EdgeWX] Altura do widget em pixels.
            .height = 100,
            // [EdgeWX] Cor da borda CFK_COLOR052.
            .borderColor = CFK_COLOR052,
            // [EdgeWX] Cor do texto dos labels.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo branco do gauge.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor do titulo.
            .titleColor = CFK_BLACK,
            // [EdgeWX] Agulha vermelha.
            .needleColor = CFK_RED,
            // [EdgeWX] Marcadores pretos na escala.
            .markersColor = CFK_BLACK,
            // [EdgeWX] Quantidade de intervalos = qtdIntervalG0.
            .amountIntervals = qtdIntervalG0,
            // [EdgeWX] Mostra os labels numericos na escala.
            .showLabels = true
        };
    // [EdgeWX] Aplica a struct ao GaugeSuper. Sem setup o gauge nao redesenha.
    gauge.setup(configGauge0);
    // [EdgeWX] Registra o array de gauges na DisplayWX.
    myDisplay.setGauge(arrayGauge,qtdGauge);


    // [EdgeWX] Config do led verde: raio 16, ligado em CFK_COLOR037.
    LedConfig configLed0 = {
            // [EdgeWX] Raio do circulo do led.
            .radius = 16,
            // [EdgeWX] Cor quando ligado (verde do editor).
            .colorOn = CFK_COLOR037,
            // [EdgeWX] Cor quando desligado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] Estado inicial desligado.
            .initialState = false
        };
    // [EdgeWX] Aplica a config ao led verde.
    ledtelaverde.setup(configLed0);
    // [EdgeWX] Config do led amarelo: ligado em CFK_COLOR019, inicia ligado.
    LedConfig configLed1 = {
            // [EdgeWX] Mesmo raio 16.
            .radius = 16,
            // [EdgeWX] Cor quando ligado (amarelo do editor).
            .colorOn = CFK_COLOR019,
            // [EdgeWX] Cor quando desligado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] Estado inicial ligado.
            .initialState = true
        };
    // [EdgeWX] Aplica a config ao led amarelo.
    ledtelaamarelo.setup(configLed1);
    // [EdgeWX] Config do led vermelho: ligado em CFK_COLOR007, inicia ligado.
    LedConfig configLed2 = {
            // [EdgeWX] Mesmo raio 16.
            .radius = 16,
            // [EdgeWX] Cor quando ligado (vermelho do editor).
            .colorOn = CFK_COLOR007,
            // [EdgeWX] Cor quando desligado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] Estado inicial ligado.
            .initialState = true
        };
    // [EdgeWX] Aplica a config ao led vermelho.
    ledtelavermelho.setup(configLed2);
    // [EdgeWX] Registra os tres leds na DisplayWX.
    myDisplay.setLed(arrayLed,qtdLed);


    // [EdgeWX] Propriedades do TextButton "Buzzer" copiadas do editor.
    TextButtonConfig configTextButton0 = {
            // [EdgeWX] Texto central do botao.
            .text = "Buzzer",
            // [EdgeWX] A tarefa de UI chama textbutton1_cb no toque.
            .callback = textbutton1_cb,
            // [EdgeWX] Fonte Roboto Bold 10 pt.
            .fontFamily = &RobotoBold10pt7b,
            // [EdgeWX] Largura 100 px.
            .width = 100,
            // [EdgeWX] Altura 40 px.
            .height = 40,
            // [EdgeWX] Canto arredondado com raio 10.
            .radius = 10,
            // [EdgeWX] Fundo CFK_COLOR046.
            .backgroundColor = CFK_COLOR046,
            // [EdgeWX] Texto branco.
            .textColor = CFK_WHITE
        };

    // [EdgeWX] Aplica a struct ao TextButton.
    textbutton1.setup(configTextButton0);
    // [EdgeWX] Registra o array de TextButton na DisplayWX.
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

// [EdgeWX] Comentario fixo do gerador: callback do textbutton1.
// This function is a callback of this element textbutton1.
// [EdgeWX] Texto fixo do gerador: nao chame o callback no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Corpo do callback do TextButton Buzzer.
void textbutton1_cb(){

    // [EdgeWX] Log padrao do gerador ao clicar no textbutton1.
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
    // [manual] Pede um apito de 500 ms para a taskBuzzer.
    pedirApitoBuzzer = true;
}

// [manual] Configura pinModes e deixa o buzzer em LOW no boot.
void iniciarGPIOs(){
    // [manual] Buzzer como saida.
    pinMode(pinBuzzer, OUTPUT);
    // [manual] Botoes com pull-up interno (solto = HIGH, apertado = LOW).
    pinMode(pinBotaoVerde, INPUT_PULLUP);
    pinMode(pinBotaoAmarelo, INPUT_PULLUP);
    pinMode(pinBotaoVermelho, INPUT_PULLUP);
    // [manual] Potenciometro como entrada analogica.
    pinMode(pinPotenciometro, INPUT);
    // [manual] Garante o buzzer desligado ao iniciar.
    digitalWrite(pinBuzzer, LOW);
}

// [manual] Atualiza widgets da tela a partir dos GPIOs fisicos.
void lerGPIOs(){
    // [manual] Botao verde apertado/solto espelha no Led da tela (API do widget e setState).
    ledtelaverde.setState(digitalRead(pinBotaoVerde) == LOW);
    // [manual] Botao amarelo -> led amarelo da tela.
    ledtelaamarelo.setState(digitalRead(pinBotaoAmarelo) == LOW);
    // [manual] Botao vermelho -> led vermelho da tela.
    ledtelavermelho.setState(digitalRead(pinBotaoVermelho) == LOW);
    // [manual] ADC do potenciometro vai para o GaugeSuper (0..4095).
    gauge.setValue(analogRead(pinPotenciometro));
}

// [manual] Task dedicada: quando pedirApitoBuzzer fica true, liga o buzzer 500 ms.
void taskBuzzer(void *pvParameters){
    (void)pvParameters;
    for(;;){
        if(pedirApitoBuzzer){
            pedirApitoBuzzer = false;
            digitalWrite(pinBuzzer, HIGH);
            vTaskDelay(pdMS_TO_TICKS(500));
            digitalWrite(pinBuzzer, LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
