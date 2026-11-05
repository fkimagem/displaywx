// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. Este sketch nao tem logica escrita a mao.
// [legenda] Preview da OV02C10 no ST7701 480x800 com GT911. Nao ha user_setup.h nesta pasta.
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
    * Display: ST7701 - DSI
    * Touch: GT911
    * Camera: OV02C10
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e lido neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; este sketch nao le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura do painel em pixels, usada no toque e como tamanho logico.
const int DISPLAY_W = 480;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 800;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X1 = 480;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y1 = 800;
// [EdgeWX] Eixo do toque: nao troca X com Y.
const bool TOUCH_SWAP_XY = false;
// [EdgeWX] nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] nao espelha o eixo Y do toque.
const bool TOUCH_INVERT_Y = false;
// [EdgeWX] SDA do SCCB do sensor.
const int CAM_SDA = 7;
// [EdgeWX] SCL do SCCB do sensor.
const int CAM_SCL = 8;
// [EdgeWX] Clock I2C do sensor, em Hz.
const int CAM_FREQUENCY_SCCB = 400000;
// [EdgeWX] Interrupcao da camera. -1 = nao ligada. O gerador entrega este valor no argumento pwdn de startCamera.
const int CAM_INT = -1;
// [EdgeWX] Reset da camera. -1 = nao ligado.
const int CAM_RST = -1;
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
// [EdgeWX] GPIO de reset do painel. -1 quer dizer nao ligado.
const int DISP_RST = 5;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SCL = 8;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SDA = 7;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_INT = -1;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_RST = -1;
// [EdgeWX] Rotacao 0..3 do painel. O comentario em ingles no fim da linha e do gerador.
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
// [EdgeWX] Flag de IPS ou big-endian. O construtor DSI nao le isIPS. O comentario Come display e typo do gerador.
const bool isIPS = false; // Come display can use this as bigEndian flag

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0();
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions

// [EdgeWX] GPIO do backlight, quando a placa tem um pino direto.
#define DISP_LED 23
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_ESP32DSIPanel *bus = nullptr;
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_DSI_Display *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] CameraMipi cameraMipi no ponto (135, 35), tela 0. Nome e posicao vieram do editor.
CameraMipi cameraMipi(135, 35, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCameraMipi = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

// [EdgeWX] Marca do gerador imediatamente antes do setup.
// linha de camera
// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){

    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [EdgeWX] Comentario do gerador abrindo o bloco da camera.
    // Setup camera
    // [EdgeWX] Guarda rotacao e espelho. O PPA aplica isso ao carregar a foto.
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    // [EdgeWX] Abre o sensor. O sexto argumento e CAM_INT, que o gerador entrega no lugar de pwdn.
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        // [EdgeWX] Contrata o PPA. A miniatura passa a ir por DMA, sem o processador copiar ponto a ponto.
        myDisplay.getCamera().startPpa();
    // [EdgeWX] Ramo falso da condicao de cima.
    }else{
        // [EdgeWX] Avisa na serial quando o sensor nao abre. Sem isso nao ha tarefa de camera.
        Serial.println("Camera not started");
    }
    // [EdgeWX] Cria o barramento MIPI DSI com os tempos deste painel.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] Pulso, porch traseiro e porch dianteiro do HSYNC, na unidade do painel.
    12  /* hsync_pulse_width */,  42  /* hsync_back_porch */,  42  /* hsync_front_porch */,
    // [EdgeWX] Pulso e porches do VSYNC.
    2   /* vsync_pulse_width */, 8   /* vsync_back_porch */,  166 /* vsync_front_porch */,
    // [EdgeWX] Clock DPI do ST7701, 34 MHz.
    34000000 /* prefer_speed */);
    // [EdgeWX] Cria o display DSI preso a esse barramento. auto_flush true entrega o framebuffer ao painel.
    tft = new Arduino_DSI_Display(
    // [EdgeWX] Largura e altura nativas, barramento, rotacao e auto_flush.
    480 /* width default */, 800 /* height default */, bus, rotationScreen, true,
    // [EdgeWX] Pino de reset e tabela de init da Arduino_GFX, com o tamanho em comandos.
    DISP_RST, st7701_init_operations, sizeof(st7701_init_operations) / sizeof(lcd_init_cmd_t));
    // [EdgeWX] Sobe o PHY, aplica a tabela de init e aloca o framebuffer.
    tft->begin();
    // [EdgeWX] Cerca de compilacao.
    #if defined(DISP_LED)
    // [EdgeWX] Backlight como saida.
    pinMode(DISP_LED, OUTPUT);
    // [EdgeWX] Escreve HIGH no backlight.
    digitalWrite(DISP_LED, HIGH);
    // [EdgeWX] Cerca de compilacao.
    #endif
    // [EdgeWX] Entrega o display aos widgets e, se houver camera, ao caminho do preview. O comentario em ingles e do gerador.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Comentario do gerador abrindo o bloco do toque.
    // Setup touch
    // [EdgeWX] Canto do mapa de toque, antes da calibracao.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Aplica o espelhamento definido nas constantes.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Aplica, ou nao, a troca dos eixos.
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o GT911. Os GPIO de dados batem com o SCCB quando a camera divide o barramento.
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.enableTouchLog();
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo individualy, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra a funcao da tela. A tarefa de UI chama esse desenho.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false desliga o watchdog; 3 e o timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

// [EdgeWX] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop(){

    // [EdgeWX] Reset do watchdog comentado. createTask(false) nao liga o WDT.
    //esp_task_wdt_reset();
    // [EdgeWX] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(100);
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
            .width = 213,
            // [EdgeWX] Altura, em pixels.
            .height = 166,
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


