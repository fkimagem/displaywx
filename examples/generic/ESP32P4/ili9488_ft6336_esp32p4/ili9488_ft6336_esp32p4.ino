// [legenda] [EdgeWX] = export antigo, ainda com SPI compartilhado e o placeholder random. [manual] = refreshScreen dentro do callback do toggle.
// [legenda] ILI9488 18 bit em SPI e toque FT6336. Nao ha camera. A tela logica gira (rotation 1) e o mapa de toque troca os eixos.
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Cerca de compilacao.
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
// [EdgeWX] Constante de preprocessador.
#define VSPI FSPI
// [EdgeWX] Cerca de compilacao.
#endif
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] Arduino_GFX pede SPI.h mesmo quando o painel e MIPI e nao usa esse barramento.
#include <SPI.h>
// [EdgeWX] Classes do painel, primitivas e fontes GFX.
#include <Arduino_GFX_Library.h>
// [EdgeWX] Biblioteca: widgets, toque, camera e a tarefa de UI.
#include <displaywx.h>

// [EdgeWX] Flag de cartao SD deixada comentada pelo gerador. Este sketch nao abre SD.
// #define DFK_SD
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e lido neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; este sketch nao le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura do painel em pixels, usada no toque e como tamanho logico.
const int DISPLAY_W = 480;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 320;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_FREQUENCY = 27000000;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] O mapa troca os lados: a tela gira para 480x320 e o toque nasce em 320x480.
const int TOUCH_MAP_X1 = 320;
// [EdgeWX] O mapa troca os lados: a tela gira para 480x320 e o toque nasce em 320x480.
const int TOUCH_MAP_Y1 = 480;
// [EdgeWX] Eixo do toque: troca X com Y.
const bool TOUCH_SWAP_XY = true;
// [EdgeWX] nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] espelha o eixo Y do toque.
const bool TOUCH_INVERT_Y = true;


// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_MOSI = 23;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_MISO = -1;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_SCLK = 26;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_CS = 20;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_DC = 22;
// [EdgeWX] GPIO de reset do painel. -1 quer dizer nao ligado.
const int DISP_RST = 21;

// [EdgeWX] SCL do FT6336. O numero depois de // e uma anotacao manual do GPIO antigo.
const int TOUCH_SCL = 18;//8
// [EdgeWX] SDA do FT6336. O numero depois de // e uma anotacao manual do GPIO antigo.
const int TOUCH_SDA = 17;//7
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_INT = 47;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_RST = 33;


// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0();
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider_cb();

// [EdgeWX] Cerca de compilacao.
#if defined(CONFIG_IDF_TARGET_ESP32S3)
// [EdgeWX] SPI compartilhado do painel. No S3 o gerador escolhe FSPI; nos outros, HSPI.
SPIClass spi_shared(FSPI);
// [EdgeWX] Cerca de compilacao.
#else
// [EdgeWX] SPI compartilhado do painel. No S3 o gerador escolhe FSPI; nos outros, HSPI.
SPIClass spi_shared(HSPI);
// [EdgeWX] Cerca de compilacao.
#endif
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_DataBus *bus = nullptr;
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_GFX *tft = nullptr;
// [EdgeWX] Rotacao 1: a area logica fica deitada, 480x320.
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] HSlider slider no ponto (85, 270), tela 0. Nome e posicao vieram do editor.
HSlider slider(85, 270, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdHSlider = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
HSlider *arrayHslider[qtdHSlider] = {&slider};
// [EdgeWX] ToggleButton toggle no ponto (170, 180), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle(170, 180, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdToggleBtn = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle};
// [EdgeWX] CircularBar circload no ponto (390, 90), tela 0. Nome e posicao vieram do editor.
CircularBar circload(390, 90, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCircBar = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CircularBar *arrayCircularbar[qtdCircBar] = {&circload};

// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){
    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [EdgeWX] SPI compartilhado do painel. No S3 o gerador escolhe FSPI; nos outros, HSPI.
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    // [EdgeWX] Barramento SPI do display, com os GPIO da ficha.
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    // [EdgeWX] Driver do painel SPI. isIPS e a rotacao entram aqui.
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, true);
    // [EdgeWX] Inicializa o painel SPI no clock DISP_FREQUENCY.
    tft->begin(DISP_FREQUENCY);
    // [EdgeWX] Caminho antigo, antes de setDrawObject. O gerador deixou comentado.
    //WidgetBase::objTFT = tft; // Reference to object to draw on screen
    // [EdgeWX] Entrega o display aos widgets e, se houver camera, ao caminho do preview. O comentario em ingles e do gerador.
    myDisplay.setDrawObject(tft);
    // [EdgeWX] Comentario do gerador abrindo o bloco do toque.
    // Setup touch
    // [EdgeWX] Canto do mapa de toque, antes da calibracao.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Aplica o espelhamento definido nas constantes.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Aplica, ou nao, a troca dos eixos.
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o FT6336 no I2C indicado.
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Liga o log de toque. Em varios exports a mesma linha sai comentada; neste ficou ativa.
    myDisplay.enableTouchLog();
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.startAutoClick(); // Start auto click
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo individualy, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Forma antiga de trocar de tela. A linha ativa logo abaixo e myDisplay.loadScreen.
    //WidgetBase::loadScreen = screen0; // Use this line to change between screens
    // [EdgeWX] Registra a funcao da tela. A tarefa de UI chama esse desenho.
    myDisplay.loadScreen(screen0);
    // [EdgeWX] Cria a tarefa de toque e desenho. false desliga o watchdog; 3 e o timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

// [EdgeWX] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop(){
    // [EdgeWX] Placeholder do gerador: sorteia um numero so para o widget se mexer. O comentario em ingles e fixo.
    circload.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Espera 2 s entre um random e o proximo.
    delay(2000);
}

// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0(){
    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_WHITE);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Preenche um circulo estatico.
    tft->fillCircle(65, 45, 40, CFK_COLOR01);
    // [EdgeWX] Contorna um circulo estatico.
    tft->drawCircle(65, 45, 40, CFK_COLOR01);
    // [EdgeWX] Preenche um circulo estatico.
    tft->fillCircle(94, 92, 40, CFK_COLOR11);
    // [EdgeWX] Contorna um circulo estatico.
    tft->drawCircle(94, 92, 40, CFK_COLOR01);
    // [EdgeWX] Preenche um circulo estatico.
    tft->fillCircle(41, 95, 40, CFK_COLOR22);
    // [EdgeWX] Contorna um circulo estatico.
    tft->drawCircle(41, 95, 40, CFK_COLOR01);
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
            .callback = slider_cb,
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 16,
            // [EdgeWX] Largura, em pixels.
            .width = 279,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR03
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    slider.setup(configHSlider0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setHSlider(arrayHslider,qtdHSlider);

    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle0 = {
            // [EdgeWX] Largura, em pixels.
            .width = 94,
            // [EdgeWX] Altura, em pixels.
            .height = 47,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR01,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle_cb
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle.setup(configToggle0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar0 = {
            // [EdgeWX] Raio, em pixels.
            .radius = 80,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR19,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_WHITE,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload.setup(configCirculaBar0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);

}

// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slider.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = slider.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element toggle.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = toggle.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for toggle is: ");Serial.println(myValue);
    // [EdgeWX] Condicao ou repeticao.
    if(myValue){
      // [manual] Pede para screen0 rodar de novo quando o toggle liga. O gerador nao coloca esta chamada.
      myDisplay.refreshScreen();  
    }
}
