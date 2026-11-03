// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. [manual] = escrita depois, a logica dos tres tanques.
// [legenda] O user_setup.h desta pasta liga DISP_DEFAULT, TOUCH_GT911 e CAM_SC2336; displaywx.h o inclui.
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] Arduino_GFX pede SPI.h mesmo com painel MIPI DSI, que nao usa o barramento SPI.
#include <SPI.h>
// [EdgeWX] Arduino_GFX 1.5.9: classes DSI, primitivas (fillRoundRect) e fontes GFX.
#include <Arduino_GFX_Library.h>
// [EdgeWX] HAL DisplayWX: widgets, toque GT911, camera MIPI e a tarefa de UI.
#include <displaywx.h>
// [EdgeWX] ESP_Video abre o sensor MIPI. Presente em todo export com CameraMipi.
#include <ESP_Video.h>


// [EdgeWX] Ficha da placa escolhida no EdgeWX. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: EK79007 - DSI
    * Touch: GT911
    * Camera: SC2336
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e usado neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; aqui ninguem le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Area util do EK79007 em pixels, passada ao GT911.
const int DISPLAY_W = 1024;
// [EdgeWX] Altura do painel, par de DISPLAY_W.
const int DISPLAY_H = 600;
// [EdgeWX] Canto minimo X do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto maximo X do mapa. Igual a largura: toque 1:1 com o painel.
const int TOUCH_MAP_X1 = 1024;
// [EdgeWX] Canto minimo Y do mapa de toque.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto maximo Y do mapa de toque.
const int TOUCH_MAP_Y1 = 600;
// [EdgeWX] false: X e Y do GT911 nao trocam de eixo.
const bool TOUCH_SWAP_XY = false;
// [EdgeWX] true: espelha o eixo X. Ajuste de montagem deste 7 polegadas.
const bool TOUCH_INVERT_X = true;
// [EdgeWX] true: espelha o eixo Y, par do invert X desta placa.
const bool TOUCH_INVERT_Y = true;
// [EdgeWX] SDA do SCCB da SC2336. O mesmo GPIO 7 do GT911; o barramento e compartilhado.
const int CAM_SDA = 7;
// [EdgeWX] SCL do SCCB. O mesmo GPIO 8 do toque.
const int CAM_SCL = 8;
// [EdgeWX] Clock I2C do sensor, 400 kHz.
const int CAM_FREQUENCY_SCCB = 400000;
// [EdgeWX] Pino de interrupcao da camera. -1 = nao ligado. O gerador entrega este valor no argumento pwdn de startCamera.
const int CAM_INT = -1;
// [EdgeWX] Reset da camera. -1 = nao ligado. Vai para o argumento reset de startCamera.
const int CAM_RST = -1;
// [EdgeWX] Largura nativa pedida ao SC2336, 1080p.
const int CAM_WIDTH = 1920;
// [EdgeWX] Altura nativa do sensor. O widget mostra so um recorte escalado.
const int CAM_HEIGHT = 1080;
// [EdgeWX] Porta I2C 0 do SCCB.
const int CAM_I2C_PORT = 0;
// [EdgeWX] Espelha a imagem no eixo X. Opcao de camera do projeto, nao da logica dos tanques.
const bool CAM_MIRROR_X = true;
// [EdgeWX] Espelha a imagem no eixo Y.
const bool CAM_MIRROR_Y = true;
// [EdgeWX] GPIO de reset do painel EK79007.
const int DISP_RST = 27;
// [EdgeWX] SCL do GT911, compartilhado com a camera.
const int TOUCH_SCL = 8;
// [EdgeWX] SDA do GT911, compartilhado com a camera.
const int TOUCH_SDA = 7;
// [EdgeWX] Interrupcao do GT911. -1 = o driver nao usa o pino.
const int TOUCH_INT = -1;
// [EdgeWX] Reset do GT911. -1 = nao ligado neste desenho.
const int TOUCH_RST = -1;
// [EdgeWX] Rotacao 0..3 do painel e da camera. O comentario em ingles e do gerador.
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
// [EdgeWX] Flag de IPS/big-endian de paineis SPI. O construtor DSI abaixo nao le isIPS.
const bool isIPS = false; // Come display can use this as bigEndian flag

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Unica tela desenhada no editor. Indice 0.
void screen0();
// [EdgeWX] Aplica TextButtonConfig, ToggleButtonConfig, CircularBarConfig e CameraMipiConfig.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback do toggle "Encher" do reservatorio 1.
void toggle1_cb();
// [EdgeWX] Callback do toggle do reservatorio 2.
void toggle2_cb();
// [EdgeWX] Callback do toggle do reservatorio 3.
void toggle3_cb();
// [EdgeWX] Callback do TextButton "Esvaziar 1".
void esvaziar1_cb();
// [EdgeWX] Callback do TextButton "Esvaziar 2".
void esvaziar2_cb();
// [EdgeWX] Callback do TextButton "Esvaziar 3".
void esvaziar3_cb();

