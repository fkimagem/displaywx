// [legenda] [EdgeWX] = linha emitida pelo exportador. [manual] = alimentacao, clock do OV2710 e os tres botoes de zoom.
// [legenda] Painel ST7789 em SPI, sem toque. A camera ainda chama startPpa(); o destino da miniatura neste painel e SPI, nao DSI.
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] Arduino_GFX pede SPI.h mesmo quando o painel e MIPI e nao usa esse barramento.
#include <SPI.h>
// [EdgeWX] Classes do painel, primitivas e fontes GFX.
#include <Arduino_GFX_Library.h>
// [EdgeWX] Biblioteca: widgets, toque, camera e a tarefa de UI.
#include <displaywx.h>
// [EdgeWX] Abre o sensor MIPI. Entra em todo export que tem CameraMipi.
#include <ESP_Video.h>


    // [EdgeWX] Abre a ficha da placa. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7789 - SPIDMA
    * Touch: None
    * Camera: OV2710
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e lido neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; este sketch nao le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura do painel em pixels, usada no toque e como tamanho logico.
const int DISPLAY_W = 240;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 240;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_FREQUENCY = 40000000;
// [EdgeWX] SDA do SCCB do sensor.
const int CAM_SDA = 14;
// [EdgeWX] SCL do SCCB do sensor.
const int CAM_SCL = 13;
// [EdgeWX] O nome diz SCCB, mas 24 MHz e o XCLK do OV2710. O I2C de startCamera recebe 400000 explicito.
const int CAM_FREQUENCY_SCCB = 24000000;
// [EdgeWX] Interrupcao da camera. -1 = nao ligada. O gerador entrega este valor no argumento pwdn de startCamera.
const int CAM_INT = -1;
// [EdgeWX] Reset da camera. -1 = nao ligado.
const int CAM_RST = 26;
// [EdgeWX] Largura nativa pedida ao sensor. O widget mostra so um recorte.
const int CAM_WIDTH = 1920;
// [EdgeWX] Altura nativa do sensor.
const int CAM_HEIGHT = 1080;
// [EdgeWX] Porta I2C do SCCB.
const int CAM_I2C_PORT = 0;
// [EdgeWX] Espelho horizontal. O PPA aplica isto ao carregar a foto.
const bool CAM_MIRROR_X = false;
// [EdgeWX] Espelho vertical. O PPA aplica isto ao carregar a foto.
const bool CAM_MIRROR_Y = false;
// [manual] GPIO do XCLK. Nesta placa o clock sai daqui, nao do nome SCCB.
const int CAM_PIN_CLK = 11;
// [manual] GPIO que liga a alimentacao da camera e do LCD.
const int CAM_PIN_PWR_EN = 12;
// [EdgeWX] GPIO de reset do painel. -1 quer dizer nao ligado.
const int DISP_RST = 15;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_DC = 19;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_CS = 18;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_MOSI = 16;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_MISO = -1;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_SCLK = 17;
// [EdgeWX] Backlight do ST7789 no GPIO 20.
const int DISP_LED = 20;
// [EdgeWX] Rotacao 0..3 do painel. O comentario em ingles no fim da linha e do gerador.
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
// [EdgeWX] ST7789 IPS. Aqui isIPS entra no construtor, diferente do caminho DSI.
const bool isIPS = true; // Come display can use this as bigEndian flag

// [manual] GPIO do botao de zoom 1. Apertado le LOW.
const int ZOOM_PIN_1X = 3;
// [manual] GPIO do botao de zoom 2.
const int ZOOM_PIN_2X = 4;
// [manual] GPIO do botao de zoom 4. O pino se chama 3X, o nivel pedido e 4.
const int ZOOM_PIN_3X = 5;

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0();
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions

// [EdgeWX] Ponteiro preenchido no setup.
Arduino_DataBus *bus = nullptr;
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_GFX *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] CameraMipi cameraMipi no ponto (0, 0), tela 0. Nome e posicao vieram do editor.
CameraMipi cameraMipi(0, 0, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCameraMipi = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};


// [manual] Prepara os tres botoes de zoom. Pull-up: solto le HIGH.
static void beginZoomButtons()
{
    // [manual] Configura o pino como entrada ou saida.
    pinMode(ZOOM_PIN_1X, INPUT_PULLUP);
    // [manual] Configura o pino como entrada ou saida.
    pinMode(ZOOM_PIN_2X, INPUT_PULLUP);
    // [manual] Configura o pino como entrada ou saida.
    pinMode(ZOOM_PIN_3X, INPUT_PULLUP);
}

