// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. Este sketch nao tem logica de processo escrita a mao.
// [legenda] Painel ST7701, toque GT911, rotacao 1. O loop com random e o placeholder que o gerador deixa para os widgets se mexerem.
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


    // [EdgeWX] Abre a ficha da placa. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7701 - DSI
    * Touch: GT911
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e lido neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; este sketch nao le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura do painel em pixels, usada no toque e como tamanho logico.
const int DISPLAY_W = 800;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 480;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X1 = 800;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y1 = 480;
// [EdgeWX] Eixo do toque: troca X com Y.
const bool TOUCH_SWAP_XY = true;
// [EdgeWX] nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] espelha o eixo Y do toque.
const bool TOUCH_INVERT_Y = true;
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
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
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
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy2_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy3_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle1copy_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1copy_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1copy_cb();

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
// [EdgeWX] HSlider slider1 no ponto (430, 30), tela 0. Nome e posicao vieram do editor.
HSlider slider1(430, 30, 0);
// [EdgeWX] HSlider slider1copy no ponto (430, 95), tela 0. Nome e posicao vieram do editor.
HSlider slider1copy(430, 95, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdHSlider = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
HSlider *arrayHslider[qtdHSlider] = {&slider1, &slider1copy};
// [EdgeWX] RectButton rectbutton1 no ponto (35, 30), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton1(35, 30, 0);
// [EdgeWX] RectButton rectbutton1copy no ponto (115, 30), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton1copy(115, 30, 0);
// [EdgeWX] RectButton rectbutton1copy2 no ponto (200, 30), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton1copy2(200, 30, 0);
// [EdgeWX] RectButton rectbutton1copy3 no ponto (295, 30), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton1copy3(295, 30, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdRectBtn = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton1, &rectbutton1copy, &rectbutton1copy2, &rectbutton1copy3};
// [EdgeWX] SpinBox spinbox1 no ponto (405, 230), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox1(405, 230, 0);
// [EdgeWX] SpinBox spinbox1copy no ponto (590, 230), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox1copy(590, 230, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdSpinbox = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox1, &spinbox1copy};
// [EdgeWX] ToggleButton toggle1 no ponto (455, 180), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle1(455, 180, 0);
// [EdgeWX] ToggleButton toggle1copy no ponto (645, 180), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle1copy(645, 180, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdToggleBtn = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1, &toggle1copy};
// [EdgeWX] VBar vertbar1 no ponto (40, 340), tela 0. Nome e posicao vieram do editor.
VBar vertbar1(40, 340, 0);
// [EdgeWX] VBar vertbar1copy no ponto (115, 340), tela 0. Nome e posicao vieram do editor.
VBar vertbar1copy(115, 340, 0);
// [EdgeWX] VBar vertbar1copy2 no ponto (200, 340), tela 0. Nome e posicao vieram do editor.
VBar vertbar1copy2(200, 340, 0);
// [EdgeWX] VBar vertbar1copy3 no ponto (300, 340), tela 0. Nome e posicao vieram do editor.
VBar vertbar1copy3(300, 340, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdVBar = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
VBar *arrayVbar[qtdVBar] = {&vertbar1, &vertbar1copy, &vertbar1copy2, &vertbar1copy3};
// [EdgeWX] CircularBar circload1 no ponto (460, 390), tela 0. Nome e posicao vieram do editor.
CircularBar circload1(460, 390, 0);
// [EdgeWX] CircularBar circload1copy no ponto (580, 390), tela 0. Nome e posicao vieram do editor.
CircularBar circload1copy(580, 390, 0);
// [EdgeWX] CircularBar circload1copy2 no ponto (705, 385), tela 0. Nome e posicao vieram do editor.
CircularBar circload1copy2(705, 385, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCircBar = 3;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CircularBar *arrayCircularbar[qtdCircBar] = {&circload1, &circload1copy, &circload1copy2};
// [EdgeWX] Thermometer thermometer1 no ponto (35, 141), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1(35, 141, 0);
// [EdgeWX] Thermometer thermometer1copy no ponto (115, 140), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1copy(115, 140, 0);
// [EdgeWX] Thermometer thermometer1copy2 no ponto (200, 140), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1copy2(200, 140, 0);
// [EdgeWX] Thermometer thermometer1copy3 no ponto (300, 140), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1copy3(300, 140, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdThermometer = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer1, &thermometer1copy, &thermometer1copy2, &thermometer1copy3};

// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){

    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [EdgeWX] Cria o barramento MIPI DSI com os tempos deste painel.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] Pulso, porch traseiro e porch dianteiro do HSYNC, na unidade do painel.
    12  /* hsync_pulse_width */,  42  /* hsync_back_porch */,  42  /* hsync_front_porch */,
    // [EdgeWX] Pulso e porches do VSYNC.
    2   /* vsync_pulse_width */, 8   /* vsync_back_porch */,  166 /* vsync_front_porch */,
    // [EdgeWX] Clock preferido do DPI, em Hz.
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

    // [EdgeWX] Toma o mutex de desenho. setValue nao corre junto com a tarefa de UI.
    myDisplay.startCustomDraw();
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    vertbar1.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    vertbar1copy.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    vertbar1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    vertbar1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    circload1.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    circload1copy.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    circload1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    thermometer1.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    thermometer1copy.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    thermometer1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    thermometer1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Devolve o mutex. A tarefa redesenha o que foi marcado.
    myDisplay.finishCustomDraw();
    // [EdgeWX] Espera 2 s. O bloco random acima e o placeholder do gerador.
    delay(2000);
}

// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0(){

    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(15, 325, 365, 139, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(15, 325, 365, 139, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(15, 10, 365, 298, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(15, 10, 365, 298, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(395, 15, 387, 139, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(395, 15, 387, 139, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(395, 165, 387, 139, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(395, 165, 387, 139, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(395, 315, 387, 150, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(395, 315, 387, 150, 10, CFK_BLACK);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e o desta chamada.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Comentario fixo do gerador, inclusive o typo widgtes.
// Configure each widgtes to be used
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets(){

    // [EdgeWX] Struct copiada do editor para este widget.
    HSliderConfig configHSlider0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = slider1_cb,
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 18,
            // [EdgeWX] Largura, em pixels.
            .width = 331,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    slider1.setup(configHSlider0);
    // [EdgeWX] Struct copiada do editor para este widget.
    HSliderConfig configHSlider1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = slider1copy_cb,
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 18,
            // [EdgeWX] Largura, em pixels.
            .width = 331,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    slider1copy.setup(configHSlider1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setHSlider(arrayHslider,qtdHSlider);


    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton0 = {
            // [EdgeWX] Largura, em pixels.
            .width = 36,
            // [EdgeWX] Altura, em pixels.
            .height = 91,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR01,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton1_cb
        };

// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton1.setup(configRectButton0);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton1 = {
            // [EdgeWX] Largura, em pixels.
            .width = 36,
            // [EdgeWX] Altura, em pixels.
            .height = 91,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR02,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton1copy_cb
        };

// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton1copy.setup(configRectButton1);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton2 = {
            // [EdgeWX] Largura, em pixels.
            .width = 36,
            // [EdgeWX] Altura, em pixels.
            .height = 91,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR03,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton1copy2_cb
        };

// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton1copy2.setup(configRectButton2);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton3 = {
            // [EdgeWX] Largura, em pixels.
            .width = 36,
            // [EdgeWX] Altura, em pixels.
            .height = 91,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR04,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton1copy3_cb
        };

// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton1copy3.setup(configRectButton3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);


    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox1_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 174,
            // [EdgeWX] Altura, em pixels.
            .height = 57,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_GREY11,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox1.setup(configSpinBox0);
    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox1copy_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 174,
            // [EdgeWX] Altura, em pixels.
            .height = 57,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_GREY11,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox1copy.setup(configSpinBox1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);


    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle0 = {
            // [EdgeWX] Largura, em pixels.
            .width = 74,
            // [EdgeWX] Altura, em pixels.
            .height = 37,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR14,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle1_cb
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle1.setup(configToggle0);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle1 = {
            // [EdgeWX] Largura, em pixels.
            .width = 74,
            // [EdgeWX] Altura, em pixels.
            .height = 37,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR14,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle1copy_cb
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle1copy.setup(configToggle1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar0 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 0,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 32,
            // [EdgeWX] Altura, em pixels.
            .height = 110,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR01
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vertbar1.setup(configVBar0);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar1 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 0,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 34,
            // [EdgeWX] Altura, em pixels.
            .height = 112,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR02
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vertbar1copy.setup(configVBar1);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar2 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 0,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 34,
            // [EdgeWX] Altura, em pixels.
            .height = 112,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR03
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vertbar1copy2.setup(configVBar2);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar3 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 0,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 34,
            // [EdgeWX] Altura, em pixels.
            .height = 112,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR04
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vertbar1copy3.setup(configVBar3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setVBar(arrayVbar,qtdVBar);


    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar0 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 56,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR27,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_BLACK,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY8,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload1.setup(configCirculaBar0);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar1 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 56,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR29,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_BLACK,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY8,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload1copy.setup(configCirculaBar1);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar2 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 56,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR31,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_BLACK,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY8,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload1copy2.setup(configCirculaBar2);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);


    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer0 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 37,
            // [EdgeWX] Altura, em pixels.
            .height = 143,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR01,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1.setup(configThermometer0);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer1 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 39,
            // [EdgeWX] Altura, em pixels.
            .height = 145,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR02,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1copy.setup(configThermometer1);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer2 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 39,
            // [EdgeWX] Altura, em pixels.
            .height = 145,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1copy2.setup(configThermometer2);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer3 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 39,
            // [EdgeWX] Altura, em pixels.
            .height = 145,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR04,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1copy3.setup(configThermometer3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);


}

// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slider1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = slider1.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slider1copy.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1copy_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = slider1copy.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton1.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton1copy.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton1copy.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton1copy2.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy2_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton1copy2.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton1copy3.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1copy3_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton1copy3.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = spinbox1.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox1copy.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1copy_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = spinbox1copy.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element toggle1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle1_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = toggle1.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element toggle1copy.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle1copy_cb(){

    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = toggle1copy.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