// [EdgeWX] Backlight do EK79007 neste hardware. Outros exports usam outro GPIO.
#define DISP_LED 26
// [EdgeWX] Ponteiro do barramento MIPI DSI. Preenchido no setup.
Arduino_ESP32DSIPanel *bus = nullptr;
// [EdgeWX] Ponteiro do display DSI. screen0 desenha o fundo estatico nele.
Arduino_DSI_Display *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa de UI.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] TextButton no canto superior esquerdo (810, 95), tela 0. Nome veio do editor.
TextButton esvaziar1(810, 95, 0);
// [EdgeWX] Segundo botao, 90 px abaixo do primeiro.
TextButton esvaziar2(810, 185, 0);
// [EdgeWX] Terceiro botao, coluna direita do painel.
TextButton esvaziar3(810, 275, 0);
// [EdgeWX] Tamanho do array exigido por setTextButton.
const uint8_t qtdTextButton = 3;
// [EdgeWX] Ponteiros dos botoes, na ordem do editor.
TextButton *arrayTextButton[qtdTextButton] = {&esvaziar1, &esvaziar2, &esvaziar3};
// [EdgeWX] Toggle "Encher" do tanque 1. Origem no canto superior esquerdo do widget.
ToggleButton toggle1(170, 135, 0);
// [EdgeWX] Toggle do tanque 2.
ToggleButton toggle2(170, 210, 0);
// [EdgeWX] Toggle do tanque 3.
ToggleButton toggle3(170, 275, 0);
// [EdgeWX] Quantidade passada a setToggle.
const uint8_t qtdToggleBtn = 3;
// [EdgeWX] Ponteiros dos toggles.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1, &toggle2, &toggle3};
// [EdgeWX] Espelho do estado do toggle1. O gerador cria uma bool por toggle; o callback so a atualiza.
bool  switch0_val = false; // Global variable that stores the value of the widget toggle1
// [EdgeWX] Espelho do toggle2. A maquina de estados nao consulta esta bool.
bool  switch1_val = false; // Global variable that stores the value of the widget toggle2
// [EdgeWX] Espelho do toggle3.
bool  switch2_val = false; // Global variable that stores the value of the widget toggle3
// [EdgeWX] Barra do reservatorio 1. Em CircularBar, x e y sao o centro do anel.
CircularBar reservatorio1(275, 460, 0);
// [EdgeWX] Centro do segundo anel.
CircularBar reservatorio2(510, 460, 0);
// [EdgeWX] Centro do terceiro anel.
CircularBar reservatorio3(765, 460, 0);
// [EdgeWX] Quantidade passada a setCircularBar.
const uint8_t qtdCircBar = 3;
// [EdgeWX] Ponteiros das barras.
CircularBar *arrayCircularbar[qtdCircBar] = {&reservatorio1, &reservatorio2, &reservatorio3};
// [EdgeWX] Canto superior esquerdo do preview, dentro do retangulo central da tela.
CameraMipi cameraMipi(295, 80, 0);
// [EdgeWX] Um unico widget de camera.
const uint8_t qtdCameraMipi = 1;
// [EdgeWX] Array pedido por setCameraMipi.
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

