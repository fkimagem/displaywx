#include "ipg1back.h"
#include "ipg2back.h"
// Include for plugins of chip 0
// Include external libraries and files
#include <Arduino.h>
#include "esp_log.h" // [MANUAL]
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include "fluxo.h" // [MANUAL]


    /* Project setup:
    * MCU: ESP32S3_EMBEDDED
    * Display: ESP32-8048S070
    * Touch: GT911
    * Camera: None
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 800; ///< Largura util do display, em pixels.
const int DISPLAY_H = 480; ///< Altura util do display, em pixels.
const int TOUCH_MAP_X0 = 0; ///< Limite esquerdo do mapeamento do toque, em pixels.
const int TOUCH_MAP_X1 = 800; ///< Limite direito do mapeamento do toque, em pixels.
const int TOUCH_MAP_Y0 = 0; ///< Limite superior do mapeamento do toque, em pixels.
const int TOUCH_MAP_Y1 = 480; ///< Limite inferior do mapeamento do toque, em pixels.
const bool TOUCH_SWAP_XY = false; ///< Troca os eixos X e Y do toque quando verdadeiro.
const bool TOUCH_INVERT_X = false; ///< Inverte o eixo X do toque quando verdadeiro.
const bool TOUCH_INVERT_Y = false; ///< Inverte o eixo Y do toque quando verdadeiro.
const int TOUCH_SCL = 20; ///< Pino SCL do controlador de toque GT911.
const int TOUCH_SDA = 19; ///< Pino SDA do controlador de toque GT911.
const int TOUCH_INT = -1; ///< Pino de interrupcao do toque; -1 indica que nao e usado.
const int TOUCH_RST = 38; ///< Pino de reset do controlador de toque.
const uint8_t rotationScreen = 0; ///< Rotacao da tela, em passos de 90 graus a partir da origem.
const bool isIPS = false; ///< Seleciona a ordem de bytes deste painel; neste display permanece desligado.

// Prototypes for each screen
void screen0();
void screen1();
void screen2();
void loadWidgets();

// Prototypes for callback functions
void slidergeral_cb();
void sliderprg1_cb();
void sliderprg2_cb();
void pg1starttime_cb();
void pg1runtime_cb();
void pg1endtime_cb();
void pg1freqmax_cb();
void pg1rpmmax_cb();
void pg2starttime_cb();
void pg2runtime_cb();
void pg2stoptime_cb();
void pg2freqmax_cb();
void pg2rpmmax_cb();
void iipg1back_cb();
void iipg2back_cb();
void startbtn_cb();
void reversebtn_cb();
void emergcybtn_cb();
void stopbtn_cb();
void prog1btn_cb();
void prog2btn_cb();
void prog1startbtn_cb();
void stop1btn_cb();
void prog2startbtn_cb();
void stop2btn_cb();

#define DISP_LED 2
Arduino_ESP32RGBPanel *bus = nullptr; ///< Barramento RGB do painel, criado em setup().
Arduino_RGB_Display *tft = nullptr; ///< Controlador do display RGB de 800 por 480.
DisplayWX myDisplay; ///< Interface DisplayWX que registra widgets, toque e telas.
// Create global objects. Constructor is: xPos, yPos and indexScreen
GaugeSuper gaugerpm(215, 120, 0); ///< Medidor de ponteiro do RPM na tela principal.
const uint8_t qtdGauge = 1; ///< Quantidade de medidores de ponteiro registrados.
GaugeSuper *arrayGauge[qtdGauge] = {&gaugerpm}; ///< Lista de medidores entregue a DisplayWX.
const uint8_t qtdIntervalG0 = 4; ///< Quantidade de faixas coloridas do medidor de RPM.
int range0[qtdIntervalG0] = {0,450,900,1350}; ///< Limites, em RPM, das faixas coloridas do medidor.
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR025,CFK_COLOR019,CFK_COLOR013,CFK_COLOR007}; ///< Cores RGB565 das faixas do medidor de RPM.
HSlider slidergeral(35, 400, 0); ///< Slider manual de velocidade da tela principal.
HSlider sliderprg1(30, 360, 1); ///< Slider de setpoint do programa 1.
HSlider sliderprg2(40, 355, 2); ///< Slider de setpoint do programa 2.
const uint8_t qtdHSlider = 3; ///< Quantidade de sliders horizontais registrados.
HSlider *arrayHslider[qtdHSlider] = {&slidergeral, &sliderprg1, &sliderprg2}; ///< Lista de sliders entregue a DisplayWX.
int hslider0_val = 0; ///< Ultimo valor lido no slider geral, em porcentagem.
int hslider1_val = 0; ///< Ultimo valor lido no slider do programa 1, em porcentagem.
int hslider2_val = 0; ///< Ultimo valor lido no slider do programa 2, em porcentagem.
Label valuefact(290, 325, 0); ///< Texto da frequencia atual recebida do motor.
Label valuefset(290, 360, 0); ///< Texto da frequencia de setpoint recebida do motor.
Label labelhsliderge(205, 445, 0); ///< Texto numerico do slider geral.
Label labelservstat(610, 20, 0); ///< Texto do estado da conexao com o servidor.
Label setrpmvalue(240, 310, 1); ///< Texto do RPM calculado para o programa 1.
Label rpmvalue(190, 415, 1); ///< Texto do valor do slider do programa 1.
Label text8copycopy(235, 305, 2); ///< Texto do RPM calculado para o programa 2.
Label rpmvalue6(205, 420, 2); ///< Texto do valor do slider do programa 2.
const uint8_t qtdLabel = 8; ///< Quantidade de textos variaveis registrados.
Label *arrayLabel[qtdLabel] = {&valuefact, &valuefset, &labelhsliderge, &labelservstat, &setrpmvalue, &rpmvalue, &text8copycopy, &rpmvalue6}; ///< Lista de textos variaveis entregue a DisplayWX.
Led led1(465, 35, 0); ///< LED que mostra o estado da conexao por cor.
const uint8_t qtdLed = 1; ///< Quantidade de LEDs registrados.
Led *arrayLed[qtdLed] = {&led1}; ///< Lista de LEDs entregue a DisplayWX.
LineChart graphfreq(435, 85, 0); ///< Grafico de RPM da tela principal.
LineChart graphfreqcopycopy(435, 40, 1); ///< Grafico de RPM da tela do programa 1; o nome e exigido por fluxo.h.
LineChart graphfreqcopycopycopy(435, 40, 2); ///< Grafico de RPM da tela do programa 2; o nome e exigido por fluxo.h.
const uint8_t qtdLineChart = 3; ///< Quantidade de graficos de linha registrados.
LineChart *arrayLinechart[qtdLineChart] = {&graphfreq, &graphfreqcopycopy, &graphfreqcopycopycopy}; ///< Lista de graficos entregue a DisplayWX.
const uint8_t qtdLinesChart0 = 2; ///< Quantidade de series do grafico da tela principal.
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR031,CFK_COLOR055}; ///< Cores das series do grafico da tela principal.
const uint8_t qtdLinesChart1 = 2; ///< Quantidade de series do grafico do programa 1.
uint16_t colorsChart1[qtdLinesChart1] = {CFK_COLOR031,CFK_COLOR055}; ///< Cores das series do grafico do programa 1.
const uint8_t qtdLinesChart2 = 2; ///< Quantidade de series do grafico do programa 2.
uint16_t colorsChart2[qtdLinesChart2] = {CFK_COLOR031,CFK_COLOR055}; ///< Cores das series do grafico do programa 2.
NumberBox pg1starttime(240, 82, 1); ///< Caixa do tempo de espera antes da partida do programa 1.
NumberBox pg1runtime(240, 132, 1); ///< Caixa do tempo em que o programa 1 permanece ligado.
NumberBox pg1endtime(240, 177, 1); ///< Caixa do tempo parado entre ciclos do programa 1.
NumberBox pg1freqmax(240, 222, 1); ///< Caixa da frequencia maxima usada pelo programa 1.
NumberBox pg1rpmmax(240, 272, 1); ///< Caixa do RPM maximo usado pelo programa 1.
NumberBox pg2starttime(235, 85, 2); ///< Caixa do tempo de espera antes da partida do programa 2.
NumberBox pg2runtime(235, 130, 2); ///< Caixa do tempo em que o programa 2 permanece ligado.
NumberBox pg2stoptime(235, 175, 2); ///< Caixa do tempo parado entre ciclos do programa 2.
NumberBox pg2freqmax(235, 220, 2); ///< Caixa da frequencia maxima usada pelo programa 2.
NumberBox pg2rpmmax(235, 260, 2); ///< Caixa do RPM maximo usado pelo programa 2.
const uint8_t qtdNumberbox = 10; ///< Quantidade de caixas numericas registradas.
NumberBox *arrayNumberbox[qtdNumberbox] = {&pg1starttime, &pg1runtime, &pg1endtime, &pg1freqmax, &pg1rpmmax, &pg2starttime, &pg2runtime, &pg2stoptime, &pg2freqmax, &pg2rpmmax}; ///< Lista de caixas numericas entregue a DisplayWX.
float nb0_val = 10; ///< Tempo de espera do programa 1, em segundos.
float nb1_val = 20; ///< Tempo ligado do programa 1, em segundos.
float nb2_val = 10; ///< Tempo parado do programa 1, em segundos.
float nb3_val = 60; ///< Frequencia maxima do programa 1, em hertz.
float nb4_val = 1798; ///< RPM maximo do programa 1.
float nb5_val = 10; ///< Tempo de espera do programa 2, em segundos.
float nb6_val = 30; ///< Tempo ligado do programa 2, em segundos.
float nb7_val = 30; ///< Tempo parado do programa 2, em segundos.
float nb8_val = 60; ///< Frequencia maxima do programa 2, em hertz.
float nb9_val = 1798; ///< RPM maximo do programa 2.
TextButton startbtn(437, 280, 0); ///< Botao de partida manual da tela principal.
TextButton reversebtn(612, 280, 0); ///< Botao de reversao da tela principal.
TextButton emergcybtn(437, 345, 0); ///< Botao de emergencia da tela principal.
TextButton stopbtn(612, 345, 0); ///< Botao de parada da tela principal.
TextButton prog1btn(437, 410, 0); ///< Botao que abre a tela do programa 1.
TextButton prog2btn(612, 410, 0); ///< Botao que abre a tela do programa 2.
TextButton prog1startbtn(490, 275, 1); ///< Botao que inicia o ciclo do programa 1.
TextButton stop1btn(490, 365, 1); ///< Botao que interrompe o programa 1.
TextButton prog2startbtn(485, 280, 2); ///< Botao que inicia o ciclo do programa 2.
TextButton stop2btn(485, 370, 2); ///< Botao que interrompe o programa 2.
const uint8_t qtdTextButton = 10; ///< Quantidade de botoes de texto registrados.
TextButton *arrayTextButton[qtdTextButton] = {&startbtn, &reversebtn, &emergcybtn, &stopbtn, &prog1btn, &prog2btn, &prog1startbtn, &stop1btn, &prog2startbtn, &stop2btn}; ///< Lista de botoes de texto entregue a DisplayWX.
VAnalog analogvdc(70, 135, 0); ///< Barra da tensao da barramento DC.
VAnalog analogtout(190, 135, 0); ///< Barra da tensao de saida do inversor.
VAnalog analogcur(300, 135, 0); ///< Barra da corrente de saida do inversor.
const uint8_t qtdVAnalog = 3; ///< Quantidade de barras analogicas registradas.
VAnalog *arrayVanalog[qtdVAnalog] = {&analogvdc, &analogtout, &analogcur}; ///< Lista de barras analogicas entregue a DisplayWX.
Image iipg1back(0, 0, 1); ///< Imagem de voltar na tela do programa 1.
Image iipg2back(0, 0, 2); ///< Imagem de voltar na tela do programa 2.
const uint8_t qtdImagem = 2; ///< Quantidade de imagens registradas.
Image *arrayImagem[qtdImagem] = {&iipg1back, &iipg2back}; ///< Lista de imagens entregue a DisplayWX.

/**
 * @brief Inicializa o painel RGB, o toque GT911, os widgets e as tarefas de rede.
 */