// [manual] Le os tres botoes. Nao espera o toque, porque este projeto nao tem touch.
static void pollZoomButtons()
{
    // [manual] Le o pino. LOW, com pull-up, quer dizer botao apertado.
    if (digitalRead(ZOOM_PIN_1X) == LOW) {
        // [manual] Pede o zoom da camera. 1 e o recorte largo; 2 e 4 aproximam o centro.
        cameraMipi.setZoomLevel(1);
        // [manual] Sai da funcao.
        return;
    }
    // [manual] Le o pino. LOW, com pull-up, quer dizer botao apertado.
    if (digitalRead(ZOOM_PIN_2X) == LOW) {
        // [manual] Pede o zoom da camera. 1 e o recorte largo; 2 e 4 aproximam o centro.
        cameraMipi.setZoomLevel(2);
        // [manual] Sai da funcao.
        return;
    }
    // [manual] Le o pino. LOW, com pull-up, quer dizer botao apertado.
    if (digitalRead(ZOOM_PIN_3X) == LOW) {
        // [manual] Pede o zoom da camera. 1 e o recorte largo; 2 e 4 aproximam o centro.
        cameraMipi.setZoomLevel(4);
    }
}

// [EdgeWX] Marca do gerador imediatamente antes do setup.
// linha de camera
// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){

    // [EdgeWX] Aviso do gerador: o projeto nao tem toque, entao nao ha startTouch.
    // Touch model not defined. No need startTouch() function.
    // [EdgeWX] Aviso do gerador: o projeto nao tem toque, entao nao ha startTouch.
    // Touch model not defined. No need startTouch() function.
    // [EdgeWX] Aviso do gerador: o projeto nao tem toque, entao nao ha startTouch.
    // Touch model not defined. No need startTouch() function.
    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [manual] Configura os GPIO 3, 4 e 5 como entrada com pull-up.
    beginZoomButtons();
    
    // [EdgeWX] Comentario do gerador abrindo o bloco da camera.
    // Setup camera
    // [manual] Condicao ou repeticao.
    if (!myDisplay.getCamera().enableBoardPower(CAM_PIN_PWR_EN)){
        // [manual] A alimentacao da camera e do LCD nao ligou. O setup para aqui.
        Serial.println("failed to enable camera/LCD power (GPIO CAM_PIN_PWR_EN)");
        // [manual] Sai do setup. O painel nao chega a ser criado.
        return;
    }
    // [manual] Condicao ou repeticao.
    if (!myDisplay.getCamera().startXclk(CAM_PIN_CLK, CAM_FREQUENCY_SCCB)){
        // [manual] Sai do setup. O painel nao chega a ser criado.
        return;
    }
    // [EdgeWX] Guarda rotacao e espelho. O PPA aplica isso ao carregar a foto.
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    // [EdgeWX] O quarto argumento numerico e 400000, o I2C. CAM_FREQUENCY_SCCB ficou no XCLK.
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, 400000, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        // [EdgeWX] Contrata o PPA. A miniatura passa a ir por DMA, sem o processador copiar ponto a ponto.
        myDisplay.getCamera().startPpa();
    // [EdgeWX] Ramo falso da condicao de cima.
    }else{
        // [EdgeWX] Avisa na serial quando o sensor nao abre. Sem isso nao ha tarefa de camera.
        Serial.println("Camera not started");
    }
    // [EdgeWX] Backlight como saida.
    pinMode(DISP_LED, OUTPUT);
    // [EdgeWX] Nivel baixo no backlight antes do begin.
    digitalWrite(DISP_LED, LOW);
    // [EdgeWX] SPI com DMA para o ST7789. Esse DMA e do barramento da tela, nao o PPA da camera.
    bus = new Arduino_ESP32SPIDMA(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, SPI2_HOST + 1, false);
    // [EdgeWX] Painel 240x240. Os quatro zeros finais sao os offsets de coluna e pagina.
    tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS, DISPLAY_W, DISPLAY_H, 0, 0, 0, 0);
    // [EdgeWX] Inicializa o painel SPI no clock DISP_FREQUENCY.
    tft->begin(DISP_FREQUENCY);
    // [EdgeWX] Entrega o display aos widgets e, se houver camera, ao caminho do preview. O comentario em ingles e do gerador.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo individualy, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra a funcao da tela. A tarefa de UI chama esse desenho.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false desliga o watchdog; 3 e o timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    
}

// [EdgeWX] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop(){

    // [manual] Le os tres botoes. Nao espera o toque, porque este projeto nao tem touch.
    pollZoomButtons();
}

// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0(){

    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_WHITE);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an cameraMipi
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e o desta chamada.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Comentario fixo do gerador, inclusive o typo widgtes.
// Configure each widgtes to be used
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets(){

    // [EdgeWX] Struct copiada do editor para este widget.
    CameraMipiConfig configCameraMipi0 = {
            // [EdgeWX] Largura, em pixels.
            .width = 240,
            // [EdgeWX] Altura, em pixels.
            .height = 240,
            // [EdgeWX] true escreve o zoom no preview. false esconde.
            .showZoomLevel = false,
            // [EdgeWX] true escreve os quadros por segundo. false esconde.
            .showFps = false,
            // [EdgeWX] Trocas da miniatura por segundo. A tarefa espera 1000/fps ms.
            .fps = 5
        };

    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    cameraMipi.setup(configCameraMipi0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


}
