// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. [manual] = escrita depois (seno/cosseno, spinbox, toggles).
// [legenda] MCU ESP32, painel ILI9488 SPI 480x320, toque FT6236U, sem camera.
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] SPI.h para o host HSPI do ILI9488 no ESP32 classico.
#include <SPI.h>
// [EdgeWX] Arduino_GFX: Arduino_HWSPI, Arduino_ILI9488_18bit e primitivas de desenho.
#include <Arduino_GFX_Library.h>
// [EdgeWX] HAL DisplayWX: widgets, toque FT6236U e a tarefa de UI.
#include <displaywx.h>


// [EdgeWX] Ficha da placa escolhida no EdgeWX. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32
    * Display: ILI9488
    * Touch: FT6236U
    * Camera: None
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e usado neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; aqui ninguem le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura util do painel com rotacao 0: 480 px.
const int DISPLAY_W = 480;
// [EdgeWX] Altura util do painel com rotacao 0: 320 px.
const int DISPLAY_H = 320;
// [EdgeWX] Clock SPI do ILI9488 em 27 MHz, passado a tft->begin.
const int DISP_FREQUENCY = 27000000;
// [EdgeWX] Canto minimo X do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto maximo X do mapa; 480 casa com a largura do painel.
const int TOUCH_MAP_X1 = 480;
// [EdgeWX] Canto minimo Y do mapa de toque.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto maximo Y do mapa; 320 casa com a altura do painel.
const int TOUCH_MAP_Y1 = 320;
// [EdgeWX] false: nao troca X e Y do FT6236U (rotacao 0).
const bool TOUCH_SWAP_XY = true;
// [EdgeWX] false: nao espelha o eixo X do toque.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] false: nao espelha o eixo Y do toque.
const bool TOUCH_INVERT_Y = true;
// [EdgeWX] GPIO MOSI do ILI9488 no ESP32.
const int DISP_MOSI = 11;
// [EdgeWX] MISO desligado (-1); o painel e write-only neste desenho.
const int DISP_MISO = -1;
// [EdgeWX] GPIO SCLK do SPI do display.
const int DISP_SCLK = 12;
// [EdgeWX] Chip select do ILI9488.
const int DISP_CS = 10;
// [EdgeWX] Data/Command do ILI9488.
const int DISP_DC = 8;
// [EdgeWX] Reset do painel ILI9488.
const int DISP_RST = 9;
// [EdgeWX] SCL I2C do FT6236U.
const int TOUCH_SCL = 14;
// [EdgeWX] SDA I2C do FT6236U.
const int TOUCH_SDA = 13;
// [EdgeWX] Rotacao 0 (retrato/paisagem nativa). O comentario em ingles e do gerador.
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] false: painel nao IPS; entra no construtor SPI. O typo "Come" e do gerador.
const bool isIPS = false; // Come display can use this as bigEndian flag

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Unica tela desenhada no editor. Indice 0.
void screen0();
// [EdgeWX] Aplica LineChartConfig, SpinBoxConfig e ToggleButtonConfig.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do ToggleButton tgverde.
void tgverde_cb();
// [EdgeWX] Callback do ToggleButton tglaranja.
void tglaranja_cb();
// [EdgeWX] Callback do SpinBox spinbox.
void spinbox_cb();