void setup(){

    Serial.begin(115200);

    Fluxo::prepararFluxo(); // [MANUAL]

    bus = new Arduino_ESP32RGBPanel(
    41 /* DE */, 40 /* VSYNC */, 39 /* HSYNC */, 42 /* PCLK */,
    14 /* R0 */, 21 /* R1 */, 47 /* R2 */, 48 /* R3 */, 45 /* R4 */,
    9 /* G0 */, 46 /* G1 */, 3 /* G2 */, 8 /* G3 */, 16 /* G4 */, 1 /* G5 */,
    15 /* B0 */, 7 /* B1 */, 6 /* B2 */, 5 /* B3 */, 4 /* B4 */,
    0 /* hsync_polarity */, 180 /* hsync_front_porch */, 30 /* hsync_pulse_width */, 16 /* hsync_back_porch */,
    0 /* vsync_polarity */, 13 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 31 /* vsync_back_porch */,
    1 /* pclk_active_neg */, 10000000 /* prefer_speed */, isIPS /* useBigEndian */,
    0 /* de_idle_high */, 0 /* pclk_idle_high */, 16000 /* bounce_buffer_size_px */);
    tft = new Arduino_RGB_Display(
    800, 480, bus,
    rotationScreen, true);
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif

    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(true, Fluxo::TASK_WDT_TIMEOUT_S); // [MANUAL]

    ESP_LOGI("setup", "passou setup"); // [MANUAL]
    Fluxo::iniciarTarefas(); // [MANUAL] Rede em segundo plano; setup nao aguarda Wi-Fi/TLS.
}

/**
 * @brief Repete o ciclo do fluxo, que agenda a interface e alimenta o watchdog.
 */
void loop(){
    Fluxo::executar(); // [MANUAL]
}

/**
 * @brief Desenha o fundo, os paineis e os rotulos fixos da tela principal.
 */
