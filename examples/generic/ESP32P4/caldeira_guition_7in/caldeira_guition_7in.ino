// [legenda] [EdgeWX] = tela e widgets sairam do editor. [manual] = senoides, timer de 200 ms e o circulo de matiz.
// [legenda] JD9165 1024x600 e GT911. O loop troca o placeholder do gerador pelos counters[].
// [EdgeWX] Header do PNG iClosepng.h, exportado pelo editor junto com os pixels.
#include "iClosepng.h"
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
// [EdgeWX] Largura do painel em pixels, usada no toque e como tamanho logico.
const int DISPLAY_W = 1024;
// [EdgeWX] Altura do painel em pixels.
const int DISPLAY_H = 600;
// [EdgeWX] Clock de SPI deixado na ficha. Este painel e DSI e nao usa este numero.
const int DISP_FREQUENCY = 27000000;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X1 = 1024;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_Y1 = 600;
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
const int TOUCH_INT = -1;
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
// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton2_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton3_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle2_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void spinbox_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void textbutton_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void textbutton1_cb();
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void closepng_cb();

// [EdgeWX] GPIO do backlight, quando a placa tem um pino direto.
#define DISP_LED 23
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_ESP32DSIPanel *bus = nullptr;
// [EdgeWX] Ponteiro preenchido no setup.
Arduino_DSI_Display *tft = nullptr;
// [EdgeWX] Rotacao 0..3 do painel. O comentario em ingles no fim da linha e do gerador.
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;

// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] GaugeSuper widget no ponto (780, 220), tela 0. Nome e posicao vieram do editor.
GaugeSuper widget(780, 220, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdGauge = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
GaugeSuper *arrayGauge[qtdGauge] = {&widget};
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdIntervalG0 = 4;
// [EdgeWX] Variavel deste sketch.
int range0[qtdIntervalG0] = {0,20,70,80};
// [EdgeWX] Tabela de cores gerada pelo editor.
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR01,CFK_COLOR10,CFK_COLOR05,CFK_COLOR01};
// [EdgeWX] Label term1 no ponto (290, 350), tela 0. Nome e posicao vieram do editor.
Label term1(290, 350, 0);
// [EdgeWX] Label term11 no ponto (290, 395), tela 0. Nome e posicao vieram do editor.
Label term11(290, 395, 0);
// [EdgeWX] Label term12 no ponto (415, 350), tela 0. Nome e posicao vieram do editor.
Label term12(415, 350, 0);
// [EdgeWX] Label term13 no ponto (415, 395), tela 0. Nome e posicao vieram do editor.
Label term13(415, 395, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLabel = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Label *arrayLabel[qtdLabel] = {&term1, &term11, &term12, &term13};
// [EdgeWX] LineChart linechart no ponto (565, 300), tela 0. Nome e posicao vieram do editor.
LineChart linechart(565, 300, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLineChart = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
LineChart *arrayLinechart[qtdLineChart] = {&linechart};
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdLinesChart0 = 2;
// [EdgeWX] Tabela de cores gerada pelo editor.
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR28,CFK_COLOR08};
// [EdgeWX] RectButton rectbutton no ponto (30, 501), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton(30, 501, 0);
// [EdgeWX] RectButton rectbutton1 no ponto (100, 501), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton1(100, 501, 0);
// [EdgeWX] RectButton rectbutton2 no ponto (170, 501), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton2(170, 501, 0);
// [EdgeWX] RectButton rectbutton3 no ponto (235, 498), tela 0. Nome e posicao vieram do editor.
RectButton rectbutton3(235, 498, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdRectBtn = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton, &rectbutton1, &rectbutton2, &rectbutton3};
// [EdgeWX] SpinBox spinbox no ponto (320, 510), tela 0. Nome e posicao vieram do editor.
SpinBox spinbox(320, 510, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdSpinbox = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox};
// [EdgeWX] TextButton textbutton no ponto (560, 465), tela 0. Nome e posicao vieram do editor.
TextButton textbutton(560, 465, 0);
// [EdgeWX] TextButton textbutton1 no ponto (560, 520), tela 0. Nome e posicao vieram do editor.
TextButton textbutton1(560, 520, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdTextButton = 2;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
TextButton *arrayTextButton[qtdTextButton] = {&textbutton, &textbutton1};
// [EdgeWX] ToggleButton toggle no ponto (920, 450), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle(920, 450, 0);
// [EdgeWX] ToggleButton toggle1 no ponto (920, 495), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle1(920, 495, 0);
// [EdgeWX] ToggleButton toggle2 no ponto (920, 540), tela 0. Nome e posicao vieram do editor.
ToggleButton toggle2(920, 540, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdToggleBtn = 3;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle, &toggle1, &toggle2};
// [EdgeWX] Image closepng no ponto (980, 15), tela 0. Nome e posicao vieram do editor.
Image closepng(980, 15, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdImagem = 1;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Image *arrayImagem[qtdImagem] = {&closepng};
// [EdgeWX] CircularBar circload no ponto (140, 230), tela 0. Nome e posicao vieram do editor.
CircularBar circload(140, 230, 0);
// [EdgeWX] CircularBar circload1 no ponto (140, 230), tela 0. Nome e posicao vieram do editor.
CircularBar circload1(140, 230, 0);
// [EdgeWX] CircularBar circload2 no ponto (140, 230), tela 0. Nome e posicao vieram do editor.
CircularBar circload2(140, 230, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdCircBar = 3;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
CircularBar *arrayCircularbar[qtdCircBar] = {&circload, &circload1, &circload2};
// [EdgeWX] Thermometer thermometer no ponto (295, 120), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer(295, 120, 0);
// [EdgeWX] Thermometer thermometer1 no ponto (355, 120), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer1(355, 120, 0);
// [EdgeWX] Thermometer thermometer2 no ponto (415, 120), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer2(415, 120, 0);
// [EdgeWX] Thermometer thermometer3 no ponto (475, 120), tela 0. Nome e posicao vieram do editor.
Thermometer thermometer3(475, 120, 0);
// [EdgeWX] Tamanho do array exigido pela funcao set correspondente.
const uint8_t qtdThermometer = 4;
// [EdgeWX] Ponteiros na ordem em que o set da biblioteca recebe o grupo.
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer, &thermometer1, &thermometer2, &thermometer3};

// [manual] Comentario escrito a mao. A contagem real deste arquivo pode nao ser 7.
// create 7 'int count' with constexpr offset between 0 and 6
// [manual] Afastamento, em graus, entre um seno e o proximo.
constexpr int offsetCount = 20;
// [manual] Variavel deste sketch.
int countGeral = 0, auxCountGeral = 0;
// [manual] Variavel deste sketch.
const int countSize = 8;
// [manual] Variavel deste sketch.
float counters[countSize] = {
    // [manual] Oito amostras. A ultima e a curva composta.
    0,0,0,0,0,0,0,0
};

// [manual] Variavel deste sketch.
float getCurve(float x){
  // [manual] Formula anotada no rascunho. O codigo soma dois senos.
  //y\ =\ \sin\left(x\right)+\sin\left(3x\right)+\left(2\cdot\sin\left(x\right)\right)

  // [manual] Variavel deste sketch.
  float result = sin(x * DEGTORAD) + sin(3 * x * DEGTORAD);

  // [manual] Conta escrita a mao para trazer a soma dos senos para a faixa 0 a 100.
  // current result is from -3 to 3, need to normatize value from 0 to 100.
  // [manual] O typo e o calculo sao do rascunho manual. O codigo abaixo usa 25 e 50.
  // so need mutiply by 30 and add 30
  // [manual] Traz a soma dos senos para perto de 0 a 100.
  result = result * 25 + 50;

  // [manual] Sai da funcao.
  return result;
}

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
    // [manual] A oitava amostra usa a curva composta.
    counters[7] = getCurve(auxCountGeral);
    // [manual] Anda 5 graus a cada ciclo.
    countGeral+=5;
    // [manual] A curva composta anda 1 grau a cada ciclo.
    auxCountGeral++;
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

// [manual] Disco com preto no centro e a cor na borda.
void drawHueCircleDark(int16_t cx, int16_t cy, uint16_t radius) {
  // [manual] Raio ao quadrado, para saber se o ponto cai dentro do disco.
  const int32_t r2 = radius * radius;

  // [manual] Abre uma rajada de pixels. Fecha com endWrite.
  tft->startWrite();

  // [manual] Condicao ou repeticao.
  for (int16_t dy = -radius; dy <= radius; dy++) {
    // [manual] dy ao quadrado, reusado na linha inteira.
    int32_t y2 = dy * dy;
    // [manual] Condicao ou repeticao.
    for (int16_t dx = -radius; dx <= radius; dx++) {
      // [manual] Distancia ao quadrado deste ponto ate o centro.
      int32_t d2 = dx * dx + y2;
      // [manual] Condicao ou repeticao.
      if (d2 > r2) continue; // fora do círculo

      // [manual] 0 no centro, 1 na borda.
      // Distância normalizada do centro (0.0 no centro, 1.0 na borda)
      // [manual] Variavel deste sketch.
      float dist = sqrtf((float)d2) / radius;

      // [manual] A cor muda com o angulo.
      // Matiz (ângulo)
      // [manual] Variavel deste sketch.
      float hue = (atan2f(dy, dx) + PI) / (2.0f * PI);

      // [manual] Brilho cresce do centro preto ate a borda.
      // Saturação sempre máxima (1.0), brilho (v) depende da distância
      // [manual] Variavel deste sketch.
      float s = 1.0f;
      // [manual] Variavel deste sketch.
      float v = dist; // 0 no centro (preto), 1 na borda

      // [manual] Converte matiz, saturacao e brilho num pixel RGB565 e grava no circulo.
      uint16_t color = hsvToRgb565Fast(hue, s, v);
      // [manual] Grava um pixel RGB565.
      tft->writePixel(cx + dx, cy + dy, color);
    }
  }

  // [manual] Fecha a rajada de pixels.
  tft->endWrite();
}

// [manual] Disco com branco no centro e a cor na borda.
void drawHueCircleLight(int16_t cx, int16_t cy, uint16_t radius) {
  // [manual] Raio ao quadrado, para saber se o ponto cai dentro do disco.
  const int32_t r2 = radius * radius;

  // [manual] Abre uma rajada de pixels. Fecha com endWrite.
  tft->startWrite();

  // [manual] Condicao ou repeticao.
  for (int16_t dy = -radius; dy <= radius; dy++) {
    // [manual] dy ao quadrado, reusado na linha inteira.
    int32_t y2 = dy * dy;
    // [manual] Condicao ou repeticao.
    for (int16_t dx = -radius; dx <= radius; dx++) {
      // [manual] Distancia ao quadrado deste ponto ate o centro.
      int32_t d2 = dx * dx + y2;
      // [manual] Condicao ou repeticao.
      if (d2 > r2) continue;

      // [manual] Variavel deste sketch.
      float s = sqrtf((float)d2 / (float)r2);           // saturação (0–1)
      // [manual] Variavel deste sketch.
      float hue = (atan2f(dy, dx) + PI) / (2.0f * PI);  // matiz (0–1)

      // [manual] Converte matiz, saturacao e brilho num pixel RGB565 e grava no circulo.
      uint16_t color = hsvToRgb565Fast(hue, s, 1.0f);
      // [manual] Grava um pixel RGB565.
      tft->writePixel(cx + dx, cy + dy, color);
    }
  }

  // [manual] Fecha a rajada de pixels.
  tft->endWrite();
}



// [manual] Mostra o disco claro e depois o escuro, dois segundos cada.
void testColorScreen(){
    // [manual] Condicao ou repeticao.
    if (!tft) return;
    // [manual] Pinta o fundo da tela com a cor desta chamada.
    tft->fillScreen(CFK_BLACK);
    // [manual] Disco claro no centro, raio 250.
    drawHueCircleLight(DISPLAY_W / 2, DISPLAY_H / 2, 250);
    // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(2000);
    // [manual] Pinta o fundo da tela com a cor desta chamada.
    tft->fillScreen(CFK_WHITE);
    // [manual] Disco escuro no centro, raio 250.
    drawHueCircleDark(DISPLAY_W / 2, DISPLAY_H / 2, 250);
    // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(2000);
}

// [manual] Abre um bloco comentado. Nao executa.
/*void drawTestColor(){
    // create variable uint16_t for: red, gree, blue, yellow, cyan, magenta
    const uint16_t red = 0xF800;
    const uint16_t green = 0x07E0;
    const uint16_t blue = 0x001F;
    const uint16_t yellow = 0xFFE0;
    const uint16_t cyan = 0x07FF;
    const uint16_t magenta = 0xF81F;

    // draw circle in row with radius 20px, offset 20px, starting x = offset and y = half height if screen
    uint16_t colors[] = {red, green, blue, yellow, cyan, magenta};
    uint8_t colorLen = 6;
    const int radius = 20;
    const int offset = 20;
    const int halfHeight = DISPLAY_H / 2;

    const uint8_t raio = 50;
    int intervalo = 360 / colorLen;
    int centerX = DISPLAY_W / 2;
    int centerY = DISPLAY_H / 2;


    for(int i = 0; i < colorLen; i++){
        float angulo = i * intervalo;
        float radianos = angulo * DEGTORAD;
        int x = cos(radianos) * raio + centerX;
        int y = sin(radianos) * raio + centerY;
        tft->fillCircle(x, y, radius, colors[i]);
    }
}*/

// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup(){
    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [EdgeWX] Cria o barramento MIPI DSI com os tempos deste painel.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] Pulso, porch traseiro e porch dianteiro do HSYNC, na unidade do painel.
    40 /* hsync_pulse_width */, 160 /* hsync_back_porch */, 160 /* hsync_front_porch */,
    // [EdgeWX] Pulso e porches do VSYNC.
    10 /* vsync_pulse_width */, 23 /*vsync_back_porch  */, 12 /* vsync_front_porch */,
    // [EdgeWX] Tempos do JD9165 e clock de 48 MHz, como no exemplo da Guition de 7 polegadas.
    48000000 /* prefer_speed */);
    // [EdgeWX] Cria o display DSI preso a esse barramento. auto_flush true entrega o framebuffer ao painel.
    tft = new Arduino_DSI_Display(
    // [EdgeWX] Largura e altura nativas, barramento, rotacao e auto_flush.
    1024 /* width */, 600 /* height */, bus, rotationScreen /* rotation */, true /* auto_flush */,
    // [EdgeWX] Tempos do JD9165 e clock de 48 MHz, como no exemplo da Guition de 7 polegadas.
    27 /* RST */, jd9165_init_operations, sizeof(jd9165_init_operations) / sizeof(lcd_init_cmd_t));
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
    // myDisplay.enableTouchLog();
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
    // [manual] Arma o timer de 200 ms que pede a atualizacao dos medidores.
    iniciarDespertador();
}
// [manual] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop(){
    // [manual] Condicao ou repeticao.
    if(flagAtualizarTela){
      // [manual] Toma o mutex de desenho. setValue nao corre junto com a tarefa de UI.
      myDisplay.startCustomDraw();

      // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
      widget.setValue(counters[5]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    linechart.push(0, counters[1]); //Use this line to add value in serie 0.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    linechart.push(1, counters[7]); //Use this line to add value in serie 1.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    circload.setValue(counters[0]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    circload1.setValue(counters[3]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    circload2.setValue(counters[5]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    thermometer.setValue(counters[6]); //Use this command to change widget value.
    
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    thermometer1.setValue(counters[4]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    thermometer2.setValue(counters[2]); //Use this command to change widget value.
    // [manual] O comentario em ingles, se houver, e do gerador. O numero veio do seno, nao de random.
    thermometer3.setValue(counters[0]); //Use this command to change widget value.

        // [manual] Devolve o mutex. A tarefa redesenha o que foi marcado.
        myDisplay.finishCustomDraw();// Libera a task do displaywx para desenha
        // [manual] Este ciclo ja desenhou. O proximo espera outro estouro do timer.
        flagAtualizarTela = false;
        // [manual] Avanca os senos para a proxima amostra.
        increaseCounters();
    }
    // [manual] Folga curta. Quem pede o redesenho e o timer, nao este delay.
    delay(10);
}
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0(){
    // [manual] Mostra dois circulos de matiz, 2 s cada, antes do painel de verdade.
    testColorScreen();

    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_GREY3);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(545, 255, 468, 186, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(545, 255, 468, 186, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(15, 65, 255, 378, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(15, 65, 255, 378, 10, CFK_BLACK);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Controle da sala", 355, 14, TL_DATUM, CFK_COLOR09, CFK_GREY3, &RobotoRegular20pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an circularBar
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(30, 355, 20, 20, CFK_COLOR01);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(30, 355, 20, 20, CFK_BLACK);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Setor A", 60, 357, TL_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular10pt7b);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRect(30, 390, 20, 20, CFK_COLOR05);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRect(30, 390, 20, 20, CFK_BLACK);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Setor B", 60, 392, TL_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular10pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Limite de corrente por setor (%)", 145, 86, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(15, 450, 520, 134, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(15, 450, 520, 134, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(280, 65, 255, 378, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(280, 65, 255, 378, 10, CFK_BLACK);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(545, 65, 468, 176, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(545, 65, 468, 176, 10, CFK_BLACK);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an ponteiroAnalog
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an imagem
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Temperatura das bombas", 405, 86, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an thermometer
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an label
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an rectButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an spinbox
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an grafico
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Historico de consumo", 790, 271, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Preenche um retangulo estatico exportado com a tela.
    tft->fillRoundRect(545, 445, 466, 134, 10, CFK_GREY4);
    // [EdgeWX] Desenha a borda de um retangulo estatico exportado com a tela.
    tft->drawRoundRect(545, 445, 466, 134, 10, CFK_BLACK);
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an textButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an textButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do gerador: este widget nao e desenhado aqui. drawWidgetsOnScreen cuida dele.
    //This screen has a/an toggleButton
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Control. temperatura", 835, 456, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Control. umidade", 850, 501, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Iluminacao", 865, 546, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Setores (A, B, C, D)", 160, 466, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Nivel ventilacao", 415, 471, TC_DATUM, CFK_WHITE, CFK_GREY4, &RobotoRegular8pt7b);
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e o desta chamada.
    myDisplay.drawWidgetsOnScreen(0);
}
// [EdgeWX] Comentario fixo do gerador, inclusive o typo widgtes.
// Configure each widgtes to be used
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets(){
    // [EdgeWX] Struct copiada do editor para este widget.
    GaugeConfig configGauge0 = {
            // [EdgeWX] Titulo desenhado no widget.
            .title = "Pressurizador (Pa)",
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
            .width = 445,
            // [EdgeWX] Altura, em pixels.
            .height = 140,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_GREY2,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
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
    widget.setup(configGauge0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setGauge(arrayGauge,qtdGauge);

    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel0 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR03,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY4
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term1.setup(configLabel0);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel1 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR05,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY4
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term11.setup(configLabel1);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel2 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR10,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY4
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term12.setup(configLabel2);
    // [EdgeWX] Struct copiada do editor para este widget.
    LabelConfig configLabel3 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Term 1",
            // [EdgeWX] Texto antes do valor. Vazio nao escreve nada.
            .prefix = "",
            // [EdgeWX] Texto depois do valor.
            .suffix = "C",
            // [EdgeWX] Fonte GFX do texto.
            .fontFamily = &RobotoRegular10pt7b,
            // [EdgeWX] Ancora do texto. TL_DATUM e o canto superior esquerdo.
            .datum = TL_DATUM,
            // [EdgeWX] Cor da fonte.
            .fontColor = CFK_COLOR19,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY4
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    term13.setup(configLabel3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setLabel(arrayLabel,qtdLabel);

    // [EdgeWX] Struct copiada do editor para este widget.
    LineChartConfig configLineChart0 = {
            // [EdgeWX] Uma cor por serie.
            .colorsSeries = colorsChart0,
            // [EdgeWX] Legendas das series. nullptr esconde.
            .subtitles = nullptr,
            // [EdgeWX] Fonte dos numeros do grafico.
            .font = &RobotoRegular5pt7b,
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 439,
            // [EdgeWX] Altura, em pixels.
            .height = 129,
            // [EdgeWX] Cor da grade.
            .gridColor = CFK_BLACK,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE,
            // [EdgeWX] Linhas horizontais da grade.
            .verticalDivision = 10,
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
    linechart.setup(configLineChart0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);

    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 53,
            // [EdgeWX] Altura, em pixels.
            .height = 70,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR09
        };
// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton.setup(configRectButton0);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton1_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 53,
            // [EdgeWX] Altura, em pixels.
            .height = 70,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR09
        };
// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton1.setup(configRectButton1);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton2 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton2_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 53,
            // [EdgeWX] Altura, em pixels.
            .height = 70,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR09
        };
// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton2.setup(configRectButton2);
    // [EdgeWX] Struct copiada do editor para este widget.
    RectButtonConfig configRectButton3 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = rectbutton3_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 53,
            // [EdgeWX] Altura, em pixels.
            .height = 73,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR09
        };
// [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
rectbutton3.setup(configRectButton3);
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
            .startValue = 51,
            // [EdgeWX] Largura, em pixels.
            .width = 183,
            // [EdgeWX] Altura, em pixels.
            .height = 59,
            // [EdgeWX] Quanto soma ou subtrai a cada toque nas setas.
            .step = 1,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR14,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    spinbox.setup(configSpinBox0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);

    // [EdgeWX] Struct copiada do editor para este widget.
    TextButtonConfig configTextButton0 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Config",
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = textbutton_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 111,
            // [EdgeWX] Altura, em pixels.
            .height = 37,
            // [EdgeWX] Raio, em pixels.
            .radius = 10,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_COLOR31,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE
        };

    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    textbutton.setup(configTextButton0);
    // [EdgeWX] Struct copiada do editor para este widget.
    TextButtonConfig configTextButton1 = {
            // [EdgeWX] Texto visivel, copiado do editor.
            .text = "Desl. tela",
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = textbutton1_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 109,
            // [EdgeWX] Altura, em pixels.
            .height = 37,
            // [EdgeWX] Raio, em pixels.
            .radius = 10,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_WHITE
        };

    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    textbutton1.setup(configTextButton1);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle0 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 64,
            // [EdgeWX] Altura, em pixels.
            .height = 32,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR13
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle.setup(configToggle0);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle1 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle1_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 64,
            // [EdgeWX] Altura, em pixels.
            .height = 32,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR15
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle1.setup(configToggle1);
    // [EdgeWX] Struct copiada do editor para este widget.
    ToggleButtonConfig configToggle2 = {
            // [EdgeWX] Callback de toque. A tarefa de UI chama; o sketch nao.
            .callback = toggle2_cb,
            // [EdgeWX] Largura, em pixels.
            .width = 64,
            // [EdgeWX] Altura, em pixels.
            .height = 32,
            // [EdgeWX] Cor com o controle pressionado ou ligado.
            .pressedColor = CFK_COLOR17
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    toggle2.setup(configToggle2);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);

    // [EdgeWX] Struct copiada do editor para este widget.
    ImageFromPixelsConfig configImage0 = {
            // [EdgeWX] Bitmap embutido no header que o editor gerou.
            .pixels = iClosepngPixels,
            // [EdgeWX] Mascara de transparencia desse PNG.
            .maskAlpha = iClosepngMask,
            // [EdgeWX] Callback de toque da imagem.
            .cb = closepng_cb,
            // [EdgeWX] Rotacao da imagem, em graus.
            .angle = 0,
            // [EdgeWX] Largura, em pixels.
            .width = iClosepngW,
            // [EdgeWX] Altura, em pixels.
            .height = iClosepngH,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] Monta a imagem com o bitmap do header exportado.
    closepng.setupFromPixels(configImage0);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setImage(arrayImagem,qtdImagem);

    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar0 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 100,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR01,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY4,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 10,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = false,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload.setup(configCirculaBar0);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar1 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 85,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR05,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY4,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 10,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = false,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload1.setup(configCirculaBar1);
    // [EdgeWX] Struct copiada do editor para este widget.
    CircularBarConfig configCirculaBar2 = {
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Raio, em pixels.
            .radius = 70,
            // [EdgeWX] Angulo inicial do arco, em graus.
            .startAngle = 0,
            // [EdgeWX] Angulo final. 360 fecha o circulo.
            .endAngle = 360,
            // [EdgeWX] Cor principal do desenho.
            .color = CFK_COLOR10,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor do texto.
            .textColor = CFK_BLACK,
            // [EdgeWX] Fundo do numero no centro.
            .backgroundText = CFK_GREY4,
            // [EdgeWX] Espessura do arco, em pixels.
            .thickness = 10,
            // [EdgeWX] true desenha o numero no centro.
            .showValue = false,
            // [EdgeWX] true inverte o sentido em que o arco enche.
            .inverted = false
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    circload2.setup(configCirculaBar2);
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
            .width = 38,
            // [EdgeWX] Altura, em pixels.
            .height = 210,
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
            .subtitle = &term11,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 40,
            // [EdgeWX] Altura, em pixels.
            .height = 212,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR05,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer1.setup(configThermometer1);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer2 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = &term12,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 40,
            // [EdgeWX] Altura, em pixels.
            .height = 212,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR10,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer2.setup(configThermometer2);
    // [EdgeWX] Struct copiada do editor para este widget.
    ThermometerConfig configThermometer3 = {
            // [EdgeWX] Legenda. nullptr nao desenha subtitulo.
            .subtitle = &term13,
            // [EdgeWX] Unidade escrita ao lado do valor.
            .unit = "C",
            // [EdgeWX] Menor valor da faixa.
            .minValue = 0,
            // [EdgeWX] Maior valor da faixa.
            .maxValue = 100,
            // [EdgeWX] Largura, em pixels.
            .width = 40,
            // [EdgeWX] Altura, em pixels.
            .height = 212,
            // [EdgeWX] Cor da parte cheia.
            .filledColor = CFK_COLOR19,
            // [EdgeWX] Cor de fundo deste widget.
            .backgroundColor = CFK_GREY5,
            // [EdgeWX] Cor das marcas da escala.
            .markColor = CFK_BLACK,
            // [EdgeWX] Casas depois da virgula.
            .decimalPlaces = 1
        };
    // [EdgeWX] Aplica a struct. Sem setup o widget nao entra no redraw.
    thermometer3.setup(configThermometer3);
    // [EdgeWX] Entrega o array a DisplayWX. A contagem tem de bater com o tamanho do array.
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);

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
// This function is a callback of this element rectbutton2.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton2_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton2.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element rectbutton3.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void rectbutton3_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = rectbutton3.getStatus();
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
// This function is a callback of this element textbutton.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void textbutton_cb(){
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("Clicked on: ");Serial.println("textbutton_cb");
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element textbutton1.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void textbutton1_cb(){
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
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
// This function is a callback of this element toggle2.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void toggle2_cb(){
    // [EdgeWX] Le o estado que o widget guardou e o gerador copia para a variavel local ou global.
    bool myValue = toggle2.getStatus();
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element closepng.
// [EdgeWX] O gerador avisa para nao chamar esta funcao no sketch.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Callback do widget citado no comentario do gerador logo acima.
void closepng_cb(){
    // [EdgeWX] Log que o gerador coloca no callback de uma imagem.
    // Image Closepng clicked
    // [EdgeWX] Log na serial. Nos callbacks, a frase em ingles e o texto fixo do gerador.
    Serial.println("Image Closepng clicked");
}