// [EdgeWX] Comentario do gerador abrindo o SPI global.
// Create global SPI object
// [EdgeWX] SPIClass no host HSPI do ESP32 classico, compartilhado com o HWSPI.
SPIClass spiPrimary(HSPI);
// [EdgeWX] Ponteiro do barramento Arduino_HWSPI. Preenchido no setup.
Arduino_DataBus *bus = nullptr;
// [EdgeWX] Ponteiro do Arduino_ILI9488_18bit. screen0 e os widgets desenham nele.
Arduino_GFX *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque e a tarefa de UI.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] LineChart em (65, 165), tela 0: grafico de duas series.
LineChart grafico(65, 165, 0);
// [EdgeWX] Quantidade de LineCharts passada a setLineChart.
const uint8_t qtdLineChart = 1;
// [EdgeWX] Array de ponteiros exigido por myDisplay.setLineChart.
LineChart *arrayLinechart[qtdLineChart] = {&grafico};
// [EdgeWX] Duas series no grafico 0.
const uint8_t qtdLinesChart0 = 2;
// [EdgeWX] Cores das series 0 e 1 do LineChart.
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR031,CFK_COLOR013};
// [EdgeWX] SpinBox em (140, 85), tela 0.
SpinBox spinbox(140, 85, 0);
// [EdgeWX] Um SpinBox neste export.
const uint8_t qtdSpinbox = 1;
// [EdgeWX] Array de ponteiros para setSpinbox.
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox};
// [EdgeWX] ToggleButton verde em (145, 20), tela 0.
ToggleButton tgverde(145, 20, 0);
// [EdgeWX] ToggleButton laranja em (255, 20), tela 0.
ToggleButton tglaranja(255, 20, 0);
// [EdgeWX] Dois toggles registrados em setToggle.
const uint8_t qtdToggleBtn = 2;
// [EdgeWX] Array na ordem tgverde, tglaranja.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&tgverde, &tglaranja};
// [EdgeWX] Espelho do estado de tgverde; o comentario em ingles e do gerador.
bool  switch0_val = false; // Global variable that stores the value of the widget tgverde
// [EdgeWX] Espelho do estado de tglaranja; o comentario em ingles e do gerador.
bool  switch1_val = false; // Global variable that stores the value of the widget tglaranja


// [manual] Angulo em graus acumulado a cada ciclo do loop.
int contador = 0;
// [manual] Fator graus -> radianos (PI/180). Nome minusculo para nao colidir com o macro do Arduino.
const float deg_to_rad = 0.017453292519943295f;
// [manual] Amplitude do grafico; a callback do SpinBox grava aqui.
float amplitudeSpinbox = 0.0f;
// [manual] Ultimo seno enviado a serie 0; reusado com tgverde desligado.
float ultimoSeno = 0.0f;
// [manual] Ultimo cosseno enviado a serie 1; reusado com tglaranja desligado.
float ultimoCosseno = 0.0f;

// [EdgeWX] setup: sobe Serial, SPI, ILI9488, toque FT6236U, widgets e a tarefa de UI.
void setup(){

    // [EdgeWX] Abre a Serial a 115200 baud para logs do export.
    Serial.begin(115200);
    // [EdgeWX] Comentario do gerador antes de spiPrimary.begin.
    // Start SPI object for display
    // [EdgeWX] Inicializa HSPI com SCLK, MISO e MOSI do display.
    spiPrimary.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    // [EdgeWX] Cria o Arduino_HWSPI com DC, CS e o SPIClass acima.
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spiPrimary);
    // [EdgeWX] Instancia ILI9488 18-bit com RST, rotacao 0 e isIPS false.
    tft = new Arduino_ILI9488_18bit(bus, DISP_RST, rotationScreen, isIPS);
    // [EdgeWX] Liga o painel a 27 MHz.
    tft->begin(DISP_FREQUENCY);
    // [EdgeWX] Entrega o Arduino_GFX ao DisplayWX para desenho. Comentario ingles do gerador.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Comentario do gerador abrindo o bloco de toque.
    // Setup touch
    // [EdgeWX] Define os quatro cantos do mapa de toque.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Aplica (ou nao) inversao dos eixos X/Y.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Aplica (ou nao) a troca XY do toque.
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o FT6236U com SDA/SCL; INT e RST em -1 (sem pino dedicado).
    myDisplay.startTouchFT6236U(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, -1, -1);
    // [EdgeWX] enableTouchLog fica comentado neste export; o gerador deixa a opcao desligada.
    // myDisplay.enableTouchLog();
    // [EdgeWX] Configura cada widget. O typo "individualy" e do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Carrega a tela 0. Comentario ingles do gerador.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] createTask(true, 3): liga o watchdog; o 3 fica guardado e nao e usado.
    myDisplay.createTask(true, 3); // Initialize the task to read touch and draw
    // [EdgeWX] Registra a task atual no watchdog; neste export a chamada ficou ligada.
    esp_task_wdt_add(NULL);
}