void screen0(){

    myDisplay.setBackgroundColor(CFK_GREY6);
    tft->fillRoundRect(415, 5, 372, 57, 25, CFK_GREY5);
    tft->drawRoundRect(415, 5, 372, 57, 25, CFK_BLACK);
    tft->fillRoundRect(10, 6, 391, 297, 25, CFK_GREY5);
    tft->drawRoundRect(10, 6, 391, 297, 25, CFK_BLACK);
    tft->fillRoundRect(9, 312, 393, 162, 25, CFK_GREY5);
    tft->drawRoundRect(9, 312, 393, 162, 25, CFK_BLACK);
    tft->fillRoundRect(417, 69, 372, 186, 25, CFK_GREY5);
    tft->drawRoundRect(417, 69, 372, 186, 25, CFK_BLACK);
    tft->fillRoundRect(415, 265, 376, 210, 25, CFK_GREY5);
    tft->drawRoundRect(415, 265, 376, 210, 25, CFK_BLACK);
    //This screen has a/an ponteiroAnalog
    //This screen has a/an grafico
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an verticalAnalog
    //This screen has a/an slider
    myDisplay.printText("Frequencia Atual:", 60, 327, TL_DATUM, CFK_BLACK, CFK_GREY5, &RobotoRegular9pt7b);
    myDisplay.printText("Frequencia Setada:", 60, 362, TL_DATUM, CFK_BLACK, CFK_GREY5, &RobotoRegular9pt7b);
    myDisplay.printText("Tensao DC", 50, 276, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Tensao OUT", 160, 276, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("Corrente", 285, 276, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular8pt7b);
    myDisplay.printText("RPM", 580, 227, TL_DATUM, CFK_COLOR020, CFK_GREY5, &RobotoRegular11pt7b);
    //This screen has a/an texto
    //This screen has a/an texto
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an texto
    //This screen has a/an led
    //This screen has a/an texto
    myDisplay.drawWidgetsOnScreen(0);
}
/**
 * @brief Desenha o fundo, os paineis e os rotulos fixos da tela do programa 1.
 */
void screen1(){

    myDisplay.setBackgroundColor(CFK_GREY6);
    tft->fillRoundRect(410, 30, 380, 430, 25, CFK_GREY5);
    tft->drawRoundRect(410, 30, 380, 430, 25, CFK_BLACK);
    tft->fillRoundRect(10, 30, 380, 430, 25, CFK_GREY5);
    tft->drawRoundRect(10, 30, 380, 430, 25, CFK_BLACK);
    //This screen has a/an grafico
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    myDisplay.printText("RPM", 570, 232, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular11pt7b);
    myDisplay.printText("Tempo de inicio:", 220, 82, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Tempo ligado:", 225, 132, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Tempo parado:", 225, 177, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Freq. maxima:", 225, 222, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("RPM maxima:", 225, 272, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("SET RPM:", 225, 312, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    //This screen has a/an texto
    //This screen has a/an texto
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an slider
    //This screen has a/an imagem
    myDisplay.drawWidgetsOnScreen(1);
}
/**
 * @brief Desenha o fundo, os paineis e os rotulos fixos da tela do programa 2.
 */
void screen2(){

    myDisplay.setBackgroundColor(CFK_GREY6);
    tft->fillRoundRect(415, 30, 375, 430, 25, CFK_GREY5);
    tft->drawRoundRect(415, 30, 375, 430, 25, CFK_BLACK);
    tft->fillRoundRect(10, 30, 395, 430, 25, CFK_GREY5);
    tft->drawRoundRect(10, 30, 395, 430, 25, CFK_BLACK);
    //This screen has a/an grafico
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    //This screen has a/an caixaNumero
    myDisplay.printText("RPM", 585, 232, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular11pt7b);
    myDisplay.printText("Tempo de inicio:", 220, 87, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Tempo ligado:", 220, 127, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Tempo parado:", 220, 177, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("Freq. maxima:", 220, 222, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("RPM maxima:", 220, 262, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    myDisplay.printText("SET RPM:", 220, 307, TR_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular13pt7b);
    //This screen has a/an texto
    //This screen has a/an texto
    //This screen has a/an textButton
    //This screen has a/an textButton
    //This screen has a/an slider
    //This screen has a/an imagem
    myDisplay.drawWidgetsOnScreen(2);
}
/**
 * @brief Configura cada widget uma vez e registra os grupos na DisplayWX.
 */
void loadWidgets(){ ///< Monta a ficha de cada widget e registra os grupos na DisplayWX.

    GaugeConfig configGauge0 = { ///< Abre a ficha do medidor de RPM.
            .title = "RPM", ///< Titulo desenhado no medidor de RPM: "RPM".
            .intervals = range0, ///< Limites das faixas coloridas do medidor de RPM, guardados em range0.
            .colors = colors0, ///< Cores RGB565 das faixas do medidor de RPM, guardadas em colors0.
            .fontFamily = &RobotoRegular8pt7b, ///< Fonte do medidor de RPM: RobotoRegular8pt7b.
            .minValue = 0, ///< Menor valor da escala do medidor de RPM: 0.
            .maxValue = 1800, ///< Maior valor da escala do medidor de RPM: 1800.
            .width = 276, ///< Largura do medidor de RPM: 276 pixels.
            .height = 110, ///< Altura do medidor de RPM: 110 pixels.
            .borderColor = CFK_GREY1, ///< Cor da borda do medidor de RPM: CFK_GREY1.
            .textColor = CFK_WHITE, ///< Cor do texto do medidor de RPM: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo do medidor de RPM: CFK_GREY6.
            .titleColor = CFK_WHITE, ///< Cor do titulo do medidor de RPM: CFK_WHITE.
            .needleColor = CFK_RED, ///< Cor do ponteiro do medidor de RPM: CFK_RED.
            .markersColor = CFK_BLACK, ///< Cor das marcas da escala do medidor de RPM: CFK_BLACK.
            .amountIntervals = qtdIntervalG0, ///< Quantidade de faixas coloridas do medidor de RPM, vinda de qtdIntervalG0.
            .showLabels = true ///< Mostra os numeros da escala do medidor de RPM.
        }; ///< Fecha a ficha do medidor de RPM.
    gaugerpm.setup(configGauge0); ///< Aplica a ficha configGauge0 ao widget gaugerpm.
    myDisplay.setGauge(arrayGauge,qtdGauge); ///< Registra o medidor de RPM na DisplayWX.


    HSliderConfig configHSlider0 = { ///< Abre a ficha do slider geral.
            .callback = slidergeral_cb, ///< Chama slidergeral_cb quando o slider geral e alterado.
            .subtitle = &labelhsliderge, ///< O slider geral mostra o valor no texto labelhsliderge.
            .minValue = 0, ///< Menor valor da escala do slider geral: 0.
            .maxValue = 100, ///< Maior valor da escala do slider geral: 100.
            .radius = 18, ///< Raio do slider geral: 18 pixels.
            .width = 337, ///< Largura do slider geral: 337 pixels.
            .pressedColor = CFK_COLOR041, ///< Cor do slider geral enquanto esta pressionado: CFK_COLOR041.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo do slider geral: CFK_WHITE.
        }; ///< Fecha a ficha do slider geral.
    slidergeral.setup(configHSlider0); ///< Aplica a ficha configHSlider0 ao widget slidergeral.
    HSliderConfig configHSlider1 = { ///< Abre a ficha do slider do programa 1.
            .callback = sliderprg1_cb, ///< Chama sliderprg1_cb quando o slider do programa 1 e alterado.
            .subtitle = nullptr, ///< O slider do programa 1 nao usa um texto auxiliar.
            .minValue = 0, ///< Menor valor da escala do slider do programa 1: 0.
            .maxValue = 100, ///< Maior valor da escala do slider do programa 1: 100.
            .radius = 21, ///< Raio do slider do programa 1: 21 pixels.
            .width = 343, ///< Largura do slider do programa 1: 343 pixels.
            .pressedColor = CFK_GREY3, ///< Cor do slider do programa 1 enquanto esta pressionado: CFK_GREY3.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo do slider do programa 1: CFK_WHITE.
        }; ///< Fecha a ficha do slider do programa 1.
    sliderprg1.setup(configHSlider1); ///< Aplica a ficha configHSlider1 ao widget sliderprg1.
    HSliderConfig configHSlider2 = { ///< Abre a ficha do slider do programa 2.
            .callback = sliderprg2_cb, ///< Chama sliderprg2_cb quando o slider do programa 2 e alterado.
            .subtitle = nullptr, ///< O slider do programa 2 nao usa um texto auxiliar.
            .minValue = 0, ///< Menor valor da escala do slider do programa 2: 0.
            .maxValue = 100, ///< Maior valor da escala do slider do programa 2: 100.
            .radius = 21, ///< Raio do slider do programa 2: 21 pixels.
            .width = 343, ///< Largura do slider do programa 2: 343 pixels.
            .pressedColor = CFK_GREY3, ///< Cor do slider do programa 2 enquanto esta pressionado: CFK_GREY3.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo do slider do programa 2: CFK_WHITE.
        }; ///< Fecha a ficha do slider do programa 2.
    sliderprg2.setup(configHSlider2); ///< Aplica a ficha configHSlider2 ao widget sliderprg2.
    myDisplay.setHSlider(arrayHslider,qtdHSlider); ///< Registra os tres sliders na DisplayWX.


    LabelConfig configLabel0 = { ///< Abre a ficha do texto da frequencia atual.
            .text = " Hz", ///< Texto inicial do texto da frequencia atual: " Hz".
            .prefix = "", ///< Nao escreve texto antes do valor do texto da frequencia atual.
            .suffix = " Hz", ///< Texto fixo depois do valor do texto da frequencia atual: " Hz".
            .fontFamily = &RobotoRegular12pt7b, ///< Fonte do texto da frequencia atual: RobotoRegular12pt7b.
            .datum = TL_DATUM, ///< No texto da frequencia atual, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_COLOR013, ///< Cor da fonte do texto da frequencia atual: CFK_COLOR013.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto da frequencia atual: CFK_GREY5.
        }; ///< Fecha a ficha do texto da frequencia atual.
    valuefact.setup(configLabel0); ///< Aplica a ficha configLabel0 ao widget valuefact.
    LabelConfig configLabel1 = { ///< Abre a ficha do texto da frequencia de setpoint.
            .text = " Hz", ///< Texto inicial do texto da frequencia de setpoint: " Hz".
            .prefix = "", ///< Nao escreve texto antes do valor do texto da frequencia de setpoint.
            .suffix = " Hz", ///< Texto fixo depois do valor do texto da frequencia de setpoint: " Hz".
            .fontFamily = &RobotoRegular12pt7b, ///< Fonte do texto da frequencia de setpoint: RobotoRegular12pt7b.
            .datum = TL_DATUM, ///< No texto da frequencia de setpoint, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_COLOR043, ///< Cor da fonte do texto da frequencia de setpoint: CFK_COLOR043.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto da frequencia de setpoint: CFK_GREY5.
        }; ///< Fecha a ficha do texto da frequencia de setpoint.
    valuefset.setup(configLabel1); ///< Aplica a ficha configLabel1 ao widget valuefset.
    LabelConfig configLabel2 = { ///< Abre a ficha do texto do percentual do slider geral.
            .text = "%", ///< Texto inicial do texto do percentual do slider geral: "%".
            .prefix = "", ///< Nao escreve texto antes do valor do texto do percentual do slider geral.
            .suffix = "%", ///< Texto fixo depois do valor do texto do percentual do slider geral: "%".
            .fontFamily = &RobotoRegular8pt7b, ///< Fonte do texto do percentual do slider geral: RobotoRegular8pt7b.
            .datum = TC_DATUM, ///< No texto do percentual do slider geral, alinha o texto pelo centro superior.
            .fontColor = CFK_COLOR043, ///< Cor da fonte do texto do percentual do slider geral: CFK_COLOR043.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do percentual do slider geral: CFK_GREY5.
        }; ///< Fecha a ficha do texto do percentual do slider geral.
    labelhsliderge.setup(configLabel2); ///< Aplica a ficha configLabel2 ao widget labelhsliderge.
    LabelConfig configLabel3 = { ///< Abre a ficha do texto do RPM do programa 1.
            .text = "900", ///< Texto inicial do texto do RPM do programa 1: "900".
            .prefix = "", ///< Nao escreve texto antes do valor do texto do RPM do programa 1.
            .suffix = "", ///< Nao escreve texto depois do valor do texto do RPM do programa 1.
            .fontFamily = &RobotoRegular13pt7b, ///< Fonte do texto do RPM do programa 1: RobotoRegular13pt7b.
            .datum = TL_DATUM, ///< No texto do RPM do programa 1, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_WHITE, ///< Cor da fonte do texto do RPM do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do RPM do programa 1: CFK_GREY5.
        }; ///< Fecha a ficha do texto do RPM do programa 1.
    setrpmvalue.setup(configLabel3); ///< Aplica a ficha configLabel3 ao widget setrpmvalue.
    LabelConfig configLabel4 = { ///< Abre a ficha do texto do percentual do programa 1.
            .text = "%", ///< Texto inicial do texto do percentual do programa 1: "%".
            .prefix = "", ///< Nao escreve texto antes do valor do texto do percentual do programa 1.
            .suffix = "%", ///< Texto fixo depois do valor do texto do percentual do programa 1: "%".
            .fontFamily = &RobotoRegular13pt7b, ///< Fonte do texto do percentual do programa 1: RobotoRegular13pt7b.
            .datum = TL_DATUM, ///< No texto do percentual do programa 1, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_WHITE, ///< Cor da fonte do texto do percentual do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do percentual do programa 1: CFK_GREY5.
        }; ///< Fecha a ficha do texto do percentual do programa 1.
    rpmvalue.setup(configLabel4); ///< Aplica a ficha configLabel4 ao widget rpmvalue.
    LabelConfig configLabel5 = { ///< Abre a ficha do texto do RPM do programa 2.
            .text = "900", ///< Texto inicial do texto do RPM do programa 2: "900".
            .prefix = "", ///< Nao escreve texto antes do valor do texto do RPM do programa 2.
            .suffix = "", ///< Nao escreve texto depois do valor do texto do RPM do programa 2.
            .fontFamily = &RobotoRegular13pt7b, ///< Fonte do texto do RPM do programa 2: RobotoRegular13pt7b.
            .datum = TL_DATUM, ///< No texto do RPM do programa 2, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_WHITE, ///< Cor da fonte do texto do RPM do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do RPM do programa 2: CFK_GREY5.
        }; ///< Fecha a ficha do texto do RPM do programa 2.
    text8copycopy.setup(configLabel5); ///< Aplica a ficha configLabel5 ao widget text8copycopy.
    LabelConfig configLabel6 = { ///< Abre a ficha do texto do percentual do programa 2.
            .text = "%", ///< Texto inicial do texto do percentual do programa 2: "%".
            .prefix = "", ///< Nao escreve texto antes do valor do texto do percentual do programa 2.
            .suffix = "%", ///< Texto fixo depois do valor do texto do percentual do programa 2: "%".
            .fontFamily = &RobotoRegular13pt7b, ///< Fonte do texto do percentual do programa 2: RobotoRegular13pt7b.
            .datum = TL_DATUM, ///< No texto do percentual do programa 2, alinha o texto pelo canto superior esquerdo.
            .fontColor = CFK_WHITE, ///< Cor da fonte do texto do percentual do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do percentual do programa 2: CFK_GREY5.
        }; ///< Fecha a ficha do texto do percentual do programa 2.
    rpmvalue6.setup(configLabel6); ///< Aplica a ficha configLabel6 ao widget rpmvalue6.
    LabelConfig configLabel7 = { ///< Abre a ficha do texto do estado do servidor.
            .text = "Offline", ///< Texto inicial do texto do estado do servidor: "Offline".
            .prefix = "Server: ", ///< Texto fixo antes do valor do texto do estado do servidor: "Server: ".
            .suffix = "", ///< Nao escreve texto depois do valor do texto do estado do servidor.
            .fontFamily = &RobotoRegular10pt7b, ///< Fonte do texto do estado do servidor: RobotoRegular10pt7b.
            .datum = TC_DATUM, ///< No texto do estado do servidor, alinha o texto pelo centro superior.
            .fontColor = CFK_GREY14, ///< Cor da fonte do texto do estado do servidor: CFK_GREY14.
            .backgroundColor = CFK_GREY5, ///< Cor de fundo do texto do estado do servidor: CFK_GREY5.
        }; ///< Fecha a ficha do texto do estado do servidor.
    labelservstat.setup(configLabel7); ///< Aplica a ficha configLabel7 ao widget labelservstat.
    myDisplay.setLabel(arrayLabel,qtdLabel); ///< Registra os textos variaveis na DisplayWX.


    LedConfig configLed0 = { ///< Abre a ficha do LED de conexao.
            .radius = 16, ///< Raio do LED de conexao: 16 pixels.
            .colorOn = 0xF800, ///< Cor do LED aceso no inicio: vermelho de offline; o fluxo troca a cor conforme a sessao.
            .colorOff = CFK_GREY3, ///< Cor do LED de conexao quando apagado: CFK_GREY3.
            .initialState = true ///< O LED de conexao comeca aceso.
        }; ///< Fecha a ficha do LED de conexao.
    led1.setup(configLed0); ///< Aplica a ficha configLed0 ao widget led1.
    myDisplay.setLed(arrayLed,qtdLed); ///< Registra o LED de conexao na DisplayWX.


    LineChartConfig configLineChart0 = { ///< Abre a ficha do grafico da tela principal.
            .colorsSeries = colorsChart0, ///< Cores das series do grafico da tela principal, guardadas em colorsChart0.
            .subtitles = nullptr, ///< O grafico da tela principal nao desenha legenda das series.
            .font = &RobotoRegular5pt7b, ///< Fonte do grafico da tela principal: RobotoRegular5pt7b.
            .minValue = 0, ///< Menor valor da escala do grafico da tela principal: 0.
            .maxValue = 2000, ///< Maior valor da escala do grafico da tela principal: 2000.
            .width = 335, ///< Largura do grafico da tela principal: 335 pixels.
            .height = 139, ///< Altura do grafico da tela principal: 139 pixels.
            .gridColor = CFK_BLACK, ///< Cor da grade do grafico da tela principal: CFK_BLACK.
            .borderColor = CFK_BLACK, ///< Cor da borda do grafico da tela principal: CFK_BLACK.
            .backgroundColor = CFK_GREY3, ///< Cor de fundo do grafico da tela principal: CFK_GREY3.
            .textColor = CFK_WHITE, ///< Cor do texto do grafico da tela principal: CFK_WHITE.
            .verticalDivision = 10, ///< Divisoes verticais da grade do grafico da tela principal: 10.
            .maxPointsAmount = LineChart::SHOW_ALL, ///< O grafico da tela principal guarda todos os pontos recebidos.
            .amountSeries = qtdLinesChart0, ///< Quantidade de series do grafico da tela principal, vinda de qtdLinesChart0.
            .workInBackground = true, ///< O grafico da tela principal continua recebendo pontos mesmo fora da tela atual.
            .showZeroLine = true, ///< Desenha a linha de referencia do zero no grafico da tela principal.
            .boldLine = false, ///< Desenha o traco do grafico da tela principal com a espessura normal.
            .showDots = false ///< Nao marca um ponto em cada amostra do grafico da tela principal.
        }; ///< Fecha a ficha do grafico da tela principal.
    graphfreq.setup(configLineChart0); ///< Aplica a ficha configLineChart0 ao widget graphfreq.
    LineChartConfig configLineChart1 = { ///< Abre a ficha do grafico do programa 1.
            .colorsSeries = colorsChart1, ///< Cores das series do grafico do programa 1, guardadas em colorsChart1.
            .subtitles = nullptr, ///< O grafico do programa 1 nao desenha legenda das series.
            .font = &RobotoRegular5pt7b, ///< Fonte do grafico do programa 1: RobotoRegular5pt7b.
            .minValue = 0, ///< Menor valor da escala do grafico do programa 1: 0.
            .maxValue = 2000, ///< Maior valor da escala do grafico do programa 1: 2000.
            .width = 335, ///< Largura do grafico do programa 1: 335 pixels.
            .height = 185, ///< Altura do grafico do programa 1: 185 pixels.
            .gridColor = CFK_BLACK, ///< Cor da grade do grafico do programa 1: CFK_BLACK.
            .borderColor = CFK_BLACK, ///< Cor da borda do grafico do programa 1: CFK_BLACK.
            .backgroundColor = CFK_GREY3, ///< Cor de fundo do grafico do programa 1: CFK_GREY3.
            .textColor = CFK_WHITE, ///< Cor do texto do grafico do programa 1: CFK_WHITE.
            .verticalDivision = 10, ///< Divisoes verticais da grade do grafico do programa 1: 10.
            .maxPointsAmount = LineChart::SHOW_ALL, ///< O grafico do programa 1 guarda todos os pontos recebidos.
            .amountSeries = qtdLinesChart1, ///< Quantidade de series do grafico do programa 1, vinda de qtdLinesChart1.
            .workInBackground = false, ///< O grafico do programa 1 so e atualizado enquanto a sua tela esta visivel.
            .showZeroLine = true, ///< Desenha a linha de referencia do zero no grafico do programa 1.
            .boldLine = true, ///< Desenha o traco do grafico do programa 1 mais grosso.
            .showDots = false ///< Nao marca um ponto em cada amostra do grafico do programa 1.
        }; ///< Fecha a ficha do grafico do programa 1.
    graphfreqcopycopy.setup(configLineChart1); ///< Aplica a ficha configLineChart1 ao widget graphfreqcopycopy.
    LineChartConfig configLineChart2 = { ///< Abre a ficha do grafico do programa 2.
            .colorsSeries = colorsChart2, ///< Cores das series do grafico do programa 2, guardadas em colorsChart2.
            .subtitles = nullptr, ///< O grafico do programa 2 nao desenha legenda das series.
            .font = &RobotoRegular5pt7b, ///< Fonte do grafico do programa 2: RobotoRegular5pt7b.
            .minValue = 0, ///< Menor valor da escala do grafico do programa 2: 0.
            .maxValue = 2000, ///< Maior valor da escala do grafico do programa 2: 2000.
            .width = 335, ///< Largura do grafico do programa 2: 335 pixels.
            .height = 185, ///< Altura do grafico do programa 2: 185 pixels.
            .gridColor = CFK_BLACK, ///< Cor da grade do grafico do programa 2: CFK_BLACK.
            .borderColor = CFK_BLACK, ///< Cor da borda do grafico do programa 2: CFK_BLACK.
            .backgroundColor = CFK_GREY3, ///< Cor de fundo do grafico do programa 2: CFK_GREY3.
            .textColor = CFK_WHITE, ///< Cor do texto do grafico do programa 2: CFK_WHITE.
            .verticalDivision = 10, ///< Divisoes verticais da grade do grafico do programa 2: 10.
            .maxPointsAmount = LineChart::SHOW_ALL, ///< O grafico do programa 2 guarda todos os pontos recebidos.
            .amountSeries = qtdLinesChart2, ///< Quantidade de series do grafico do programa 2, vinda de qtdLinesChart2.
            .workInBackground = false, ///< O grafico do programa 2 so e atualizado enquanto a sua tela esta visivel.
            .showZeroLine = true, ///< Desenha a linha de referencia do zero no grafico do programa 2.
            .boldLine = true, ///< Desenha o traco do grafico do programa 2 mais grosso.
            .showDots = false ///< Nao marca um ponto em cada amostra do grafico do programa 2.
        }; ///< Fecha a ficha do grafico do programa 2.
    graphfreqcopycopycopy.setup(configLineChart2); ///< Aplica a ficha configLineChart2 ao widget graphfreqcopycopycopy.
    myDisplay.setLineChart(arrayLinechart,qtdLineChart); ///< Registra os tres graficos na DisplayWX.


    Numpad::m_backgroundColor = CFK_GREY3; ///< Define o fundo do teclado numerico das caixas.
    Numpad::m_letterColor = CFK_BLACK; ///< Define a cor das letras do teclado numerico.
    Numpad::m_keyColor = CFK_GREY13; ///< Define a cor das teclas do teclado numerico.


    NumberBoxConfig configNumberBox0 = { ///< Abre a ficha da caixa de tempo de espera do programa 1.
            .funcPtr = screen1, ///< Redesenha a tela do programa 1 quando o teclado da caixa de tempo de espera do programa 1 abre.
            .callback = pg1starttime_cb, ///< Chama pg1starttime_cb quando a caixa de tempo de espera do programa 1 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo de espera do programa 1: RobotoRegular10pt7b.
            .startValue = nb0_val, ///< Valor inicial da caixa de tempo de espera do programa 1, copiado de nb0_val.
            .width = 125, ///< Largura da caixa de tempo de espera do programa 1: 125 pixels.
            .height = 25, ///< Altura da caixa de tempo de espera do programa 1: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo de espera do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo de espera do programa 1: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo de espera do programa 1 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo de espera do programa 1.
    pg1starttime.setup(configNumberBox0); ///< Aplica a ficha configNumberBox0 ao widget pg1starttime.
    NumberBoxConfig configNumberBox1 = { ///< Abre a ficha da caixa de tempo ligado do programa 1.
            .funcPtr = screen1, ///< Redesenha a tela do programa 1 quando o teclado da caixa de tempo ligado do programa 1 abre.
            .callback = pg1runtime_cb, ///< Chama pg1runtime_cb quando a caixa de tempo ligado do programa 1 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo ligado do programa 1: RobotoRegular10pt7b.
            .startValue = nb1_val, ///< Valor inicial da caixa de tempo ligado do programa 1, copiado de nb1_val.
            .width = 127, ///< Largura da caixa de tempo ligado do programa 1: 127 pixels.
            .height = 25, ///< Altura da caixa de tempo ligado do programa 1: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo ligado do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo ligado do programa 1: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo ligado do programa 1 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo ligado do programa 1.
    pg1runtime.setup(configNumberBox1); ///< Aplica a ficha configNumberBox1 ao widget pg1runtime.
    NumberBoxConfig configNumberBox2 = { ///< Abre a ficha da caixa de tempo parado do programa 1.
            .funcPtr = screen1, ///< Redesenha a tela do programa 1 quando o teclado da caixa de tempo parado do programa 1 abre.
            .callback = pg1endtime_cb, ///< Chama pg1endtime_cb quando a caixa de tempo parado do programa 1 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo parado do programa 1: RobotoRegular10pt7b.
            .startValue = nb2_val, ///< Valor inicial da caixa de tempo parado do programa 1, copiado de nb2_val.
            .width = 127, ///< Largura da caixa de tempo parado do programa 1: 127 pixels.
            .height = 25, ///< Altura da caixa de tempo parado do programa 1: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo parado do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo parado do programa 1: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo parado do programa 1 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo parado do programa 1.
    pg1endtime.setup(configNumberBox2); ///< Aplica a ficha configNumberBox2 ao widget pg1endtime.
    NumberBoxConfig configNumberBox3 = { ///< Abre a ficha da caixa de frequencia maxima do programa 1.
            .funcPtr = screen1, ///< Redesenha a tela do programa 1 quando o teclado da caixa de frequencia maxima do programa 1 abre.
            .callback = pg1freqmax_cb, ///< Chama pg1freqmax_cb quando a caixa de frequencia maxima do programa 1 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de frequencia maxima do programa 1: RobotoRegular10pt7b.
            .startValue = nb3_val, ///< Valor inicial da caixa de frequencia maxima do programa 1, copiado de nb3_val.
            .width = 127, ///< Largura da caixa de frequencia maxima do programa 1: 127 pixels.
            .height = 25, ///< Altura da caixa de frequencia maxima do programa 1: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de frequencia maxima do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de frequencia maxima do programa 1: CFK_GREY6.
            .decimalPlaces = 2 ///< A caixa de frequencia maxima do programa 1 aceita 2 casas decimais.
        }; ///< Fecha a ficha da caixa de frequencia maxima do programa 1.
    pg1freqmax.setup(configNumberBox3); ///< Aplica a ficha configNumberBox3 ao widget pg1freqmax.
    NumberBoxConfig configNumberBox4 = { ///< Abre a ficha da caixa de RPM maximo do programa 1.
            .funcPtr = screen1, ///< Redesenha a tela do programa 1 quando o teclado da caixa de RPM maximo do programa 1 abre.
            .callback = pg1rpmmax_cb, ///< Chama pg1rpmmax_cb quando a caixa de RPM maximo do programa 1 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de RPM maximo do programa 1: RobotoRegular10pt7b.
            .startValue = nb4_val, ///< Valor inicial da caixa de RPM maximo do programa 1, copiado de nb4_val.
            .width = 129, ///< Largura da caixa de RPM maximo do programa 1: 129 pixels.
            .height = 25, ///< Altura da caixa de RPM maximo do programa 1: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de RPM maximo do programa 1: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de RPM maximo do programa 1: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de RPM maximo do programa 1 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de RPM maximo do programa 1.
    pg1rpmmax.setup(configNumberBox4); ///< Aplica a ficha configNumberBox4 ao widget pg1rpmmax.
    NumberBoxConfig configNumberBox5 = { ///< Abre a ficha da caixa de tempo de espera do programa 2.
            .funcPtr = screen2, ///< Redesenha a tela do programa 2 quando o teclado da caixa de tempo de espera do programa 2 abre.
            .callback = pg2starttime_cb, ///< Chama pg2starttime_cb quando a caixa de tempo de espera do programa 2 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo de espera do programa 2: RobotoRegular10pt7b.
            .startValue = nb5_val, ///< Valor inicial da caixa de tempo de espera do programa 2, copiado de nb5_val.
            .width = 125, ///< Largura da caixa de tempo de espera do programa 2: 125 pixels.
            .height = 25, ///< Altura da caixa de tempo de espera do programa 2: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo de espera do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo de espera do programa 2: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo de espera do programa 2 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo de espera do programa 2.
    pg2starttime.setup(configNumberBox5); ///< Aplica a ficha configNumberBox5 ao widget pg2starttime.
    NumberBoxConfig configNumberBox6 = { ///< Abre a ficha da caixa de tempo ligado do programa 2.
            .funcPtr = screen2, ///< Redesenha a tela do programa 2 quando o teclado da caixa de tempo ligado do programa 2 abre.
            .callback = pg2runtime_cb, ///< Chama pg2runtime_cb quando a caixa de tempo ligado do programa 2 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo ligado do programa 2: RobotoRegular10pt7b.
            .startValue = nb6_val, ///< Valor inicial da caixa de tempo ligado do programa 2, copiado de nb6_val.
            .width = 125, ///< Largura da caixa de tempo ligado do programa 2: 125 pixels.
            .height = 25, ///< Altura da caixa de tempo ligado do programa 2: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo ligado do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo ligado do programa 2: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo ligado do programa 2 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo ligado do programa 2.
    pg2runtime.setup(configNumberBox6); ///< Aplica a ficha configNumberBox6 ao widget pg2runtime.
    NumberBoxConfig configNumberBox7 = { ///< Abre a ficha da caixa de tempo parado do programa 2.
            .funcPtr = screen2, ///< Redesenha a tela do programa 2 quando o teclado da caixa de tempo parado do programa 2 abre.
            .callback = pg2stoptime_cb, ///< Chama pg2stoptime_cb quando a caixa de tempo parado do programa 2 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de tempo parado do programa 2: RobotoRegular10pt7b.
            .startValue = nb7_val, ///< Valor inicial da caixa de tempo parado do programa 2, copiado de nb7_val.
            .width = 124, ///< Largura da caixa de tempo parado do programa 2: 124 pixels.
            .height = 25, ///< Altura da caixa de tempo parado do programa 2: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de tempo parado do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de tempo parado do programa 2: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de tempo parado do programa 2 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de tempo parado do programa 2.
    pg2stoptime.setup(configNumberBox7); ///< Aplica a ficha configNumberBox7 ao widget pg2stoptime.
    NumberBoxConfig configNumberBox8 = { ///< Abre a ficha da caixa de frequencia maxima do programa 2.
            .funcPtr = screen2, ///< Redesenha a tela do programa 2 quando o teclado da caixa de frequencia maxima do programa 2 abre.
            .callback = pg2freqmax_cb, ///< Chama pg2freqmax_cb quando a caixa de frequencia maxima do programa 2 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de frequencia maxima do programa 2: RobotoRegular10pt7b.
            .startValue = nb8_val, ///< Valor inicial da caixa de frequencia maxima do programa 2, copiado de nb8_val.
            .width = 125, ///< Largura da caixa de frequencia maxima do programa 2: 125 pixels.
            .height = 25, ///< Altura da caixa de frequencia maxima do programa 2: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de frequencia maxima do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de frequencia maxima do programa 2: CFK_GREY6.
            .decimalPlaces = 2 ///< A caixa de frequencia maxima do programa 2 aceita 2 casas decimais.
        }; ///< Fecha a ficha da caixa de frequencia maxima do programa 2.
    pg2freqmax.setup(configNumberBox8); ///< Aplica a ficha configNumberBox8 ao widget pg2freqmax.
    NumberBoxConfig configNumberBox9 = { ///< Abre a ficha da caixa de RPM maximo do programa 2.
            .funcPtr = screen2, ///< Redesenha a tela do programa 2 quando o teclado da caixa de RPM maximo do programa 2 abre.
            .callback = pg2rpmmax_cb, ///< Chama pg2rpmmax_cb quando a caixa de RPM maximo do programa 2 e alterada.
            .font = &RobotoRegular10pt7b, ///< Fonte da caixa de RPM maximo do programa 2: RobotoRegular10pt7b.
            .startValue = nb9_val, ///< Valor inicial da caixa de RPM maximo do programa 2, copiado de nb9_val.
            .width = 125, ///< Largura da caixa de RPM maximo do programa 2: 125 pixels.
            .height = 25, ///< Altura da caixa de RPM maximo do programa 2: 25 pixels.
            .letterColor = CFK_WHITE, ///< Cor dos caracteres da caixa de RPM maximo do programa 2: CFK_WHITE.
            .backgroundColor = CFK_GREY6, ///< Cor de fundo da caixa de RPM maximo do programa 2: CFK_GREY6.
            .decimalPlaces = 0 ///< A caixa de RPM maximo do programa 2 aceita somente numeros inteiros.
        }; ///< Fecha a ficha da caixa de RPM maximo do programa 2.
    pg2rpmmax.setup(configNumberBox9); ///< Aplica a ficha configNumberBox9 ao widget pg2rpmmax.
    myDisplay.setNumberbox(arrayNumberbox,qtdNumberbox); ///< Registra as caixas numericas na DisplayWX.


    TextButtonConfig configTextButton0 = { ///< Abre a ficha do botao START.
            .text = "START", ///< Texto inicial do botao START: "START".
            .callback = startbtn_cb, ///< Chama startbtn_cb quando o botao START e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao START: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao START: 160 pixels.
            .height = 50, ///< Altura do botao START: 50 pixels.
            .radius = 10, ///< Raio do botao START: 10 pixels.
            .backgroundColor = CFK_COLOR025, ///< Cor de fundo do botao START: CFK_COLOR025.
            .textColor = CFK_BLACK ///< Cor do texto do botao START: CFK_BLACK.
        }; ///< Fecha a ficha do botao START.

    startbtn.setup(configTextButton0); ///< Aplica a ficha configTextButton0 ao widget startbtn.
    TextButtonConfig configTextButton1 = { ///< Abre a ficha do botao REVERSE.
            .text = "REVERSE", ///< Texto inicial do botao REVERSE: "REVERSE".
            .callback = reversebtn_cb, ///< Chama reversebtn_cb quando o botao REVERSE e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao REVERSE: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao REVERSE: 160 pixels.
            .height = 50, ///< Altura do botao REVERSE: 50 pixels.
            .radius = 10, ///< Raio do botao REVERSE: 10 pixels.
            .backgroundColor = CFK_COLOR050, ///< Cor de fundo do botao REVERSE: CFK_COLOR050.
            .textColor = CFK_BLACK ///< Cor do texto do botao REVERSE: CFK_BLACK.
        }; ///< Fecha a ficha do botao REVERSE.

    reversebtn.setup(configTextButton1); ///< Aplica a ficha configTextButton1 ao widget reversebtn.
    TextButtonConfig configTextButton2 = { ///< Abre a ficha do botao EMERGENCY.
            .text = "EMERGENCY", ///< Texto inicial do botao EMERGENCY: "EMERGENCY".
            .callback = emergcybtn_cb, ///< Chama emergcybtn_cb quando o botao EMERGENCY e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao EMERGENCY: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao EMERGENCY: 160 pixels.
            .height = 50, ///< Altura do botao EMERGENCY: 50 pixels.
            .radius = 10, ///< Raio do botao EMERGENCY: 10 pixels.
            .backgroundColor = CFK_COLOR007, ///< Cor de fundo do botao EMERGENCY: CFK_COLOR007.
            .textColor = CFK_BLACK ///< Cor do texto do botao EMERGENCY: CFK_BLACK.
        }; ///< Fecha a ficha do botao EMERGENCY.

    emergcybtn.setup(configTextButton2); ///< Aplica a ficha configTextButton2 ao widget emergcybtn.
    TextButtonConfig configTextButton3 = { ///< Abre a ficha do botao STOP da tela principal.
            .text = "STOP", ///< Texto inicial do botao STOP da tela principal: "STOP".
            .callback = stopbtn_cb, ///< Chama stopbtn_cb quando o botao STOP da tela principal e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao STOP da tela principal: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao STOP da tela principal: 160 pixels.
            .height = 50, ///< Altura do botao STOP da tela principal: 50 pixels.
            .radius = 10, ///< Raio do botao STOP da tela principal: 10 pixels.
            .backgroundColor = CFK_COLOR019, ///< Cor de fundo do botao STOP da tela principal: CFK_COLOR019.
            .textColor = CFK_BLACK ///< Cor do texto do botao STOP da tela principal: CFK_BLACK.
        }; ///< Fecha a ficha do botao STOP da tela principal.

    stopbtn.setup(configTextButton3); ///< Aplica a ficha configTextButton3 ao widget stopbtn.
    TextButtonConfig configTextButton4 = { ///< Abre a ficha do botao PROG 1 da tela principal.
            .text = "PROG 1", ///< Texto inicial do botao PROG 1 da tela principal: "PROG 1".
            .callback = prog1btn_cb, ///< Chama prog1btn_cb quando o botao PROG 1 da tela principal e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao PROG 1 da tela principal: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao PROG 1 da tela principal: 160 pixels.
            .height = 50, ///< Altura do botao PROG 1 da tela principal: 50 pixels.
            .radius = 10, ///< Raio do botao PROG 1 da tela principal: 10 pixels.
            .backgroundColor = CFK_COLOR013, ///< Cor de fundo do botao PROG 1 da tela principal: CFK_COLOR013.
            .textColor = CFK_BLACK ///< Cor do texto do botao PROG 1 da tela principal: CFK_BLACK.
        }; ///< Fecha a ficha do botao PROG 1 da tela principal.

    prog1btn.setup(configTextButton4); ///< Aplica a ficha configTextButton4 ao widget prog1btn.
    TextButtonConfig configTextButton5 = { ///< Abre a ficha do botao PROG 2 da tela principal.
            .text = "PROG 2", ///< Texto inicial do botao PROG 2 da tela principal: "PROG 2".
            .callback = prog2btn_cb, ///< Chama prog2btn_cb quando o botao PROG 2 da tela principal e alterado.
            .fontFamily = &RobotoBold13pt7b, ///< Fonte do botao PROG 2 da tela principal: RobotoBold13pt7b.
            .width = 160, ///< Largura do botao PROG 2 da tela principal: 160 pixels.
            .height = 50, ///< Altura do botao PROG 2 da tela principal: 50 pixels.
            .radius = 10, ///< Raio do botao PROG 2 da tela principal: 10 pixels.
            .backgroundColor = CFK_COLOR013, ///< Cor de fundo do botao PROG 2 da tela principal: CFK_COLOR013.
            .textColor = CFK_BLACK ///< Cor do texto do botao PROG 2 da tela principal: CFK_BLACK.
        }; ///< Fecha a ficha do botao PROG 2 da tela principal.

    prog2btn.setup(configTextButton5); ///< Aplica a ficha configTextButton5 ao widget prog2btn.
    TextButtonConfig configTextButton6 = { ///< Abre a ficha do botao que inicia o programa 1.
            .text = "PROG 1", ///< Texto inicial do botao que inicia o programa 1: "PROG 1".
            .callback = prog1startbtn_cb, ///< Chama prog1startbtn_cb quando o botao que inicia o programa 1 e alterado.
            .fontFamily = &RobotoBold14pt7b, ///< Fonte do botao que inicia o programa 1: RobotoBold14pt7b.
            .width = 233, ///< Largura do botao que inicia o programa 1: 233 pixels.
            .height = 80, ///< Altura do botao que inicia o programa 1: 80 pixels.
            .radius = 10, ///< Raio do botao que inicia o programa 1: 10 pixels.
            .backgroundColor = CFK_COLOR013, ///< Cor de fundo do botao que inicia o programa 1: CFK_COLOR013.
            .textColor = CFK_BLACK ///< Cor do texto do botao que inicia o programa 1: CFK_BLACK.
        }; ///< Fecha a ficha do botao que inicia o programa 1.

    prog1startbtn.setup(configTextButton6); ///< Aplica a ficha configTextButton6 ao widget prog1startbtn.
    TextButtonConfig configTextButton7 = { ///< Abre a ficha do botao STOP do programa 1.
            .text = "STOP", ///< Texto inicial do botao STOP do programa 1: "STOP".
            .callback = stop1btn_cb, ///< Chama stop1btn_cb quando o botao STOP do programa 1 e alterado.
            .fontFamily = &RobotoBold14pt7b, ///< Fonte do botao STOP do programa 1: RobotoBold14pt7b.
            .width = 233, ///< Largura do botao STOP do programa 1: 233 pixels.
            .height = 80, ///< Altura do botao STOP do programa 1: 80 pixels.
            .radius = 10, ///< Raio do botao STOP do programa 1: 10 pixels.
            .backgroundColor = CFK_COLOR007, ///< Cor de fundo do botao STOP do programa 1: CFK_COLOR007.
            .textColor = CFK_WHITE ///< Cor do texto do botao STOP do programa 1: CFK_WHITE.
        }; ///< Fecha a ficha do botao STOP do programa 1.

    stop1btn.setup(configTextButton7); ///< Aplica a ficha configTextButton7 ao widget stop1btn.
    TextButtonConfig configTextButton8 = { ///< Abre a ficha do botao que inicia o programa 2.
            .text = "PROG 2", ///< Texto inicial do botao que inicia o programa 2: "PROG 2".
            .callback = prog2startbtn_cb, ///< Chama prog2startbtn_cb quando o botao que inicia o programa 2 e alterado.
            .fontFamily = &RobotoBold14pt7b, ///< Fonte do botao que inicia o programa 2: RobotoBold14pt7b.
            .width = 233, ///< Largura do botao que inicia o programa 2: 233 pixels.
            .height = 80, ///< Altura do botao que inicia o programa 2: 80 pixels.
            .radius = 10, ///< Raio do botao que inicia o programa 2: 10 pixels.
            .backgroundColor = CFK_COLOR013, ///< Cor de fundo do botao que inicia o programa 2: CFK_COLOR013.
            .textColor = CFK_BLACK ///< Cor do texto do botao que inicia o programa 2: CFK_BLACK.
        }; ///< Fecha a ficha do botao que inicia o programa 2.

    prog2startbtn.setup(configTextButton8); ///< Aplica a ficha configTextButton8 ao widget prog2startbtn.
    TextButtonConfig configTextButton9 = { ///< Abre a ficha do botao STOP do programa 2.
            .text = "STOP", ///< Texto inicial do botao STOP do programa 2: "STOP".
            .callback = stop2btn_cb, ///< Chama stop2btn_cb quando o botao STOP do programa 2 e alterado.
            .fontFamily = &RobotoBold14pt7b, ///< Fonte do botao STOP do programa 2: RobotoBold14pt7b.
            .width = 233, ///< Largura do botao STOP do programa 2: 233 pixels.
            .height = 80, ///< Altura do botao STOP do programa 2: 80 pixels.
            .radius = 10, ///< Raio do botao STOP do programa 2: 10 pixels.
            .backgroundColor = CFK_COLOR007, ///< Cor de fundo do botao STOP do programa 2: CFK_COLOR007.
            .textColor = CFK_WHITE ///< Cor do texto do botao STOP do programa 2: CFK_WHITE.
        }; ///< Fecha a ficha do botao STOP do programa 2.

    stop2btn.setup(configTextButton9); ///< Aplica a ficha configTextButton9 ao widget stop2btn.
    myDisplay.setTextButton(arrayTextButton,qtdTextButton); ///< Registra os botoes de texto na DisplayWX.


    VerticalAnalogConfig configVAnalog0 = { ///< Abre a ficha da barra de tensao DC.
            .minValue = 0, ///< Menor valor da escala da barra de tensao DC: 0.
            .maxValue = 400, ///< Maior valor da escala da barra de tensao DC: 400.
            .width = 40, ///< Largura da barra de tensao DC: 40 pixels.
            .height = 135, ///< Altura da barra de tensao DC: 135 pixels.
            .arrowColor = CFK_COLOR007, ///< Cor da seta da barra de tensao DC: CFK_COLOR007.
            .textColor = CFK_BLACK, ///< Cor do texto da barra de tensao DC: CFK_BLACK.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo da barra de tensao DC: CFK_WHITE.
            .borderColor = CFK_BLACK, ///< Cor da borda da barra de tensao DC: CFK_BLACK.
            .steps = 10 ///< Quantidade de marcas da barra de tensao DC: 10.
        }; ///< Fecha a ficha da barra de tensao DC.
    analogvdc.setup(configVAnalog0); ///< Aplica a ficha configVAnalog0 ao widget analogvdc.
    VerticalAnalogConfig configVAnalog1 = { ///< Abre a ficha da barra de tensao de saida.
            .minValue = 0, ///< Menor valor da escala da barra de tensao de saida: 0.
            .maxValue = 270, ///< Maior valor da escala da barra de tensao de saida: 270.
            .width = 40, ///< Largura da barra de tensao de saida: 40 pixels.
            .height = 134, ///< Altura da barra de tensao de saida: 134 pixels.
            .arrowColor = CFK_COLOR007, ///< Cor da seta da barra de tensao de saida: CFK_COLOR007.
            .textColor = CFK_BLACK, ///< Cor do texto da barra de tensao de saida: CFK_BLACK.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo da barra de tensao de saida: CFK_WHITE.
            .borderColor = CFK_BLACK, ///< Cor da borda da barra de tensao de saida: CFK_BLACK.
            .steps = 10 ///< Quantidade de marcas da barra de tensao de saida: 10.
        }; ///< Fecha a ficha da barra de tensao de saida.
    analogtout.setup(configVAnalog1); ///< Aplica a ficha configVAnalog1 ao widget analogtout.
    VerticalAnalogConfig configVAnalog2 = { ///< Abre a ficha da barra de corrente.
            .minValue = 0, ///< Menor valor da escala da barra de corrente: 0.
            .maxValue = 10, ///< Maior valor da escala da barra de corrente: 10.
            .width = 40, ///< Largura da barra de corrente: 40 pixels.
            .height = 134, ///< Altura da barra de corrente: 134 pixels.
            .arrowColor = CFK_COLOR007, ///< Cor da seta da barra de corrente: CFK_COLOR007.
            .textColor = CFK_BLACK, ///< Cor do texto da barra de corrente: CFK_BLACK.
            .backgroundColor = CFK_WHITE, ///< Cor de fundo da barra de corrente: CFK_WHITE.
            .borderColor = CFK_BLACK, ///< Cor da borda da barra de corrente: CFK_BLACK.
            .steps = 10 ///< Quantidade de marcas da barra de corrente: 10.
        }; ///< Fecha a ficha da barra de corrente.
    analogcur.setup(configVAnalog2); ///< Aplica a ficha configVAnalog2 ao widget analogcur.
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog); ///< Registra as barras analogicas na DisplayWX.


    ImageFromPixelsConfig configImage1 = { ///< Abre a ficha da imagem de voltar do programa 1.
            .pixels = ipg1backPixels, ///< Bitmap RGB565 da imagem de voltar do programa 1, vindo de ipg1backPixels.
            .maskAlpha = ipg1backMask, ///< Mascara de transparencia da imagem de voltar do programa 1, vinda de ipg1backMask.
            .cb = iipg1back_cb, ///< Chama iipg1back_cb ao tocar a imagem de voltar do programa 1.
            .angle = 0, ///< A imagem de voltar do programa 1 fica sem rotacao.
            .width = ipg1backW, ///< Largura da imagem de voltar do programa 1: ipg1backW pixels.
            .height = ipg1backH, ///< Altura da imagem de voltar do programa 1: ipg1backH pixels.
            .backgroundColor = CFK_GREY6 ///< Cor de fundo da imagem de voltar do programa 1: CFK_GREY6.
        }; ///< Fecha a ficha da imagem de voltar do programa 1.
    iipg1back.setupFromPixels(configImage1); ///< Aplica a ficha configImage1 ao widget iipg1back.
    ImageFromPixelsConfig configImage2 = { ///< Abre a ficha da imagem de voltar do programa 2.
            .pixels = ipg2backPixels, ///< Bitmap RGB565 da imagem de voltar do programa 2, vindo de ipg2backPixels.
            .maskAlpha = ipg2backMask, ///< Mascara de transparencia da imagem de voltar do programa 2, vinda de ipg2backMask.
            .cb = iipg2back_cb, ///< Chama iipg2back_cb ao tocar a imagem de voltar do programa 2.
            .angle = 0, ///< A imagem de voltar do programa 2 fica sem rotacao.
            .width = ipg2backW, ///< Largura da imagem de voltar do programa 2: ipg2backW pixels.
            .height = ipg2backH, ///< Altura da imagem de voltar do programa 2: ipg2backH pixels.
            .backgroundColor = CFK_GREY6 ///< Cor de fundo da imagem de voltar do programa 2: CFK_GREY6.
        }; ///< Fecha a ficha da imagem de voltar do programa 2.
    iipg2back.setupFromPixels(configImage2); ///< Aplica a ficha configImage2 ao widget iipg2back.
    myDisplay.setImage(arrayImagem,qtdImagem); ///< Registra as imagens de voltar na DisplayWX.


} ///< Termina a montagem dos widgets.

/**
 * @brief Callback do slider geral: guarda o percentual e solicita o ajuste manual do motor.
 */
void slidergeral_cb(){
    hslider0_val = slidergeral.getValue();
    Fluxo::atualizarSliderManual(hslider0_val); // [MANUAL]
}
/**
 * @brief Callback do slider do programa 1: guarda o percentual e atualiza o texto da tela.
 */
void sliderprg1_cb(){
    hslider1_val = sliderprg1.getValue();
    rpmvalue.setText(String(hslider1_val)); // [MANUAL]
}
/**
 * @brief Callback do slider do programa 2: guarda o percentual e atualiza o texto da tela.
 */
void sliderprg2_cb(){
    hslider2_val = sliderprg2.getValue();
    rpmvalue6.setText(String(hslider2_val)); // [MANUAL]
}
/**
 * @brief Callback da caixa de tempo de espera do programa 1.
 */
void pg1starttime_cb(){

    nb0_val = pg1starttime.getValue();
}
/**
 * @brief Callback da caixa de tempo ligado do programa 1.
 */
void pg1runtime_cb(){

    nb1_val = pg1runtime.getValue();
}
/**
 * @brief Callback da caixa de tempo parado do programa 1.
 */
void pg1endtime_cb(){

    nb2_val = pg1endtime.getValue();
}
/**
 * @brief Callback da caixa de frequencia maxima do programa 1.
 */
void pg1freqmax_cb(){

    nb3_val = pg1freqmax.getValue();
}
/**
 * @brief Callback da caixa de RPM maximo do programa 1.
 */
void pg1rpmmax_cb(){

    nb4_val = pg1rpmmax.getValue();
}
/**
 * @brief Callback da caixa de tempo de espera do programa 2.
 */
void pg2starttime_cb(){

    nb5_val = pg2starttime.getValue();
}
/**
 * @brief Callback da caixa de tempo ligado do programa 2.
 */
void pg2runtime_cb(){

    nb6_val = pg2runtime.getValue();
}
/**
 * @brief Callback da caixa de tempo parado do programa 2.
 */
void pg2stoptime_cb(){

    nb7_val = pg2stoptime.getValue();
}
/**
 * @brief Callback da caixa de frequencia maxima do programa 2.
 */
void pg2freqmax_cb(){

    nb8_val = pg2freqmax.getValue();
}
/**
 * @brief Callback da caixa de RPM maximo do programa 2.
 */
void pg2rpmmax_cb(){

    nb9_val = pg2rpmmax.getValue();
}
/**
 * @brief Callback do botao de partida: pede o comando START ao motor.
 */
void startbtn_cb(){
    Fluxo::sendCommandToMotor("START"); // [MANUAL]
    Serial.println("Clicked on: startbtn_cb");
}
/**
 * @brief Callback do botao de reversao: pede o comando REVERSE ao motor.
 */
void reversebtn_cb(){
    Fluxo::sendCommandToMotor("REVERSE"); // [MANUAL]
    Serial.println("Clicked on: reversebtn_cb");
}
/**
 * @brief Callback do botao de emergencia: pede o comando EMERGENCY ao motor.
 */
void emergcybtn_cb(){
    Fluxo::sendCommandToMotor("EMERGENCY"); // [MANUAL]
    Serial.println("Clicked on: emergcybtn_cb");
}
/**
 * @brief Callback do botao de parada: pede o comando STOP ao motor.
 */
void stopbtn_cb(){
    Fluxo::sendCommandToMotor("STOP"); // [MANUAL]
    Serial.println("Clicked on: stopbtn_cb");
}
/**
 * @brief Callback que abre a tela de configuracao do programa 1.
 */
void prog1btn_cb(){
    myDisplay.loadScreen(screen1);
    Serial.println("Clicked on: prog1btn_cb");
}
/**
 * @brief Callback que abre a tela de configuracao do programa 2.
 */
void prog2btn_cb(){
    myDisplay.loadScreen(screen2);
    Serial.println("Clicked on: prog2btn_cb");
}
/**
 * @brief Callback que inicia o ciclo local do programa 1.
 */
void prog1startbtn_cb(){
    Fluxo::progStart(1); // [MANUAL]
    Serial.println("Clicked on: prog1startbtn_cb");
}
/**
 * @brief Callback que interrompe o programa em andamento a partir da tela 1.
 */
void stop1btn_cb(){
    Fluxo::progStop(); // [MANUAL]
    ESP_LOGI("MAIN", "Clicked on: stop1btn_cb"); // [MANUAL]
}
/**
 * @brief Callback que inicia o ciclo local do programa 2.
 */
void prog2startbtn_cb(){
    Fluxo::progStart(2); // [MANUAL]
    Serial.println("Clicked on: prog2startbtn_cb");
}
/**
 * @brief Callback que interrompe o programa em andamento a partir da tela 2.
 */
void stop2btn_cb(){
    Fluxo::progStop(); // [MANUAL]
    ESP_LOGI("MAIN", "Clicked on: stop2btn_cb"); // [MANUAL]
}
/**
 * @brief Callback da imagem de voltar do programa 1: retorna a tela principal.
 */
void iipg1back_cb(){
    myDisplay.loadScreen(screen0);
    Serial.println("Image ipg1back clicked");
}
/**
 * @brief Callback da imagem de voltar do programa 2: retorna a tela principal.
 */
void iipg2back_cb(){
    myDisplay.loadScreen(screen0);
    Serial.println("Image ipg2back clicked");
}