// [manual] Nivel 0..100 do tanque 1. A barra mostra este numero.
int totalTanque1 = 0;
// [manual] Nivel do tanque 2.
int totalTanque2 = 0;
// [manual] Nivel do tanque 3.
int totalTanque3 = 0;

// [manual] Periodo entre cada +1 enquanto enche. 100 passos x 500 ms = 50 s para encher.
uint32_t intervaloEncherTanque = 500;
// [manual] Periodo entre cada -1 enquanto esvazia. 100 passos x 600 ms = 60 s para esvaziar.
uint32_t intervaloEsvaziarTanque = 600;

// [manual] Marca millis() do ultimo passo do tanque 1. 0 faz o primeiro passo ocorrer na hora.
uint32_t timerTanque1 = 0;
// [manual] Ultimo passo do tanque 2.
uint32_t timerTanque2 = 0;
// [manual] Ultimo passo do tanque 3.
uint32_t timerTanque3 = 0;

// [manual] Estados da simulacao. So ENCHENDO e ESVAZIANDO alteram o nivel.
enum class EstadoTanque : uint8_t {
    // [manual] Nivel chegou a 0. O switch nao faz nada neste estado.
    VAZIO = 0,
    // [manual] Nivel chegou a 100. O switch nao faz nada neste estado.
    CHEIO = 1,
    // [manual] Toggle ligado: soma 1 a cada intervaloEncherTanque.
    ENCHENDO = 2,
    // [manual] Botao Esvaziar: subtrai 1 a cada intervaloEsvaziarTanque.
    ESVAZIANDO = 3,
    // [manual] Toggle desligado no meio do enchimento. O nivel congela.
    PARADO = 4
};

// [manual] Estado inicial: nada enche nem esvazia ate um toque.
EstadoTanque estadoTanque1 = EstadoTanque::PARADO;
// [manual] Estado inicial do tanque 2.
EstadoTanque estadoTanque2 = EstadoTanque::PARADO;
// [manual] Estado inicial do tanque 3.
EstadoTanque estadoTanque3 = EstadoTanque::PARADO;


// [manual] Um passo da simulacao. Os tres tanques chamam esta mesma funcao.
void loopTanque(EstadoTanque &estado, uint32_t &timer, int &total, uint32_t intervaloEncher, uint32_t intervaloEsvaziar, CircularBar &circularBar){
    // [manual] Desvia pelo estado corrente. Os callbacks so escrevem o enum; quem anima e o loop.
    switch(estado){
        // [manual] Tanque ja vazio: nenhuma escrita na barra.
        case EstadoTanque::VAZIO:
            break;
        // [manual] Tanque ja cheio: nenhuma escrita na barra.
        case EstadoTanque::CHEIO:
            break;
        // [manual] Bloco do enchimento, com chaves proprias.
        case EstadoTanque::ENCHENDO:
        {
            // [manual] Espera o periodo antes de somar. millis() evita delay bloqueante.
            if(millis() - timer >= intervaloEncher){
                // [manual] Sobe um ponto percentual.
                total++;
                // [manual] setValue limita a [min, max], guarda o valor anterior e pede redraw.
                circularBar.setValue(total);
                // [manual] Reabre a janela do proximo passo.
                timer = millis();
                // [manual] No teto, para de somar.
                if(total >= 100){
                    estado = EstadoTanque::CHEIO;
                }
            }
        }
            break;
        // [manual] Bloco do esvaziamento.
        case EstadoTanque::ESVAZIANDO:{
            // [manual] Mesma cadencia, com o intervalo mais lento.
            if(millis() - timer >= intervaloEsvaziar){
                // [manual] Desce um ponto.
                total--;
                // [manual] A barra acompanha o nivel. Abaixo de 0, setValue prende no minimo.
                circularBar.setValue(total);
                timer = millis();
                // [manual] No piso, encerra o esvaziamento.
                if(total <= 0){
                    estado = EstadoTanque::VAZIO;
                }
            }
        }
            break;
        // [manual] Pausa: o nivel e o desenho ficam como estao.
        case EstadoTanque::PARADO:
            break;
    }
}