// [manual] loop: seno/cosseno * amplitude; toggles congelam cada serie no ultimo valor.
void loop(){

    // [EdgeWX] Abre a janela de desenho customizado do DisplayWX.
    myDisplay.startCustomDraw();
    // [manual] Angulo atual em radianos: contador (graus) * deg_to_rad.
    float anguloRad = contador * deg_to_rad;
    // [manual] Seno do angulo escalado pela amplitude do SpinBox.
    float valorSeno = sin(anguloRad) * amplitudeSpinbox;
    // [manual] Cosseno do angulo escalado pela amplitude do SpinBox.
    float valorCosseno = cos(anguloRad) * amplitudeSpinbox;
    // [manual] tgverde ligado: atualiza o seno; desligado: mantem ultimoSeno.
    if(switch0_val){
        ultimoSeno = valorSeno;
    }
    // [manual] tglaranja ligado: atualiza o cosseno; desligado: mantem ultimoCosseno.
    if(switch1_val){
        ultimoCosseno = valorCosseno;
    }
    // [manual] Serie 0 (1a serie): seno novo ou congelado.
    grafico.push(0, ultimoSeno); //Use this line to add value in serie 0.
    // [manual] Serie 1 (2a serie): cosseno novo ou congelado.
    grafico.push(1, ultimoCosseno); //Use this line to add value in serie 1.
    // [manual] Avanca 1 grau por ciclo para a proxima amostra.
    contador++;
    // [EdgeWX] Fecha a janela de desenho customizado.
    myDisplay.finishCustomDraw();
    // [EdgeWX] Alimenta o watchdog da loop task. Comentario ingles do gerador.
    esp_task_wdt_reset(); // Reset watchdog
    // [EdgeWX] Pausa curta apos o reset do WDT.
    delay(100);
}

// [EdgeWX] Desenha o fundo e os widgets da tela 0.
void screen0(){

    // [EdgeWX] Preenche o fundo com CFK_COLOR048.
    myDisplay.setBackgroundColor(CFK_COLOR048);
    // [EdgeWX] Comentario do gerador listando um toggleButton na tela.
    //This screen has a/an toggleButton
    // [EdgeWX] Comentario do gerador listando o grafico na tela.
    //This screen has a/an grafico
    // [EdgeWX] Comentario do gerador listando o segundo toggleButton.
    //This screen has a/an toggleButton
    // [EdgeWX] Comentario do gerador listando o spinbox na tela.
    //This screen has a/an spinbox
    // [EdgeWX] Desenha todos os widgets registrados no indice de tela 0.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Titulo do gerador (typo "widgtes") antes de configurar cada widget.
// Configure each widgtes to be used
// [EdgeWX] Monta configs e registra LineChart, SpinBox e ToggleButtons no DisplayWX.
void loadWidgets(){

    // [EdgeWX] Struct de configuracao do LineChart 0 (cores, faixa, grade e series).
    LineChartConfig configLineChart0 = {
            // [EdgeWX] Ponteiro para as cores das duas series.
            .colorsSeries = colorsChart0,
            // [EdgeWX] Sem legendas de serie neste export.
            .subtitles = nullptr,
            // [EdgeWX] Fonte Roboto Regular 5pt para eixos/textos do grafico.
            .font = &RobotoRegular5pt7b,
            // [EdgeWX] Valor minimo do eixo Y: -100.
            .minValue = -100,
            // [EdgeWX] Valor maximo do eixo Y: 100.
            .maxValue = 100,
            // [EdgeWX] Largura do grafico em pixels.
            .width = 359,
            // [EdgeWX] Altura do grafico em pixels.
            .height = 147,
            // [EdgeWX] Cor da grade.
            .gridColor = CFK_BLACK,
            // [EdgeWX] Cor da borda.
            .borderColor = CFK_BLACK,
            // [EdgeWX] Cor de fundo do grafico.
            .backgroundColor = CFK_GREY3,
            // [EdgeWX] Cor do texto dos eixos.
            .textColor = CFK_WHITE,
            // [EdgeWX] Dez divisoes verticais na grade.
            .verticalDivision = 10,
            // [EdgeWX] SHOW_ALL: guarda todos os pontos empilhados.
            .maxPointsAmount = LineChart::SHOW_ALL,
            // [EdgeWX] Quantidade de series = qtdLinesChart0 (2).
            .amountSeries = qtdLinesChart0,
            // [EdgeWX] false: desenha na tela, nao so em background.
            .workInBackground = false,
            // [EdgeWX] true: mostra a linha do zero.
            .showZeroLine = true,
            // [EdgeWX] false: linhas sem negrito.
            .boldLine = false,
            // [EdgeWX] false: nao desenha pontos sobre a linha.
            .showDots = false
        };
    // [EdgeWX] Aplica a config no objeto grafico.
    grafico.setup(configLineChart0);
    // [EdgeWX] Registra o array de LineCharts no DisplayWX.
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);


    // [EdgeWX] Struct de configuracao do SpinBox 0.
    SpinBoxConfig configSpinBox0 = {
            // [EdgeWX] Callback disparado ao mudar o valor.
            .callback = spinbox_cb,
            // [EdgeWX] Valor minimo do spinbox: 0.
            .minValue = 0,
            // [EdgeWX] Valor maximo do spinbox: 100.
            .maxValue = 100,
            // [EdgeWX] Valor inicial: 0.
            .startValue = 0,
            // [EdgeWX] Largura do widget em pixels.
            .width = 202,
            // [EdgeWX] Altura do widget em pixels.
            .height = 57,
            // [EdgeWX] Passo de incremento/decremento: 10.
            .step = 10,
            // [EdgeWX] Cor de fundo/destaque do spinbox.
            .color = CFK_COLOR069,
            // [EdgeWX] Cor do texto do valor.
            .textColor = CFK_BLACK
        };
    // [EdgeWX] Aplica a config no objeto spinbox.
    spinbox.setup(configSpinBox0);
    // [EdgeWX] Registra o array de SpinBoxes no DisplayWX.
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);


    // [EdgeWX] Struct de configuracao do ToggleButton tgverde.
    ToggleButtonConfig configToggle0 = {
            // [EdgeWX] Callback ao alternar tgverde.
            .callback = tgverde_cb,
            // [EdgeWX] Largura do toggle verde.
            .width = 79,
            // [EdgeWX] Altura do toggle verde.
            .height = 40,
            // [EdgeWX] Cor quando pressionado/ligado.
            .pressedColor = CFK_COLOR037
        };
    // [EdgeWX] Aplica a config em tgverde.
    tgverde.setup(configToggle0);
    // [EdgeWX] Struct de configuracao do ToggleButton tglaranja.
    ToggleButtonConfig configToggle1 = {
            // [EdgeWX] Callback ao alternar tglaranja.
            .callback = tglaranja_cb,
            // [EdgeWX] Largura do toggle laranja.
            .width = 79,
            // [EdgeWX] Altura do toggle laranja.
            .height = 40,
            // [EdgeWX] Cor quando pressionado/ligado.
            .pressedColor = CFK_COLOR013
        };
    // [EdgeWX] Aplica a config em tglaranja.
    tglaranja.setup(configToggle1);
    // [EdgeWX] Registra o array de toggles no DisplayWX.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


}

