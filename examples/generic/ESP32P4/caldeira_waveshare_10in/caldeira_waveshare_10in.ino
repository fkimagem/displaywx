// [legenda] [EdgeWX] = tela e widgets sairam do editor. [manual] = senoides e o timer que atualiza os medidores.
// [legenda] JD9365 com rotacao 1 (logico 1280x800) e toque GSL3680 no mapa nativo 800x1280.
// [EdgeWX] Header do PNG iPinionpng.h, exportado pelo editor junto com os pixels.
#include "iPinionpng.h"
// [EdgeWX] Header do PNG iStoppng.h, exportado pelo editor junto com os pixels.
#include "iStoppng.h"
// [EdgeWX] Header do PNG iRightpng.h, exportado pelo editor junto com os pixels.
#include "iRightpng.h"
// [EdgeWX] Header do PNG iHelppng.h, exportado pelo editor junto com os pixels.
#include "iHelppng.h"
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
// [EdgeWX] Flag de cartao SD deixada comentada pelo gerador. Este sketch nao abre SD.
// #define DFK_SD
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e lido neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; este sketch nao le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura logica depois da rotacao 1. O construtor DSI ainda nasce 800x1280.
const int DISPLAY_W = 1280;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 800;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int DISP_FREQUENCY = 27000000;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] O GSL3680 continua no mapa nativo, sem a rotacao logica.
const int TOUCH_MAP_X1 = 800;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] O GSL3680 continua no mapa nativo, sem a rotacao logica.
const int TOUCH_MAP_Y1 = 1280;
// [EdgeWX] Eixo do toque: nao troca X com Y.
const bool TOUCH_SWAP_XY = false;
// [EdgeWX] nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] nao espelha o eixo Y do toque.
const bool TOUCH_INVERT_Y = false;
// [EdgeWX] GPIO de reset do painel. -1 quer dizer nao ligado.
const int DISP_RST = 27;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SCL = 8;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SDA = 7;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_INT = 21;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_RST = 22;
// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0();
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets();
// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor2_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor3_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor4_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbgas_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbresf_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbsuperresf_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void pinionpng_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void stoppng_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rightpng_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void helppng_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox2_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox3_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void callbackOfGroup_grupo1();
// [EdgeWX] GPIO do backlight, quando a placa tem um pino direto.
#define DISP_LED 23
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_ESP32DSIPanel *bus = nullptr;
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_DSI_Display *tft = nullptr;
// [EdgeWX] Rotacao 0..3 do painel. O comentario em ingles no fim da linha e do gerador.
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] CheckBox cbgas no ponto (850, 320), tela 0. Nome e posicao vieram do editor.
CheckBox cbgas(850, 320, 0);
// [EdgeWX] CheckBox cbresf no ponto (850, 355), tela 0. Nome e posicao vieram do editor.
CheckBox cbresf(850, 355, 0);
// [EdgeWX] CheckBox cbsuperresf no ponto (850, 390), tela 0. Nome e posicao vieram do editor.
CheckBox cbsuperresf(850, 390, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCheckbox = 3;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CheckBox *arrayCheckbox[qtdCheckbox] = {&cbgas, &cbresf, &cbsuperresf};
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool check0_val = false; // Global variable that stores the value of the widget cbgas
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool check1_val = false; // Global variable that stores the value of the widget cbresf
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool check2_val = false; // Global variable that stores the value of the widget cbsuperresf
// [EdgeWX] GaugeSuper gauge no ponto (249, 300), tela 0. Nome e posicao vieram do editor.
GaugeSuper gauge(249, 300, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdGauge = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
GaugeSuper *arrayGauge[qtdGauge] = {&gauge};
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdIntervalG0 = 4;
// [EdgeWX] Variavel deste sketch.
int range0[qtdIntervalG0] = {0,25,50,75};
// [EdgeWX] Tabela de cores gerada pelo editor.
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR12,CFK_COLOR06,CFK_COLOR03,CFK_COLOR01};
// [EdgeWX] HSlider slider no ponto (95, 438), tela 0. Nome e posicao vieram do editor.
HSlider slider(95, 438, 0);
// [EdgeWX] HSlider slider1 no ponto (34, 358), tela 0. Nome e posicao vieram do editor.
HSlider slider1(34, 358, 0);
// [EdgeWX] Variavel deste sketch.
int slider_val = 0;
// [EdgeWX] Variavel deste sketch.
int slider1_val = 0;
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdHSlider = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
HSlider *arrayHslider[qtdHSlider] = {&slider, &slider1};
// [EdgeWX] Label line1 no ponto (720, 620), tela 0. Nome e posicao vieram do editor.
Label line1(720, 620, 0);
// [EdgeWX] Label line2 no ponto (885, 620), tela 0. Nome e posicao vieram do editor.
Label line2(885, 620, 0);
// [EdgeWX] Label text no ponto (405, 425), tela 0. Nome e posicao vieram do editor.
Label text(405, 425, 0);
// [EdgeWX] Label text3 no ponto (405, 345), tela 0. Nome e posicao vieram do editor.
Label text3(405, 345, 0);
// [EdgeWX] Label term1 no ponto (545, 460), tela 0. Nome e posicao vieram do editor.
Label term1(545, 460, 0);
// [EdgeWX] Label term15 no ponto (545, 585), tela 0. Nome e posicao vieram do editor.
Label term15(545, 585, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLabel = 6;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Label *arrayLabel[qtdLabel] = {&line1, &line2, &text, &text3, &term1, &term15};
// [EdgeWX] Led led no ponto (510, 185), tela 0. Nome e posicao vieram do editor.
Led led(510, 185, 0);
// [EdgeWX] Led led1 no ponto (510, 225), tela 0. Nome e posicao vieram do editor.
Led led1(510, 225, 0);
// [EdgeWX] Led led2 no ponto (510, 265), tela 0. Nome e posicao vieram do editor.
Led led2(510, 265, 0);
// [EdgeWX] Led led3 no ponto (510, 310), tela 0. Nome e posicao vieram do editor.
Led led3(510, 310, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLed = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Led *arrayLed[qtdLed] = {&led, &led1, &led2, &led3};
// [EdgeWX] LineChart grafico no ponto (643, 470), tela 0. Nome e posicao vieram do editor.
LineChart grafico(643, 470, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLineChart = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
LineChart *arrayLinechart[qtdLineChart] = {&grafico};
// [EdgeWX] Tabela que o editor gerou para faixas, cores ou botoes de radio.
Label *seriesGrafico0[2] = {&line1,&line2};
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLinesChart0 = 2;
// [EdgeWX] Tabela de cores gerada pelo editor.
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR04,CFK_COLOR15};
// [EdgeWX] Grupo de radio de nome grupo1. O construtor recebe o indice do grupo.
RadioGroup grupo1(0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdRadioGroup = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1};
// [EdgeWX] Tabela que o editor gerou para faixas, cores ou botoes de radio.
radio_t radiosOfGroup_grupo1[3] = {{650, 365, CFK_COLOR01, 2},{650, 330, CFK_COLOR01, 1 },{650, 400, CFK_COLOR01, 3}};
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
uint16_t radiog1 = 2; // Global variable that stores the value of the widget grupo1
// [EdgeWX] RectButton rectbutton no ponto (30, 420), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton(30, 420, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdRectBtn = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton};
// [EdgeWX] SpinBox spinbox no ponto (855, 710), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox(855, 710, 0);
// [EdgeWX] SpinBox spinbox1 no ponto (500, 710), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox1(500, 710, 0);
// [EdgeWX] SpinBox spinbox2 no ponto (680, 710), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox2(680, 710, 0);
// [EdgeWX] SpinBox spinbox3 no ponto (855, 710), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox3(855, 710, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdSpinbox = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox, &spinbox1, &spinbox2, &spinbox3};
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
int spin1_val = 0; // Global variable that stores the value of the widget spinbox1
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
int spin2_val = 0; // Global variable that stores the value of the widget spinbox2
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
int spin3_val = 0; // Global variable that stores the value of the widget spinbox3
// [EdgeWX] ToggleButton motor no ponto (35, 720), tela 0. Nome e posicao vieram do editor.
ToggleButton motor(35, 720, 0);
// [EdgeWX] ToggleButton motor2 no ponto (145, 720), tela 0. Nome e posicao vieram do editor.
ToggleButton motor2(145, 720, 0);
// [EdgeWX] ToggleButton motor3 no ponto (260, 720), tela 0. Nome e posicao vieram do editor.
ToggleButton motor3(260, 720, 0);
// [EdgeWX] ToggleButton motor4 no ponto (375, 720), tela 0. Nome e posicao vieram do editor.
ToggleButton motor4(375, 720, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdToggleBtn = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&motor, &motor2, &motor3, &motor4};
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool  switch1_val = false; // Global variable that stores the value of the widget motor
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool  switch2_val = false; // Global variable that stores the value of the widget motor2
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool  switch3_val = false; // Global variable that stores the value of the widget motor3
// [EdgeWX] Espelho do widget. O comentario em ingles no fim da linha e fixo do gerador.
bool  switch4_val = false; // Global variable that stores the value of the widget motor4
// [EdgeWX] VAnalog barramotor no ponto (60, 500), tela 0. Nome e posicao vieram do editor.
VAnalog barramotor(60, 500, 0);
// [EdgeWX] VAnalog barramotor1 no ponto (285, 500), tela 0. Nome e posicao vieram do editor.
VAnalog barramotor1(285, 500, 0);
// [EdgeWX] VAnalog barramotor2 no ponto (175, 500), tela 0. Nome e posicao vieram do editor.
VAnalog barramotor2(175, 500, 0);
// [EdgeWX] VAnalog barramotor3 no ponto (395, 500), tela 0. Nome e posicao vieram do editor.
VAnalog barramotor3(395, 500, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdVAnalog = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
VAnalog *arrayVanalog[qtdVAnalog] = {&barramotor, &barramotor1, &barramotor2, &barramotor3};
// [EdgeWX] VBar vbar no ponto (865, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar(865, 70, 0);
// [EdgeWX] VBar vbar1 no ponto (825, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar1(825, 70, 0);
// [EdgeWX] VBar vbar2 no ponto (785, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar2(785, 70, 0);
// [EdgeWX] VBar vbar3 no ponto (750, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar3(750, 70, 0);
// [EdgeWX] VBar vbar4 no ponto (715, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar4(715, 70, 0);
// [EdgeWX] VBar vbar5 no ponto (680, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar5(680, 70, 0);
// [EdgeWX] VBar vbar6 no ponto (645, 70), tela 0. Nome e posicao vieram do editor.
VBar vbar6(645, 70, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdVBar = 7;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
VBar *arrayVbar[qtdVBar] = {&vbar, &vbar1, &vbar2, &vbar3, &vbar4, &vbar5, &vbar6};
// [EdgeWX] Image pinionpng no ponto (1230, 715), tela 0. Nome e posicao vieram do editor.
Image pinionpng(1230, 715, 0);
// [EdgeWX] Image stoppng no ponto (1120, 715), tela 0. Nome e posicao vieram do editor.
Image stoppng(1120, 715, 0);
// [EdgeWX] Image rightpng no ponto (1175, 715), tela 0. Nome e posicao vieram do editor.
Image rightpng(1175, 715, 0);
// [EdgeWX] Image helppng no ponto (1065, 715), tela 0. Nome e posicao vieram do editor.
Image helppng(1065, 715, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdImagem = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Image *arrayImagem[qtdImagem] = {&pinionpng, &stoppng, &rightpng, &helppng};
// [EdgeWX] CircularBar load no ponto (1165, 145), tela 0. Nome e posicao vieram do editor.
CircularBar load(1165, 145, 0);
// [EdgeWX] CircularBar load1 no ponto (1164, 315), tela 0. Nome e posicao vieram do editor.
CircularBar load1(1164, 315, 0);
// [EdgeWX] CircularBar load2 no ponto (1165, 480), tela 0. Nome e posicao vieram do editor.
CircularBar load2(1165, 480, 0);
// [EdgeWX] CircularBar load3 no ponto (1165, 650), tela 0. Nome e posicao vieram do editor.
CircularBar load3(1165, 650, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCircBar = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CircularBar *arrayCircularbar[qtdCircBar] = {&load, &load1, &load2, &load3};
// [EdgeWX] Thermometer thermometer no ponto (500, 415), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer(500, 415, 0);
// [EdgeWX] Thermometer thermometer1 no ponto (500, 535), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1(500, 535, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdThermometer = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer, &thermometer1};


// [manual] Comentario escrito a mao. A contagem real deste arquivo pode nao ser 7.
// create 7 'int count' with constexpr offset between 0 and 6
// [manual] Afastamento, em graus, entre um seno e o proximo.
constexpr int offsetCount = 20;
// [manual] Variavel deste sketch.
int countGeral = 0;
// [manual] Variavel deste sketch.
const int countSize = 7;
// [manual] Variavel deste sketch.
float counters[countSize] = {
    // [manual] Sete amostras, uma por medidor.
    0,0,0,0,0,0,0
};


// [manual] Recalcula os senos. countSize diz quantos.
void increaseCounters(){
    // [manual] Condicao ou repeticao.
    for (size_t i = 0; i < countSize; i++)
    {
        // [manual] Seno deslocado. Cada indice anda offsetCount graus na frente.
        counters[i] = sin((countGeral + (offsetCount * i)) * DEGTORAD);
        // [manual] Traz o seno de -1..1 para a faixa 0..100.
        counters[i] = counters[i] * 50 + 50;// De 0 a 100
    }
    // [manual] Anda 1 grau a cada ciclo.
    countGeral++;
}

// [manual] Variavel deste sketch.
bool flagAtualizarTela = false;

// [manual] Variavel deste sketch.
TimerHandle_t despertador;
// [manual] O timer so levanta o flag. Quem desenha e o loop.
void vTimerCallback(TimerHandle_t xTimer)
{
    // [manual] Condicao ou repeticao.
    if(!flagAtualizarTela){
        // [manual] Pede uma atualizacao. Se a anterior nao rodou, nao empilha outra.
        flagAtualizarTela = true;      
    }
  
}

// [manual] Cria o timer periodico de 200 ms.
void iniciarDespertador(){
    // [manual] Timer de software do FreeRTOS.
    despertador = xTimerCreate(
    // [manual] Nome do timer, visivel so em debug.
    "Timer200ms",                 // nome do timer (opcional)
    // [manual] Periodo de 200 ms. O comentario ao lado diz 100; o numero vale.
    pdMS_TO_TICKS(200),           // período em ticks (100ms)
    // [manual] O timer repete sozinho.
    pdTRUE,                       // pdTRUE = auto-reload (infinito)
    // [manual] Sem identificador extra.
    (void*)0,                     // ID opcional do timer
    // [manual] Funcao chamada a cada disparo.
    vTimerCallback                // função de callback
  );

  // [manual] Condicao ou repeticao.
  if (despertador != NULL) {
    // [manual] Comentario escrito a mao.
    // Inicia o timer (sem bloqueio)
    // [manual] Comeca a contar sem bloquear o setup.
    xTimerStart(despertador, 0);
    // [manual] Mensagem na serial.
    Serial.println("Timer iniciado!");
  // [manual] Ramo falso da condicao de cima.
  } else {
    // [manual] Mensagem na serial.
    Serial.println("Falha ao criar o timer!");
  }
}

// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){
    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [EdgeWX] Cria o barramento MIPI DSI com os tempos deste painel.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] Pulso, porch traseiro e porch dianteiro do HSYNC, na unidade do painel.
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    // [EdgeWX] Pulso e porches do VSYNC.
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    // [EdgeWX] Clock preferido do DPI, em Hz.
    60000000 /* prefer_speed */);
    // [EdgeWX] Cria o display DSI preso a esse barramento. auto_flush true entrega o framebuffer ao painel.
    tft = new Arduino_DSI_Display(
    // [EdgeWX] Resolucao nativa do JD9365. O segundo argumento numerico 1 e a rotacao, nao rotationScreen.
    800 /* width default */, 1280 /* height default */, bus, 1, true,
    // [EdgeWX] Pino de reset e tabela de init da Arduino_GFX, com o tamanho em comandos.
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
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
    // [EdgeWX] Sobe o GSL3680 deste painel.
    myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.enableTouchLog();
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.setupAutoClick(2000, 100, 100); // Setup auto click
    // [EdgeWX] Linha de codigo desligada. Nao executa.
    //myDisplay.startAutoClick(); // Start auto click
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo individualy, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra a funcao da tela. A tarefa de UI chama esse desenho.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false desliga o watchdog; 3 e o timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    // [manual] Arma o timer de 200 ms.
    iniciarDespertador();
}
// [manual] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop(){
    // [manual] Condicao ou repeticao.
    if(flagAtualizarTela){
        // [manual] Toma o mutex de desenho. setValue nao corre junto com a tarefa de UI.
        myDisplay.startCustomDraw(); // Bloqueia a task do displaywx de desenhar
        // [manual] Atualiza o valor. A tarefa desenha depois.
        gauge.setValue(counters[0]);
        // [manual] O label mostra o valor do slider principal.
        text.setTextInt(slider_val);
        // [manual] O outro label mostra o segundo slider.
        text3.setTextInt(slider1_val);
        // [manual] Acende a linha 1 quando o seno passa de 50.
        led.setState(counters[0] > 50);
        // [manual] Linha 2, mesmo limiar.
        led1.setState(counters[1] > 50);
        // [manual] Linha 3, mesmo limiar.
        led2.setState(counters[2] > 50);
        // [manual] Linha 4, mesmo limiar.
        led3.setState(counters[3] > 50);
        // [manual] Empurra uma amostra na serie do grafico.
        grafico.push(0, counters[0]);
        // [manual] Empurra uma amostra na serie do grafico.
        grafico.push(1, counters[2]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        barramotor.setValue(counters[0], true); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        barramotor1.setValue(counters[2], true);
        // [manual] Atualiza o valor. A tarefa desenha depois.
        barramotor2.setValue(counters[4], true); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        barramotor3.setValue(counters[6], true); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar.setValue(counters[0]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar1.setValue(counters[1]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar2.setValue(counters[2]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar3.setValue(counters[3]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar4.setValue(counters[4]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar5.setValue(counters[5]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        vbar6.setValue(counters[6]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        load.setValue(counters[3]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        load1.setValue(counters[4]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        load2.setValue(counters[5]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        load3.setValue(counters[6]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        thermometer.setValue(counters[2]); 
        // [manual] Atualiza o valor. A tarefa desenha depois.
        thermometer1.setValue(counters[3]); 
        // [manual] Devolve o mutex. A tarefa redesenha o que foi marcado.
        myDisplay.finishCustomDraw();// Libera a task do displaywx para desenha
        // [manual] Este ciclo ja desenhou. O proximo espera outro estouro do timer.
        flagAtualizarTela = false;
        // [manual] Avanca os senos para a proxima amostra.
        increaseCounters();
    }
    // [manual] Folga de 1 ms. O redesenho espera o flag do timer.
    delay(1);
}
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0(){
    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_GREY5);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(630, 430, 387, 221, 6, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(485, 375, 134, 282, 6, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(20, 108, 454, 208, 6, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(17, 405, 457, 73, 6, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(15, 487, 461, 286, 6, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(484, 661, 541, 115, 6, CFK_GREY3);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(630, 280, 187, 143, 10, CFK_GREY3);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(835, 280, 187, 143, 10, CFK_GREY3);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(628, 25, 394, 241, 10, CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(1047, 25, 3, 750, CFK_GREY3);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an ponteiroAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an verticalAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an verticalAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an verticalAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 70, 20, 20, CFK_COLOR25);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 70, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 95, 20, 20, CFK_COLOR22);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 95, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 120, 20, 20, CFK_COLOR19);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 120, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 145, 20, 20, CFK_COLOR12);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 145, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 170, 20, 20, CFK_COLOR06);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 170, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 195, 20, 20, CFK_COLOR03);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 195, 20, 20, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(925, 220, 20, 20, CFK_COLOR01);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(925, 220, 20, 20, CFK_BLACK);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an verticalAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an radiobutton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an radiobutton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an radiobutton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an checkbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an checkbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an checkbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an grafico
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an imagem
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an imagem
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an imagem
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an imagem
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Ajuda", 1070, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Parar", 1125, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Prox.", 1180, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Config", 1230, 761, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular5pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Temp Caldeira 4", 1165, 566, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Temp Caldeira 3", 1165, 397, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Temp Caldeira 2", 1160, 236, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Temp Caldeira 1", 1165, 61, TC_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 1", 950, 76, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 2", 950, 101, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 3", 950, 126, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 4", 950, 151, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 5", 950, 176, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 6", 950, 201, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media 7", 950, 226, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Media nos dutos", 745, 37, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoBold10pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Vistoria", 670, 396, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Plena", 670, 361, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Monitoramento", 670, 326, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Modo", 700, 291, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Valvulas ativas", 880, 291, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Gas", 880, 326, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Resfriamento", 880, 361, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Super refriamento", 880, 396, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular7pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(490, 667, 529, 102, 6, CFK_GREY5);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(22, 667, 447, 102, 6, CFK_GREY5);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Vazao 1", 550, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Vazao 2", 725, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Vazao 3", 905, 681, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Motor 1", 50, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Motor 2", 160, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Motor 3", 275, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Motor 4", 390, 686, TL_DATUM, CFK_COLOR06, CFK_GREY5, &RobotoRegular8pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(18, 325, 456, 73, 6, CFK_GREY3);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Duto principal", 190, 127, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoBold10pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Sistema de controle", 260, 19, TC_DATUM, CFK_COLOR14, CFK_GREY5, &RobotoRegular20pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Caldeiras", 250, 68, TC_DATUM, CFK_COLOR14, CFK_GREY5, &RobotoRegular15pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Fluidos", 520, 387, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Consumo (kW)", 770, 442, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(485, 106, 134, 257, 6, CFK_GREY3);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Esteiras", 515, 122, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an led
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an led
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an led
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an led
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Linha 1", 535, 176, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Linha 2", 535, 216, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Linha 3", 535, 256, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Linha 4", 535, 301, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectRange
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Potencia caldeira", 160, 331, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Potencia sub caldeira", 165, 411, TL_DATUM, CFK_COLOR06, CFK_GREY3, &RobotoRegular8pt7b);
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e o desta chamada.
    myDisplay.drawWidgetsOnScreen(0);
}
// [EdgeWX] Comentario fixo do gerador, inclusive o typo widgtes.
// Configure each widgtes to be used
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets(){
    // [EdgeWX] Struct copiada do editor para este widget.
    CheckBoxConfig configCheckbox0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = cbgas_cb,
            // [EdgeWX] Espessura do traco do checkbox.
            .weight = CheckBoxWeight::MEDIUM,
            // [EdgeWX] Lado do checkbox, em pixels.
            .size = 21,
            // [EdgeWX] Cor do checkbox marcado.
            .checkedColor = CFK_COLOR17,
            // [EdgeWX] Cor do checkbox vazio.
            .uncheckedColor = CFK_GREY10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    cbgas.setup(configCheckbox0);
    // [EdgeWX] Struct copiada do editor para este widget.
    CheckBoxConfig configCheckbox1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = cbresf_cb,
            // [EdgeWX] Espessura do traco do checkbox.
            .weight = CheckBoxWeight::MEDIUM,
            // [EdgeWX] Lado do checkbox, em pixels.
            .size = 21,
            // [EdgeWX] Cor do checkbox marcado.
            .checkedColor = CFK_COLOR19,
            // [EdgeWX] Cor do checkbox vazio.
            .uncheckedColor = CFK_GREY10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    cbresf.setup(configCheckbox1);
    // [EdgeWX] Struct copiada do editor para este widget.
    CheckBoxConfig configCheckbox2 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = cbsuperresf_cb,
            // [EdgeWX] Espessura do traco do checkbox.
            .weight = CheckBoxWeight::MEDIUM,
            // [EdgeWX] Lado do checkbox, em pixels.
            .size = 21,
            // [EdgeWX] Cor do checkbox marcado.
            .checkedColor = CFK_COLOR21,
            // [EdgeWX] Cor do checkbox vazio.
            .uncheckedColor = CFK_GREY10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    cbsuperresf.setup(configCheckbox2);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);

    // [EdgeWX] Struct copiada do editor para este widget.
    GaugeConfig configGauge0 = {
            // [EdgeWX] Titulo desenhado no widget.
            .title = "Pressao",
            // [EdgeWX] Limites das faixas do medidor.
            .intervals = range0,
            // [EdgeWX] Uma cor por faixa.
            .colors = colors0,
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoBold10pt7b,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 428,
            // [EdgeWX] Altura, em pixels.
            .height = 140,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_GREY5,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY4,
            // [EdgeWX] Cor do titulo.
            .titleColor = CFK_WHITE,
            // [EdgeWX] Cor do ponteiro.
            .needleColor = CFK_RED,
            // [EdgeWX] Cor dos tracinhos da escala.
            .markersColor = CFK_BLACK,
            // [EdgeWX] Quantidade de faixas coloridas.
            .amountIntervals = qtdIntervalG0,
            // [EdgeWX] true escreve os numeros das faixas.
            .showLabels = true
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    gauge.setup(configGauge0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setGauge(arrayGauge,qtdGauge);

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
            .radius = 12,
            // [EdgeWX] Largura, em pixels.
            .width = 291,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR04,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    slider.setup(configHSlider0);
    // [EdgeWX] Struct copiada do editor para este widget.
    HSliderConfig configHSlider1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = slider1_cb,
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 12,
            // [EdgeWX] Largura, em pixels.
            .width = 352,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR30,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    slider1.setup(configHSlider1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setHSlider(arrayHslider,qtdHSlider);

    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel0 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Line 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "Motor A: ",
            // [EdgeWX] Texto depois do valor.
            .suffix = "",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR04,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    line1.setup(configLabel0);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel1 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Motor B ",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "Motor B:",
            // [EdgeWX] Texto depois do valor.
            .suffix = "",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR15,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    line2.setup(configLabel1);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel2 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "50",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular15pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_WHITE,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    text.setup(configLabel2);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel3 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "100",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular15pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_WHITE,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    text3.setup(configLabel3);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel4 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular8pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term1.setup(configLabel4);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel5 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular8pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR20,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term15.setup(configLabel5);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setLabel(arrayLabel,qtdLabel);

    // [EdgeWX] Struct copiada do editor para este widget.
    LedConfig configLed0 = {
            // [EdgeWX] Raio, em pixels.
            .radius = 16,
            // [EdgeWX] Cor do LED aceso.
            .colorOn = CFK_COLOR11,
            // [EdgeWX] Cor do LED apagado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] false: o LED nasce apagado.
            .initialState = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    led.setup(configLed0);
    // [EdgeWX] Struct copiada do editor para este widget.
    LedConfig configLed1 = {
            // [EdgeWX] Raio, em pixels.
            .radius = 16,
            // [EdgeWX] Cor do LED aceso.
            .colorOn = CFK_COLOR11,
            // [EdgeWX] Cor do LED apagado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] false: o LED nasce apagado.
            .initialState = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    led1.setup(configLed1);
    // [EdgeWX] Struct copiada do editor para este widget.
    LedConfig configLed2 = {
            // [EdgeWX] Raio, em pixels.
            .radius = 16,
            // [EdgeWX] Cor do LED aceso.
            .colorOn = CFK_COLOR11,
            // [EdgeWX] Cor do LED apagado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] false: o LED nasce apagado.
            .initialState = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    led2.setup(configLed2);
    // [EdgeWX] Struct copiada do editor para este widget.
    LedConfig configLed3 = {
            // [EdgeWX] Raio, em pixels.
            .radius = 16,
            // [EdgeWX] Cor do LED aceso.
            .colorOn = CFK_COLOR11,
            // [EdgeWX] Cor do LED apagado.
            .colorOff = CFK_GREY3,
            // [EdgeWX] false: o LED nasce apagado.
            .initialState = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    led3.setup(configLed3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setLed(arrayLed,qtdLed);

    // [EdgeWX] Struct copiada do editor para este widget.
    LineChartConfig configLineChart0 = {
            // [EdgeWX] Uma cor por serie.
            .colorsSeries = colorsChart0,
            // [EdgeWX] Legendas das series. nullptr esconde.
            .subtitles = seriesGrafico0,
            // [EdgeWX] Fonte dos numeros do grafico.
            .font = &RobotoRegular5pt7b,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 360,
            // [EdgeWX] Altura, em pixels.
            .height = 137,
            // [EdgeWX] Cor da grade.
            .gridColor = CFK_GREY4,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Linhas horizontais da grade.
            .verticalDivision = 5,
            // [EdgeWX] Quantos pontos guardar. SHOW_ALL nao descarta.
            .maxPointsAmount = LineChart::SHOW_ALL,
            // [EdgeWX] Quantidade de linhas no grafico.
            .amountSeries = qtdLinesChart0,
            // [EdgeWX] true deixa o grafico amostrar fora deste sketch.
            .workInBackground = false,
            // [EdgeWX] true desenha a linha do zero.
            .showZeroLine = true,
            // [EdgeWX] true engrossa a linha da serie.
            .boldLine = false,
            // [EdgeWX] true marca cada amostra.
            .showDots = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    grafico.setup(configLineChart0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);

    // [EdgeWX] Struct copiada do editor para este widget.
    RadioGroupConfig configRadioGroup0 = {
            // [EdgeWX] Radios do grupo: x, y, cor e id.
            .buttons = radiosOfGroup_grupo1,
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = callbackOfGroup_grupo1,
            // [EdgeWX] Raio, em pixels.
            .radius = 10,
            // [EdgeWX] Indice deste grupo.
            .group = 1,
            // [EdgeWX] Quantos radios o grupo tem.
            .amount = 3,
            // [EdgeWX] Id que nasce selecionado.
            .defaultClickedId = 2
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    grupo1.setup(configRadioGroup0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);

    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 51,
            // [EdgeWX] Altura, em pixels.
            .height = 39,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR04
        };
// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton.setup(configRectButton0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);

    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 152,
            // [EdgeWX] Altura, em pixels.
            .height = 51,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR20,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox.setup(configSpinBox0);
    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox1_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 152,
            // [EdgeWX] Altura, em pixels.
            .height = 51,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR08,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox1.setup(configSpinBox1);
    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox2 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox2_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 152,
            // [EdgeWX] Altura, em pixels.
            .height = 51,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR08,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox2.setup(configSpinBox2);
    // [EdgeWX] Struct copiada do editor para este widget.
    SpinBoxConfig configSpinBox3 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = spinbox3_cb,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Valor inicial.
            .startValue = 50,
            // [EdgeWX] Largura, em pixels.
            .width = 152,
            // [EdgeWX] Altura, em pixels.
            .height = 51,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR08,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox3.setup(configSpinBox3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);

    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = motor_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 83,
            // [EdgeWX] Altura, em pixels.
            .height = 42,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR02
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    motor.setup(configToggle1);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle2 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = motor2_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 83,
            // [EdgeWX] Altura, em pixels.
            .height = 42,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR02
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    motor2.setup(configToggle2);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle3 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = motor3_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 83,
            // [EdgeWX] Altura, em pixels.
            .height = 42,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR02
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    motor3.setup(configToggle3);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle4 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = motor4_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 83,
            // [EdgeWX] Altura, em pixels.
            .height = 42,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR02
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    motor4.setup(configToggle4);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalAnalogConfig configVAnalog0 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 41,
            // [EdgeWX] Altura, em pixels.
            .height = 144,
            // [EdgeWX] Cor da seta da escala.
            .arrowColor = CFK_COLOR01,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Quantidade de marcas da escala.
            .steps = 10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    barramotor.setup(configVAnalog0);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalAnalogConfig configVAnalog1 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 43,
            // [EdgeWX] Altura, em pixels.
            .height = 146,
            // [EdgeWX] Cor da seta da escala.
            .arrowColor = CFK_COLOR01,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Quantidade de marcas da escala.
            .steps = 10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    barramotor1.setup(configVAnalog1);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalAnalogConfig configVAnalog2 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 43,
            // [EdgeWX] Altura, em pixels.
            .height = 146,
            // [EdgeWX] Cor da seta da escala.
            .arrowColor = CFK_COLOR01,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Quantidade de marcas da escala.
            .steps = 10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    barramotor2.setup(configVAnalog2);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalAnalogConfig configVAnalog3 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 45,
            // [EdgeWX] Altura, em pixels.
            .height = 148,
            // [EdgeWX] Cor da seta da escala.
            .arrowColor = CFK_COLOR01,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_WHITE,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Quantidade de marcas da escala.
            .steps = 10
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    barramotor3.setup(configVAnalog3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog);

    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar0 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 22,
            // [EdgeWX] Altura, em pixels.
            .height = 174,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR01
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar.setup(configVBar0);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar1 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 175,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR03
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar1.setup(configVBar1);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar2 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 175,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR06
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar2.setup(configVBar2);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar3 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 174,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR12
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar3.setup(configVBar3);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar4 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 176,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR19
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar4.setup(configVBar4);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar5 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 176,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR22
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar5.setup(configVBar5);
    // [EdgeWX] Struct copiada do editor para este widget.
    VerticalBarConfig configVBar6 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = nullptr,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio do canto da barra. 0 deixa reto.
            .round = 6,
            // [EdgeWX] VERTICAL ou HORIZONTAL.
            .orientation = Orientation::VERTICAL,
            // [EdgeWX] Largura, em pixels.
            .width = 21,
            // [EdgeWX] Altura, em pixels.
            .height = 177,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR25
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    vbar6.setup(configVBar6);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setVBar(arrayVbar,qtdVBar);

    // [EdgeWX] Struct copiada do editor para este widget.
    ImageFromPixelsConfig configImage0 = {
            // [EdgeWX] Bitmap embutido no header que o editor gerou.
            .pixels = iPinionpngPixels,
            // [EdgeWX] Mascara de transparencia desse PNG.
            .maskAlpha = iPinionpngMask,
            // [EdgeWX] Callback de toque da imagem.
            .cb = pinionpng_cb,
            // [EdgeWX] Rotacao da imagem, em graus.
            .angle = 0,
            // [EdgeWX] Largura, em pixels.
            .width = iPinionpngW,
            // [EdgeWX] Altura, em pixels.
            .height = iPinionpngH,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5
        };
    // [EdgeWX] Monta a imagem com o bitmap do header exportado.
    pinionpng.setupFromPixels(configImage0);
    // [EdgeWX] Struct copiada do editor para este widget.
    ImageFromPixelsConfig configImage1 = {
            // [EdgeWX] Bitmap embutido no header que o editor gerou.
            .pixels = iStoppngPixels,
            // [EdgeWX] Mascara de transparencia desse PNG.
            .maskAlpha = iStoppngMask,
            // [EdgeWX] Callback de toque da imagem.
            .cb = stoppng_cb,
            // [EdgeWX] Rotacao da imagem, em graus.
            .angle = 0,
            // [EdgeWX] Largura, em pixels.
            .width = iStoppngW,
            // [EdgeWX] Altura, em pixels.
            .height = iStoppngH,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5
        };
    // [EdgeWX] Monta a imagem com o bitmap do header exportado.
    stoppng.setupFromPixels(configImage1);
    // [EdgeWX] Struct copiada do editor para este widget.
    ImageFromPixelsConfig configImage2 = {
            // [EdgeWX] Bitmap embutido no header que o editor gerou.
            .pixels = iRightpngPixels,
            // [EdgeWX] Mascara de transparencia desse PNG.
            .maskAlpha = iRightpngMask,
            // [EdgeWX] Callback de toque da imagem.
            .cb = rightpng_cb,
            // [EdgeWX] Rotacao da imagem, em graus.
            .angle = 0,
            // [EdgeWX] Largura, em pixels.
            .width = iRightpngW,
            // [EdgeWX] Altura, em pixels.
            .height = iRightpngH,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5
        };
    // [EdgeWX] Monta a imagem com o bitmap do header exportado.
    rightpng.setupFromPixels(configImage2);
    // [EdgeWX] Struct copiada do editor para este widget.
    ImageFromPixelsConfig configImage3 = {
            // [EdgeWX] Bitmap embutido no header que o editor gerou.
            .pixels = iHelppngPixels,
            // [EdgeWX] Mascara de transparencia desse PNG.
            .maskAlpha = iHelppngMask,
            // [EdgeWX] Callback de toque da imagem.
            .cb = helppng_cb,
            // [EdgeWX] Rotacao da imagem, em graus.
            .angle = 0,
            // [EdgeWX] Largura, em pixels.
            .width = iHelppngW,
            // [EdgeWX] Altura, em pixels.
            .height = iHelppngH,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5
        };
    // [EdgeWX] Monta a imagem com o bitmap do header exportado.
    helppng.setupFromPixels(configImage3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setImage(arrayImagem,qtdImagem);

    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar0 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 60,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR04,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY8,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY5,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    load.setup(configCirculaBar0);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar1 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 60,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR15,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY8,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY5,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    load1.setup(configCirculaBar1);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar2 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 60,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR14,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY8,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY5,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    load2.setup(configCirculaBar2);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar3 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 60,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR02,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY8,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY5,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 20,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = true,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    load3.setup(configCirculaBar3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);

    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer0 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = &term1,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 34,
            // [EdgeWX] Altura, em pixels.
            .height = 107,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer.setup(configThermometer0);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer1 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = &term15,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 34,
            // [EdgeWX] Altura, em pixels.
            .height = 107,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR20,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1.setup(configThermometer1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);

}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element cbgas.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbgas_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    check0_val = cbgas.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element cbresf.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbresf_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    check1_val = cbresf.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element cbsuperresf.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void cbsuperresf_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    check2_val = cbsuperresf.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slider.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    slider_val = slider.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for slider is: ");Serial.println(slider_val);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slider1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void slider1_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    slider1_val = slider1.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for slider is: ");Serial.println(slider1_val);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of radio buttons of group 1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void callbackOfGroup_grupo1(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    radiog1 = grupo1.getSelected();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    int myValue = spinbox.getValue();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox1_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    spin1_val = spinbox1.getValue();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox2.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox2_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    spin2_val = spinbox2.getValue();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element spinbox3.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox3_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    spin3_val = spinbox3.getValue();
}

// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element motor.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    switch1_val = motor.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element motor2.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor2_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    switch2_val = motor2.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element motor3.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor3_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    switch3_val = motor3.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element motor4.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void motor4_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    switch4_val = motor4.getStatus();
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element pinionpng.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void pinionpng_cb(){
    // [EdgeWX] Log que o gerador coloca no callback de uma imagem.
    // Image Pinionpng clicked
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.println("Image Pinionpng clicked");
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element stoppng.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void stoppng_cb(){
    // [EdgeWX] Log que o gerador coloca no callback de uma imagem.
    // Image Stoppng clicked
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.println("Image Stoppng clicked");
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rightpng.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rightpng_cb(){
    // [EdgeWX] Log que o gerador coloca no callback de uma imagem.
    // Image Rightpng clicked
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.println("Image Rightpng clicked");
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element helppng.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void helppng_cb(){
    // [EdgeWX] Log que o gerador coloca no callback de uma imagem.
    // Image Helppng clicked
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.println("Image Helppng clicked");
}