// [manual] Amarra o tanque 1 nas globais e na barra reservatorio1.
void loopTanque1(){
    loopTanque(estadoTanque1, timerTanque1, totalTanque1, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio1);
}

// [manual] Tanque 2, mesmos intervalos dos outros.
void loopTanque2(){
    loopTanque(estadoTanque2, timerTanque2, totalTanque2, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio2);
}

// [manual] Tanque 3.
void loopTanque3(){
    loopTanque(estadoTanque3, timerTanque3, totalTanque3, intervaloEncherTanque, intervaloEsvaziarTanque, reservatorio3);
}

// [EdgeWX] Marcador que o exportador poe imediatamente antes do setup em todo exemplo com camera.
// linha de camera
// [EdgeWX] Arduino setup. A ordem abaixo e a do gerador: camera, DSI, toque, widgets, tarefa.
void setup(){

    // [EdgeWX] Log serial a 115200. Os callbacks de botao imprimem aqui.
    Serial.begin(115200);
    // [EdgeWX] Comentario do gerador abrindo a inicializacao da SC2336.
    // Setup camera
    // [EdgeWX] Rotacao e espelho antes de startCamera. Acima de 3 a biblioteca recusa.
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    // [EdgeWX] Abre o SCCB e o sensor. Ordem real: porta, SCL, SDA, freq, reset, pwdn, largura, altura.
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        // [EdgeWX] Cliente PPA (Pixel Processing Accelerator) do P4, operacao SRM, para escalar o quadro no framebuffer DSI.
        myDisplay.getCamera().startPpa();
    }else{
        // [EdgeWX] Falha de abertura. O painel segue sem preview.
        Serial.println("Camera not started");
    }
    // [EdgeWX] Timings do EK79007 que o gerador gravou para este painel.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] hsync_pulse_width, hsync_back_porch, hsync_front_porch.
    10, 160,160 ,
    // [EdgeWX] vsync_pulse_width, vsync_back_porch, vsync_front_porch.
    1, 23, 12,
    // [EdgeWX] prefer_speed do clock DPI, 52 MHz. O lane bitrate fica no default da Arduino_GFX.
    52000000);
    // [EdgeWX] Display DSI de 1024x600 preso a esse barramento.
    tft = new Arduino_DSI_Display(
    // [EdgeWX] Largura, altura, bus, rotacao 0 e auto_flush = true.
    1024 /* width default */, 600 /* height default */, bus, rotationScreen, true,
    // [EdgeWX] Reset no GPIO 27 e tabela ek79007_init_operations da Arduino_GFX, com o tamanho em comandos.
    DISP_RST, ek79007_init_operations, sizeof(ek79007_init_operations) / sizeof(lcd_init_cmd_t));
    // [EdgeWX] Sobe o PHY DSI, aplica a tabela de init e aloca o framebuffer.
    tft->begin();
    // [EdgeWX] O #if existe porque o gerador so define DISP_LED quando a placa tem backlight em GPIO.
    #if defined(DISP_LED)
    // [EdgeWX] Backlight como saida.
    pinMode(DISP_LED, OUTPUT);
    // [EdgeWX] Acende o backlight em nivel alto.
    digitalWrite(DISP_LED, HIGH);
    #endif
    // [EdgeWX] Entrega o Arduino_DSI_Display aos widgets e ao caminho da camera. O comentario em ingles e do gerador.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Comentario do gerador abrindo o bloco do GT911.
    // Setup touch
    // [EdgeWX] Grava o retangulo de calibracao 0,1024 x 0,600.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Aplica o espelhamento X/Y definido nas constantes.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Nao troca os eixos.
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o GT911 na I2C compartilhada. So compila com TOUCH_GT911 no user_setup.h.
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Liga o log de toque. Em varios exports a mesma linha sai comentada; neste projeto ela ficou ativa.
    myDisplay.enableTouchLog();
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra screen0 como tela carregada pela tarefa de UI.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false = watchdog desligado; 3 = timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