// [EdgeWX] Cabecalho do callback do spinbox. Texto fixo do gerador.
// This function is a callback of this element spinbox.
// [EdgeWX] Aviso fixo do gerador: nao chamar o callback manualmente.
// You dont need call it. Make sure it is as short and quick as possible.
// [manual] Callback do SpinBox: grava a amplitude global usada no loop.
void spinbox_cb(){

    // [EdgeWX] Le o valor atual do spinbox.
    int myValue = spinbox.getValue();
    // [EdgeWX] Log padrao do gerador com o novo valor.
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
    // [manual] Amplitude do seno/cosseno no grafico.
    amplitudeSpinbox = myValue;
}
// [EdgeWX] Cabecalho do callback de tgverde. Texto fixo do gerador.
// This function is a callback of this element tgverde.
// [EdgeWX] Aviso fixo do gerador: nao chamar o callback manualmente.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Stub do toggle verde: copia getStatus() para switch0_val.
void tgverde_cb(){

    // [EdgeWX] Atualiza switch0_val com o estado de tgverde.
    switch0_val = tgverde.getStatus();
}
// [EdgeWX] Cabecalho do callback de tglaranja. Texto fixo do gerador.
// This function is a callback of this element tglaranja.
// [EdgeWX] Aviso fixo do gerador: nao chamar o callback manualmente.
// You dont need call it. Make sure it is as short and quick as possible.
// [EdgeWX] Stub do toggle laranja: copia getStatus() para switch1_val.
void tglaranja_cb(){

    // [EdgeWX] Atualiza switch1_val com o estado de tglaranja.
    switch1_val = tglaranja.getStatus();
}