// [EdgeWX] Assinatura do loop do Arduino. O corpo original do gerador e so um delay; o miolo abaixo e da simulacao.
void loop(){

    // [manual] Toma o mutex de desenho por ate 100 ms. setValue nao corre junto com a tarefa de UI.
    myDisplay.startCustomDraw();
    // [manual] Um passo do tanque 1, se o estado e o timer permitirem.
    loopTanque1();
    // [manual] Um passo do tanque 2.
    loopTanque2();
    // [manual] Um passo do tanque 3.
    loopTanque3();
    // [manual] Devolve o mutex. A tarefa DisplayWX redesenha as barras marcadas.
    myDisplay.finishCustomDraw();
    // [manual] Folga de 50 ms. O toque, o preview e o redraw seguem na tarefa criada por createTask.
    delay(50);
}

// [EdgeWX] Desenho estatico da tela 0. A tarefa chama isto ao carregar a tela; widgets entram so no final.
void screen0(){

    // [EdgeWX] Fundo #003F7F (CFK_COLOR046 em widgetsetup.h).
    tft->fillScreen(CFK_COLOR046);
    // [EdgeWX] Cor de apagamento dos widgets igual ao fundo, para o redraw nao deixar retangulo estranho.
    WidgetBase::backgroundColor = CFK_COLOR046;
    // [EdgeWX] Miolo do painel da camera: x 280, y 65, 481x295, raio 20, mesma cor do fundo.
    tft->fillRoundRect(280, 65, 481, 295, 20, CFK_COLOR046);
    // [EdgeWX] Borda #55FFFF (CFK_COLOR044) desse painel.
    tft->drawRoundRect(280, 65, 481, 295, 20, CFK_COLOR044);
    // [EdgeWX] Titulo. TC_DATUM ancora o topo-centro do texto em (525, 14). Padding na cor do fundo.
    myDisplay.printText("Monitoramento de tanques", 525, 14, TC_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular20pt7b);
    // [EdgeWX] Cartao esquerdo dos toggles, 256x295.
    tft->fillRoundRect(10, 65, 256, 295, 20, CFK_COLOR046);
    // [EdgeWX] Borda do cartao esquerdo.
    tft->drawRoundRect(10, 65, 256, 295, 20, CFK_COLOR044);
    // [EdgeWX] Cartao direito dos botoes Esvaziar, 230x295.
    tft->fillRoundRect(776, 65, 230, 295, 20, CFK_COLOR046);
    // [EdgeWX] Borda do cartao direito.
    tft->drawRoundRect(776, 65, 230, 295, 20, CFK_COLOR044);
    // [EdgeWX] Faixa inferior das tres barras, 995x209.
    tft->fillRoundRect(10, 376, 995, 209, 20, CFK_COLOR046);
    // [EdgeWX] Borda da faixa inferior.
    tft->drawRoundRect(10, 376, 995, 209, 20, CFK_COLOR044);
    // [EdgeWX] Lembrete do gerador: a camera nao e desenhada aqui. drawWidgetsOnScreen cuida dela.
    //This screen has a/an cameraMipi
    // [EdgeWX] Lembrete da primeira CircularBar.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete da segunda CircularBar.
    //This screen has a/an circularBar
    // [EdgeWX] Lembrete da terceira CircularBar.
    //This screen has a/an circularBar
    // [EdgeWX] Legenda sob o primeiro anel. TL_DATUM = topo-esquerda em (205, 547).
    myDisplay.printText("Reservatorio 1", 205, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Legenda do segundo anel.
    myDisplay.printText("Reservatorio 2", 440, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Legenda do terceiro anel.
    myDisplay.printText("Reservatorio 3", 700, 547, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Lembrete do primeiro ToggleButton.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do segundo toggle.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do terceiro toggle.
    //This screen has a/an toggleButton
    // [EdgeWX] Rotulo ao lado do toggle 1, no cartao esquerdo.
    myDisplay.printText("Reservatorio 1", 25, 142, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo do toggle 2.
    myDisplay.printText("Reservatorio 2", 25, 217, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo do toggle 3.
    myDisplay.printText("Reservatorio 3", 25, 287, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Titulo da coluna de enchimento.
    myDisplay.printText("Encher", 105, 87, TL_DATUM, CFK_COLOR044, CFK_COLOR046, &RobotoRegular10pt7b);
    // [EdgeWX] Lembrete do primeiro TextButton.
    //This screen has a/an textButton
    // [EdgeWX] Lembrete do segundo TextButton.
    //This screen has a/an textButton
    // [EdgeWX] Lembrete do terceiro TextButton.
    //This screen has a/an textButton
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e 0.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Comentario fixo do gerador, inclusive o typo "widgtes".
// Configure each widgtes to be used
// [EdgeWX] Preenche as structs na ordem: botoes, toggles, barras, camera.
void loadWidgets(){

    // [EdgeWX] Propriedades do botao 1 copiadas do editor.
    TextButtonConfig configTextButton0 = {
            // [EdgeWX] Texto central. Fonte Roboto Bold 10 pt.
            .text = "Esvaziar 1",
            // [EdgeWX] A tarefa de UI chama esvaziar1_cb no toque. O sketch nao chama.
            .callback = esvaziar1_cb,
            .fontFamily = &RobotoBold10pt7b,
            // [EdgeWX] 151x58 px, canto com raio 10.
            .width = 151,
            .height = 58,
            .radius = 10,
            // [EdgeWX] Fundo #55AAFF (CFK_COLOR050) e texto preto.
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    // [EdgeWX] Aplica a struct. Sem setup o botao nao entra no redraw.
    esvaziar1.setup(configTextButton0);
    // [EdgeWX] Mesma geometria e as mesmas cores, com outro rotulo e outro callback.
    TextButtonConfig configTextButton1 = {
            .text = "Esvaziar 2",
            .callback = esvaziar2_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 151,
            .height = 58,
            .radius = 10,
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    esvaziar2.setup(configTextButton1);
    // [EdgeWX] Terceiro botao, mesmo molde.
    TextButtonConfig configTextButton2 = {
            .text = "Esvaziar 3",
            .callback = esvaziar3_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 151,
            .height = 58,
            .radius = 10,
            .backgroundColor = CFK_COLOR050,
            .textColor = CFK_BLACK
        };

    esvaziar3.setup(configTextButton2);
    // [EdgeWX] Entrega o array a DisplayWX. O numero tem de bater com qtdTextButton.
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


    // [EdgeWX] Toggle 1. pressedColor e a cor da capsula ligada, #55FFAA.
    ToggleButtonConfig configToggle0 = {
            .callback = toggle1_cb,
            // [EdgeWX] 69x35 px. A bolinha desliza dentro dessa caixa.
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle1.setup(configToggle0);
    // [EdgeWX] Toggle 2, mesmas medidas.
    ToggleButtonConfig configToggle1 = {
            .callback = toggle2_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle2.setup(configToggle1);
    // [EdgeWX] Toggle 3, mesmas medidas.
    ToggleButtonConfig configToggle2 = {
            .callback = toggle3_cb,
            .width = 69,
            .height = 35,
            .pressedColor = CFK_COLOR038
        };
    toggle3.setup(configToggle2);
    // [EdgeWX] Registra os tres toggles na tarefa de toque.
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    // [EdgeWX] Anel completo de 0 a 100. O nome configCirculaBar, sem o "r", e do gerador.
    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            // [EdgeWX] Raio 70 px em volta do centro (275, 460).
            .radius = 70,
            // [EdgeWX] 0 a 360 graus: circulo fechado, sentido padrao.
            .startAngle = 0,
            .endAngle = 360,
            // [EdgeWX] Arco preenchido em ciano #00FFFF; trilha em #556A7F.
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            // [EdgeWX] Valor no centro, texto branco sobre o fundo da tela.
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            // [EdgeWX] Espessura 30 px. showValue exige raio interno suficiente; 70-30 passa.
            .thickness = 30,
            .showValue = true,
            // [EdgeWX] false: o arco cresce no sentido normal da biblioteca.
            .inverted = false
        };
    // [EdgeWX] setup tambem zera o valor no minimo da faixa.
    reservatorio1.setup(configCirculaBar0);
    // [EdgeWX] Segundo anel, copia do primeiro.
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            .thickness = 30,
            .showValue = true,
            .inverted = false
        };
    reservatorio2.setup(configCirculaBar1);
    // [EdgeWX] Terceiro anel, copia do primeiro.
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 70,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR043,
            .backgroundColor = CFK_COLOR048,
            .textColor = CFK_WHITE,
            .backgroundText = CFK_COLOR046,
            .thickness = 30,
            .showValue = true,
            .inverted = false
        };
    reservatorio3.setup(configCirculaBar2);
    // [EdgeWX] Registra as barras. Quem muda o valor depois e loopTanque, via setValue.
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);


    // [EdgeWX] Retangulo de preview. 452x264 cabe na moldura 481x295 com margem.
    CameraMipiConfig configCameraMipi0 = {
            .width = 452,
            .height = 264,
            // [EdgeWX] Sem rotulo de zoom e sem contador de fps. A tarefa redesenha a 5 fps.
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    // [EdgeWX] Registra o preview. createTask so sobe a tarefa da camera se startCamera teve sucesso.
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


    // [manual] Reforco do nivel 0 depois do setup. A biblioteca ja inicia no minValue.
    reservatorio1.setValue(0);
    // [manual] Mesmo reforco na segunda barra.
    reservatorio2.setValue(0);
    // [manual] Mesmo reforco na terceira barra.
    reservatorio3.setValue(0);


}

// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI, fora do mutex de desenho, e deve ser curto.
// This function is a callback of this element esvaziar1.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar1_cb(){
    // [manual] Desliga o toggle sem disparar toggle1_cb. O segundo false e triggerCallback.
    toggle1.setStatus(false, false);
    // [manual] A simulacao passa a descer o nivel no loop.
    estadoTanque1 = EstadoTanque::ESVAZIANDO;
    // [EdgeWX] Linha de diagnostico que o gerador coloca no corpo de TextButton.
    Serial.print("Clicked on: ");Serial.println("esvaziar1_cb");
}
// [EdgeWX] Mesmo aviso do gerador, agora para o botao 2.
// This function is a callback of this element esvaziar2.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar2_cb(){
    // [manual] Apaga o toggle 2 sem reentrar no callback de encher.
    toggle2.setStatus(false, false);
    estadoTanque2 = EstadoTanque::ESVAZIANDO;
    // [EdgeWX] Log original do export.
    Serial.print("Clicked on: ");Serial.println("esvaziar2_cb");
}
// [EdgeWX] Mesmo aviso do gerador, para o botao 3.
// This function is a callback of this element esvaziar3.
// You dont need call it. Make sure it is as short and quick as possible.
void esvaziar3_cb(){
    // [manual] Apaga o toggle 3 e manda esvaziar.
    toggle3.setStatus(false, false);
    estadoTanque3 = EstadoTanque::ESVAZIANDO;
    // [EdgeWX] Log original do export.
    Serial.print("Clicked on: ");Serial.println("esvaziar3_cb");
}
// [EdgeWX] Callback do toggle 1. O gerador avisa para nao chamar a funcao no sketch.
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){
    
    // [EdgeWX] Copia o estado visual para a bool que o exportador criou.
    switch0_val = toggle1.getStatus();
    // [manual] Ligado enche; desligado pausa. Nao esvazia: esvaziar e so o TextButton.
    estadoTanque1 = switch0_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
// [EdgeWX] Callback do toggle 2.
// This function is a callback of this element toggle2.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle2_cb(){

    // [EdgeWX] Espelho do toggle 2.
    switch1_val = toggle2.getStatus();
    // [manual] Mesma regra do tanque 1.
    estadoTanque2 = switch1_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
// [EdgeWX] Callback do toggle 3.
// This function is a callback of this element toggle3.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle3_cb(){

    // [EdgeWX] Espelho do toggle 3.
    switch2_val = toggle3.getStatus();
    // [manual] Mesma regra do tanque 1.
    estadoTanque3 = switch2_val ? EstadoTanque::ENCHENDO : EstadoTanque::PARADO;
}
