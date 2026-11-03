// [legenda] [EdgeWX] = linha emitida pelo exportador do EdgeWX. [manual] = escrita depois, a logica da esteira.
// [legenda] O user_setup.h desta pasta liga DISP_DEFAULT, TOUCH_GSL3680 e CAM_OV02C10; displaywx.h o inclui.
// [EdgeWX] PNG da webcam embutido pelo editor: pixels RGB565, mascara e iwebcampngW/H.
#include "iwebcampng.h"
// [EdgeWX] Cabecalho fixo do exportador. Este sketch nao carrega plugin de outro chip.
// Include for plugins of chip 0
// [EdgeWX] Marca o bloco de includes que o gerador sempre coloca no topo.
// Include external libraries and files
// [EdgeWX] Arduino_GFX pede SPI.h mesmo com painel MIPI DSI, que nao usa o barramento SPI.
#include <SPI.h>
// [EdgeWX] Arduino_GFX 1.5.9: classes DSI, primitivas e fontes GFX.
#include <Arduino_GFX_Library.h>
// [EdgeWX] HAL DisplayWX: widgets, toque GSL3680, camera MIPI e a tarefa de UI.
#include <displaywx.h>
// [EdgeWX] ESP_Video abre o sensor MIPI. Presente em todo export com CameraMipi.
#include <ESP_Video.h>


// [EdgeWX] Ficha da placa escolhida no EdgeWX. Nao e codigo executavel.
    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: JD9365 - DSI
    * Touch: GSL3680
    * Camera: OV02C10
    */
// [EdgeWX] Titulo do bloco de constantes. SPIFFS nao e usado neste sketch.
// Defines for font and files
// [EdgeWX] Flag de formatacao do SPIFFS. O gerador emite; aqui ninguem le.
#define FORMAT_SPIFFS_IF_FAILED false
// [EdgeWX] Largura logica depois da rotacao 1. O painel nativo e 800x1280.
const int DISPLAY_W = 1280;
// [EdgeWX] Altura logica, par de DISPLAY_W. Vai para o GSL3680 junto com a rotacao.
const int DISPLAY_H = 800;
// [EdgeWX] Canto minimo X do mapa de toque, no eixo nativo do sensor.
const int TOUCH_MAP_X0 = 0;
// [EdgeWX] Canto maximo X nativo. 800, nao 1280: o mapa fica na orientacao fisica do painel.
const int TOUCH_MAP_X1 = 800;
// [EdgeWX] Canto minimo Y do mapa de toque.
const int TOUCH_MAP_Y0 = 0;
// [EdgeWX] Canto maximo Y nativo, 1280.
const int TOUCH_MAP_Y1 = 1280;
// [EdgeWX] false: o driver nao troca X com Y alem da rotacao ja aplicada.
const bool TOUCH_SWAP_XY = false;
// [EdgeWX] false: eixo X do GSL3680 sem espelho extra.
const bool TOUCH_INVERT_X = false;
// [EdgeWX] false: eixo Y sem espelho extra.
const bool TOUCH_INVERT_Y = false;
// [EdgeWX] SDA do SCCB da OV02C10. O mesmo GPIO 7 do toque.
const int CAM_SDA = 7;
// [EdgeWX] SCL do SCCB. O mesmo GPIO 8 do toque.
const int CAM_SCL = 8;
// [EdgeWX] Clock I2C do sensor, 400 kHz.
const int CAM_FREQUENCY_SCCB = 400000;
// [EdgeWX] Interrupcao da camera. -1 = nao ligada. O gerador entrega este valor no argumento pwdn de startCamera.
const int CAM_INT = -1;
// [EdgeWX] Reset da camera. -1 = nao ligado. Vai para o argumento reset de startCamera.
const int CAM_RST = -1;
// [EdgeWX] Largura pedida ao sensor, modo 1080p da OV02C10.
const int CAM_WIDTH = 1920;
// [EdgeWX] Altura nativa. O widget mostra so o recorte escalado.
const int CAM_HEIGHT = 1080;
// [EdgeWX] Porta I2C 0 do SCCB.
const int CAM_I2C_PORT = 0;
// [EdgeWX] Sem espelho no eixo X. Opcao de camera do projeto.
const bool CAM_MIRROR_X = false;
// [EdgeWX] Sem espelho no eixo Y.
const bool CAM_MIRROR_Y = false;
// [EdgeWX] GPIO de reset do painel JD9365.
const int DISP_RST = 27;
// [EdgeWX] SCL do GSL3680, compartilhado com a camera.
const int TOUCH_SCL = 8;
// [EdgeWX] SDA do GSL3680, compartilhado com a camera.
const int TOUCH_SDA = 7;
// [EdgeWX] Interrupcao do GSL3680 no GPIO 21.
const int TOUCH_INT = 21;
// [EdgeWX] Reset do GSL3680 no GPIO 22.
const int TOUCH_RST = 22;
// [EdgeWX] Rotacao 1 gira o painel 800x1280 para a paisagem 1280x800. O comentario em ingles e do gerador.
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] Flag de IPS/big-endian de paineis SPI. O construtor DSI abaixo nao le isIPS.
const bool isIPS = true; // Come display can use this as bigEndian flag

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Unica tela desenhada no editor. Indice 0.
void screen0();
// [EdgeWX] Aplica a configuracao de cada widget.
void loadWidgets();

// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
// [EdgeWX] Callback da chave (toggle) que libera o botao INICIAR.
void toggle1_cb();
// [EdgeWX] Callback do slider de velocidade.
void slidervelocidad_cb();
// [EdgeWX] Callback do icone da webcam.
void iwebcampng_cb();
// [EdgeWX] Callback do botao INICIAR.
void btnligar_cb();
// [EdgeWX] Callback do botao PARAR.
void btnparar_cb();

// [EdgeWX] Backlight desta Waveshare de 10 polegadas. Outros paineis usam outro GPIO.
#define DISP_LED 23
// [EdgeWX] Ponteiro do barramento MIPI DSI. Preenchido no setup.
Arduino_ESP32DSIPanel *bus = nullptr;
// [EdgeWX] Ponteiro do display DSI. screen0 desenha o fundo estatico nele.
Arduino_DSI_Display *tft = nullptr;
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa de UI.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] Gauge "Velocidade" na faixa de baixo. Nome e posicao vieram do editor.
GaugeSuper gauge(285, 640, 0);
// [EdgeWX] Quantidade passada a setGauge.
const uint8_t qtdGauge = 1;
// [EdgeWX] Ponteiro do unico gauge.
GaugeSuper *arrayGauge[qtdGauge] = {&gauge};
// [EdgeWX] Tres marcas de cor na escala do gauge.
const uint8_t qtdIntervalG0 = 3;
// [EdgeWX] Limites 0, 60 e 80. Cada um abre uma faixa ate o proximo, dentro de 0..100.
int range0[qtdIntervalG0] = {0,60,80};
// [EdgeWX] Branco, amarelo #FFFF00 e laranja #FF7F00, na mesma ordem das marcas.
uint16_t colors0[qtdIntervalG0] = {CFK_WHITE,CFK_COLOR019,CFK_COLOR013};
// [EdgeWX] Slider horizontal. A origem e o canto superior esquerdo da trilha.
HSlider slidervelocidad(125, 450, 0);
// [EdgeWX] Quantidade passada a setHSlider.
const uint8_t qtdHSlider = 1;
// [EdgeWX] Ponteiro do slider.
HSlider *arrayHslider[qtdHSlider] = {&slidervelocidad};
// [EdgeWX] Espelho do valor do slider. O gerador cria um int por HSlider; o callback o atualiza.
int hslider0_val = 0; // Global variable that stores the value of the widget slidervelocidad
// [EdgeWX] Label do total de pecas. Com ML_DATUM, (x, y) e o meio da borda esquerda do texto.
Label txttotal(1010, 125, 0);
// [EdgeWX] Label de pecas danificadas.
Label txtdanificado(1010, 230, 0);
// [EdgeWX] Label do tempo de funcionamento.
Label txttempo(1005, 320, 0);
// [EdgeWX] Quantidade passada a setLabel. O gerador chama este widget de "texto" nos comentarios da tela.
const uint8_t qtdLabel = 3;
// [EdgeWX] Ponteiros na ordem do editor.
Label *arrayLabel[qtdLabel] = {&txttotal, &txtdanificado, &txttempo};
// [EdgeWX] LED vermelho de parado. O raio vem no LedConfig.
Led ledred(690, 120, 0);
// [EdgeWX] LED verde de esteira ligada, ao lado do vermelho.
Led ledgreen(745, 120, 0);
// [EdgeWX] Quantidade passada a setLed.
const uint8_t qtdLed = 2;
// [EdgeWX] Ponteiros dos LEDs.
Led *arrayLed[qtdLed] = {&ledred, &ledgreen};
// [EdgeWX] Botao INICIAR, canto superior esquerdo em (510, 160), tela 0.
TextButton btnligar(510, 160, 0);
// [EdgeWX] Botao PARAR, na faixa inferior.
TextButton btnparar(495, 680, 0);
// [EdgeWX] Quantidade passada a setTextButton.
const uint8_t qtdTextButton = 2;
// [EdgeWX] Ponteiros dos botoes.
TextButton *arrayTextButton[qtdTextButton] = {&btnligar, &btnparar};
// [EdgeWX] Chave que arma o INICIAR. Sozinha nao liga a esteira.
ToggleButton toggle1(695, 310, 0);
// [EdgeWX] Quantidade passada a setToggle.
const uint8_t qtdToggleBtn = 1;
// [EdgeWX] Ponteiro do toggle.
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1};
// [EdgeWX] Espelho do toggle. btnligar_cb so parte a esteira se isto estiver true.
bool  switch0_val = false; // Global variable that stores the value of the widget toggle1
// [EdgeWX] Primeira barra vertical. O gerador nomeia as copias com sufixo "copy".
VBar vertbar1(760, 445, 0);
// [EdgeWX] Segunda barra, 100 px a direita.
VBar vertbar1copy(860, 445, 0);
// [EdgeWX] Terceira barra.
VBar vertbar1copy2(965, 445, 0);
// [EdgeWX] Quarta barra.
VBar vertbar1copy3(1065, 445, 0);
// [EdgeWX] Quantidade passada a setVBar. Na tela o gerador chama a barra de "rectRange".
const uint8_t qtdVBar = 4;
// [EdgeWX] Ponteiros das quatro barras.
VBar *arrayVbar[qtdVBar] = {&vertbar1, &vertbar1copy, &vertbar1copy2, &vertbar1copy3};
// [EdgeWX] Icone 32x32 da webcam, no canto do cartao da camera.
Image iwebcampng(360, 95, 0);
// [EdgeWX] Quantidade passada a setImage.
const uint8_t qtdImagem = 1;
// [EdgeWX] Ponteiro da imagem.
Image *arrayImagem[qtdImagem] = {&iwebcampng};
// [EdgeWX] Canto superior esquerdo do preview, dentro do cartao esquerdo.
CameraMipi cameraMipi(44, 143, 0);
// [EdgeWX] Um unico widget de camera.
const uint8_t qtdCameraMipi = 1;
// [EdgeWX] Array pedido por setCameraMipi.
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

// [manual] Marca millis() do ultimo incremento. 0 faz o primeiro passo sair assim que a esteira liga e ja se passou 1 s.
uint32_t tempoTotal = 0;
// [manual] Periodo dos contadores, 1 s.
uint32_t intervalo = 1000;
// [manual] Pecas contadas desde o ultimo INICIAR.
uint32_t totalPecas = 0;
// [manual] Segundos com a esteira ligada. Vira MM:SS no label.
uint32_t tempoFuncionamento = 0;
// [manual] Pecas marcadas como danificadas pelo sorteio.
uint32_t totalPecasDanificadas = 0;
// [manual] false ate INICIAR com a chave ligada. PARAR volta para false.
bool esteiraLigada = false;


// [manual] true acende o verde e apaga o vermelho. false faz o contrario: parado.
void setStatusLed(bool status){
    if(status){
        // [manual] Vermelho apagado enquanto a esteira corre.
        ledred.setState(false);
        // [manual] Verde ligado. setState pede o redraw do circulo.
        ledgreen.setState(true);
    }else{
        // [manual] Vermelho ligado: esteira parada.
        ledred.setState(true);
        ledgreen.setState(false);
    }
}

// [EdgeWX] Marcador que o exportador poe imediatamente antes do setup em todo exemplo com camera.
// linha de camera
// [EdgeWX] Arduino setup. A ordem e a do gerador: camera, DSI, toque, widgets, tarefa.
void setup(){

    // [EdgeWX] Log serial a 115200. Os callbacks de botao imprimem aqui.
    Serial.begin(115200);
    // [EdgeWX] Comentario do gerador abrindo a inicializacao da OV02C10.
    // Setup camera
    // [EdgeWX] Rotacao 1 e sem espelho, antes de startCamera. Acima de 3 a biblioteca recusa.
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    // [EdgeWX] Abre o SCCB e o sensor. Ordem real: porta, SCL, SDA, freq, reset, pwdn, largura, altura.
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        // [EdgeWX] Cliente PPA do P4, operacao SRM, para escalar o quadro no framebuffer DSI.
        myDisplay.getCamera().startPpa();
    }else{
        // [EdgeWX] Falha de abertura. O painel segue sem preview.
        Serial.println("Camera not started");
    }
    // [EdgeWX] Timings do JD9365. Os comentarios entre /* */ ja vieram no export.
    bus = new Arduino_ESP32DSIPanel(
    // [EdgeWX] Pulso, back porch e front porch do HSYNC: 20, 20 e 40.
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    // [EdgeWX] Pulso, back porch e front porch do VSYNC: 4, 8 e 20.
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    // [EdgeWX] Clock DPI preferido, 60 MHz.
    60000000 /* prefer_speed */);
    // [EdgeWX] Display DSI. A medida default do construtor e a nativa, 800x1280; a rotacao 1 apresenta 1280x800.
    tft = new Arduino_DSI_Display(
    // [EdgeWX] Largura nativa, altura nativa, bus, rotacao e auto_flush = true.
    800 /* width default */, 1280 /* height default */, bus, rotationScreen, true,
    // [EdgeWX] Reset no GPIO 27 e tabela jd9365_init_operations da Arduino_GFX.
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
    // [EdgeWX] Sobe o PHY DSI, aplica a tabela de init e aloca o framebuffer.
    tft->begin();
    // [EdgeWX] O #if existe porque o gerador so define DISP_LED quando a placa tem backlight em GPIO.
    #if defined(DISP_LED)
    // [EdgeWX] Backlight como saida.
    pinMode(DISP_LED, OUTPUT);
    // [EdgeWX] Acende o backlight em nivel alto.
    digitalWrite(DISP_LED, HIGH);
    #endif
    // [EdgeWX] Entrega o Arduino_DSI_Display aos widgets e ao caminho da camera.
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // [EdgeWX] Comentario do gerador abrindo o bloco do GSL3680.
    // Setup touch
    // [EdgeWX] Grava o retangulo nativo 0,800 x 0,1280.
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    // [EdgeWX] Sem espelhamento extra.
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    // [EdgeWX] Nao troca os eixos.
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    // [EdgeWX] Sobe o GSL3680. So compila com TOUCH_GSL3680 no user_setup.h. w/h sao os 1280x800 ja rotacionados.
    myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Log de toque sai comentado neste export. Tirar as barras liga enableTouchLog().
    //myDisplay.enableTouchLog();
    // [EdgeWX] Configura cada widget. O comentario em ingles, com o typo, e texto fixo do gerador.
    loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra screen0 como tela carregada pela tarefa de UI.
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false = watchdog desligado; 3 = timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    // [manual] Garante vermelho aceso e verde apagado depois que os LEDs ja foram registrados.
    setStatusLed(false);
}



// [manual] A cada 1 s, com a esteira ligada, soma peca, tempo e um sorteio de defeito.
void updateContadores(){
    // [manual] Parada: os labels ficam no ultimo valor.
    if(!esteiraLigada){
        return;
    }

    // [manual] Janela de 1 s sem delay bloqueante.
    if(millis() - tempoTotal >= intervalo){
        // [manual] Reabre a janela.
        tempoTotal = millis();
        // [manual] Um segundo a mais no relogio.
        tempoFuncionamento++;
        // [manual] Uma peca a mais por segundo. Nao depende da posicao do slider.
        totalPecas++;

        // [manual] Sorteia 0..99. O resto da divisao por 2 vale 0 ou 1.
        int sorteio = random(0, 100);
        // [manual] Impar soma uma peca danificada; par nao soma.
        totalPecasDanificadas += sorteio % 2;

        // [manual] Label aceita int e marca redraw. O prefixo e o sufixo continuam vazios.
        txttotal.setTextInt(totalPecas);
        txtdanificado.setTextInt(totalPecasDanificadas);


        // [manual] "MM:SS" mais o terminador cabem em 6 bytes. Os minutos reiniciam em 100.
        char tempoBuf[6];
        snprintf(tempoBuf, sizeof(tempoBuf), "%02u:%02u",
                 // [manual] Minutos, limitados a dois digitos.
                 (unsigned)((tempoFuncionamento / 60u) % 100u),
                 // [manual] Segundos dentro do minuto.
                 (unsigned)(tempoFuncionamento % 60u));
        // [manual] setText pede const char*. O buffer local vale ate o retorno da funcao.
        const char *tempoFmt = tempoBuf;
        txttempo.setText(tempoFmt);
    }
}

// [EdgeWX] Assinatura do loop. O gerador ja coloca startCustomDraw e os setValue de demonstracao.
void loop(){

    // [EdgeWX] Toma o mutex de desenho por ate 100 ms. setValue nao corre junto com a tarefa de UI.
    myDisplay.startCustomDraw();

    // [manual] Atualiza pecas, defeitos e relogio se a esteira estiver ligada.
    updateContadores();

    // [EdgeWX] Placeholder do gerador: valor aleatorio 0..99. O comentario em ingles e texto fixo do export.
    vertbar1.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Mesmo placeholder na segunda barra.
    vertbar1copy.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Mesmo placeholder na terceira barra.
    vertbar1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Mesmo placeholder na quarta barra.
    vertbar1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    // [EdgeWX] Devolve o mutex. A tarefa DisplayWX redesenha o que foi marcado.
    myDisplay.finishCustomDraw();

    // [EdgeWX] Folga de 100 ms, a mesma dos outros exports com camera. Toque e preview seguem na tarefa de UI.
    delay(100);
}

// [EdgeWX] Desenho estatico da tela 0. Os widgets entram so em drawWidgetsOnScreen.
void screen0(){

    // [EdgeWX] Fundo CFK_GREY4, RGB565 0x4228.
    tft->fillScreen(CFK_GREY4);
    // [EdgeWX] Cor de apagamento dos widgets igual ao fundo da pagina.
    WidgetBase::backgroundColor = CFK_GREY4;
    // [EdgeWX] Cartao da camera: x 30, y 85, 380x292, raio 10, preenchimento CFK_GREY3.
    tft->fillRoundRect(30, 85, 380, 292, 10, CFK_GREY3);
    // [EdgeWX] Borda preta desse cartao.
    tft->drawRoundRect(30, 85, 380, 292, 10, CFK_BLACK);
    // [EdgeWX] Cartao do meio: status, chave e INICIAR.
    tft->fillRoundRect(430, 85, 380, 292, 10, CFK_GREY3);
    tft->drawRoundRect(430, 85, 380, 292, 10, CFK_BLACK);
    // [EdgeWX] Cartao direito dos contadores, 425x292.
    tft->fillRoundRect(830, 80, 425, 292, 10, CFK_GREY3);
    tft->drawRoundRect(830, 80, 425, 292, 10, CFK_BLACK);
    // [EdgeWX] Faixa do slider, das barras e do titulo Velocidade.
    tft->fillRoundRect(25, 395, 1223, 262, 10, CFK_GREY3);
    tft->drawRoundRect(25, 395, 1223, 262, 10, CFK_BLACK);
    // [EdgeWX] Faixa inferior do gauge e do botao PARAR.
    tft->fillRoundRect(25, 670, 1223, 122, 10, CFK_GREY3);
    tft->drawRoundRect(25, 670, 1223, 122, 10, CFK_BLACK);
    // [EdgeWX] Titulo. TL_DATUM ancora o topo-esquerda em (475, 24). Cor laranja #FF7F00, padding no fundo da pagina.
    myDisplay.printText("Controle de Esteira", 475, 24, TL_DATUM, CFK_COLOR013, CFK_GREY4, &RobotoRegular20pt7b);
    // [EdgeWX] Titulo do cartao da camera. O padding CFK_GREY3 cobre o retangulo do texto.
    myDisplay.printText("Visualizar", 170, 97, TL_DATUM, CFK_COLOR013, CFK_GREY3, &RobotoRegular12pt7b);
    // [EdgeWX] Lembrete do gerador: o icone nao e desenhado aqui.
    //This screen has a/an imagem
    // [EdgeWX] Lembrete do preview MIPI.
    //This screen has a/an cameraMipi
    // [EdgeWX] Lembrete do botao INICIAR.
    //This screen has a/an textButton
    // [EdgeWX] Lembrete da chave.
    //This screen has a/an toggleButton
    // [EdgeWX] Lembrete do LED vermelho.
    //This screen has a/an led
    // [EdgeWX] Lembrete do LED verde.
    //This screen has a/an led
    // [EdgeWX] Rotulo acima dos LEDs.
    myDisplay.printText("Status", 590, 107, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo da chave.
    myDisplay.printText("Chave", 625, 327, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo do contador de pecas.
    myDisplay.printText("Total", 940, 117, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo das pecas danificadas.
    myDisplay.printText("Danificados", 885, 217, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Rotulo do tempo.
    myDisplay.printText("Tempo", 925, 307, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Lembrete do label Total. O gerador chama Label de "texto".
    //This screen has a/an texto
    // [EdgeWX] Lembrete do label Danificados.
    //This screen has a/an texto
    // [EdgeWX] Lembrete do label Tempo.
    //This screen has a/an texto
    // [EdgeWX] Lembrete do HSlider.
    //This screen has a/an slider
    // [EdgeWX] Lembrete do GaugeSuper. No export o nome interno e "ponteiroAnalog".
    //This screen has a/an ponteiroAnalog
    // [EdgeWX] Titulo da faixa de velocidade.
    myDisplay.printText("Velocidade", 245, 412, TL_DATUM, CFK_WHITE, CFK_GREY3, &RobotoRegular10pt7b);
    // [EdgeWX] Lembrete da primeira VBar. No export o nome interno e "rectRange".
    //This screen has a/an rectRange
    // [EdgeWX] Segunda barra.
    //This screen has a/an rectRange
    // [EdgeWX] Terceira barra.
    //This screen has a/an rectRange
    // [EdgeWX] Quarta barra.
    //This screen has a/an rectRange
    // [EdgeWX] Lembrete do botao PARAR.
    //This screen has a/an textButton
    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e 0.
    myDisplay.drawWidgetsOnScreen(0);
}

// [EdgeWX] Comentario fixo do gerador, inclusive o typo "widgtes".
// Configure each widgtes to be used
// [EdgeWX] Preenche as structs na ordem em que o editor exportou os widgets.
void loadWidgets(){

    // [EdgeWX] Propriedades do gauge copiadas do editor.
    GaugeConfig configGauge0 = {
            // [EdgeWX] Titulo desenhado no proprio widget, alem do printText da faixa.
            .title = "Velocidade",
            // [EdgeWX] Ponteiros para as marcas 0, 60 e 80 e para as tres cores.
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoRegular8pt7b,
            // [EdgeWX] Escala 0..100. A agulha comeca no minimo ate alguem chamar setValue.
            .minValue = 0,
            .maxValue = 100,
            // [EdgeWX] Retangulo 400x130 do gauge.
            .width = 400,
            .height = 130,
            // [EdgeWX] Borda vermelha #FF0000, texto e titulo pretos, fundo branco.
            .borderColor = CFK_COLOR007,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .titleColor = CFK_BLACK,
            // [EdgeWX] Agulha vermelha e marcas pretas.
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            // [EdgeWX] Mostra os numeros das marcas. Exige a fonte acima.
            .showLabels = true
        };
    gauge.setup(configGauge0);
    // [EdgeWX] Entrega o array. O numero tem de bater com qtdGauge.
    myDisplay.setGauge(arrayGauge,qtdGauge);


    // [EdgeWX] Trilha de 0 a 100. O callback dispara quando o valor muda.
    HSliderConfig configHSlider0 = {
            .callback = slidervelocidad_cb,
            // [EdgeWX] Sem label acoplado. O titulo "Velocidade" e um printText da tela.
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            // [EdgeWX] Knob com raio 22 numa trilha de 323 px. Cor de pressao vermelha, fundo branco.
            .radius = 22,
            .width = 323,
            .pressedColor = CFK_COLOR007,
            .backgroundColor = CFK_WHITE,
        };
    slidervelocidad.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);


    // [EdgeWX] Placeholder "9999" so para medir a area. updateContadores troca o texto.
    LabelConfig configLabel0 = {
            .text = "9999",
            // [EdgeWX] Sem prefixo nem sufixo.
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular25pt7b,
            // [EdgeWX] ML_DATUM = meio da borda esquerda, alinhado ao (x, y) do construtor.
            .datum = ML_DATUM,
            // [EdgeWX] Digitos amarelos sobre o cinza do cartao.
            .fontColor = CFK_COLOR019,
            .backgroundColor = CFK_GREY3,
        };
    txttotal.setup(configLabel0);
    // [EdgeWX] Mesmo molde, fonte 20 pt e cor laranja.
    LabelConfig configLabel1 = {
            .text = "9999",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular20pt7b,
            .datum = ML_DATUM,
            .fontColor = CFK_COLOR013,
            .backgroundColor = CFK_GREY3,
        };
    txtdanificado.setup(configLabel1);
    // [EdgeWX] Relogio em fonte de 7 segmentos, texto inicial 00:00, ciano #00FFFF.
    LabelConfig configLabel2 = {
            .text = "00:00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &G7_Segment7_S520pt7b,
            .datum = ML_DATUM,
            .fontColor = CFK_COLOR043,
            .backgroundColor = CFK_GREY3,
        };
    txttempo.setup(configLabel2);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    // [EdgeWX] LED vermelho nasce aceso. Raio 16; apagado usa o cinza do cartao.
    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR007,
            .colorOff = CFK_GREY3,
            .initialState = true
        };
    ledred.setup(configLed0);
    // [EdgeWX] LED verde nasce apagado. colorOn e #00FF00.
    LedConfig configLed1 = {
            .radius = 16,
            .colorOn = CFK_COLOR031,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    ledgreen.setup(configLed1);
    myDisplay.setLed(arrayLed,qtdLed);


    // [EdgeWX] Botao INICIAR, 217x106, raio 10, fundo verde e texto preto.
    TextButtonConfig configTextButton0 = {
            .text = "INICIAR",
            .callback = btnligar_cb,
            .fontFamily = &RobotoBold20pt7b,
            .width = 217,
            .height = 106,
            .radius = 10,
            .backgroundColor = CFK_COLOR031,
            .textColor = CFK_BLACK
        };

    btnligar.setup(configTextButton0);
    // [EdgeWX] Botao PARAR, 280x104, fundo vermelho e texto branco.
    TextButtonConfig configTextButton1 = {
            .text = "PARAR",
            .callback = btnparar_cb,
            .fontFamily = &RobotoBold25pt7b,
            .width = 280,
            .height = 104,
            .radius = 10,
            .backgroundColor = CFK_COLOR007,
            .textColor = CFK_WHITE
        };

    btnparar.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


    // [EdgeWX] Chave 100x50. Ligada, a capsula fica verde.
    ToggleButtonConfig configToggle0 = {
            .callback = toggle1_cb,
            .width = 100,
            .height = 50,
            .pressedColor = CFK_COLOR031
        };
    toggle1.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    // [EdgeWX] Barra 60x182, cantos retos, preenchimento vermelho, sem label acoplado.
    VerticalBarConfig configVBar0 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            // [EdgeWX] Orientation::VERTICAL sobe o preenchimento com o valor.
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1.setup(configVBar0);
    // [EdgeWX] Segunda barra, copia da primeira.
    VerticalBarConfig configVBar1 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy.setup(configVBar1);
    // [EdgeWX] Terceira barra, mesma copia.
    VerticalBarConfig configVBar2 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy2.setup(configVBar2);
    // [EdgeWX] Quarta barra, mesma copia.
    VerticalBarConfig configVBar3 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 60,
            .height = 182,
            .filledColor = CFK_COLOR007
        };
    vertbar1copy3.setup(configVBar3);
    // [EdgeWX] Registra as barras. O loop troca o valor com setValue aleatorio.
    myDisplay.setVBar(arrayVbar,qtdVBar);


    // [EdgeWX] Icone embutido. pixels e mask saem de iwebcampng.h; o toque chama iwebcampng_cb.
    ImageFromPixelsConfig configImage0 = {
            .pixels = iwebcampngPixels,
            .maskAlpha = iwebcampngMask,
            .cb = iwebcampng_cb,
            // [EdgeWX] Sem rotacao. Largura e altura sao as constantes do header, 32x32.
            .angle = 0,
            .width = iwebcampngW,
            .height = iwebcampngH,
            .backgroundColor = CFK_GREY3
        };
    // [EdgeWX] setupFromPixels e o caminho de imagem embutida. Arquivo em SD usaria outra struct.
    iwebcampng.setupFromPixels(configImage0);
    myDisplay.setImage(arrayImagem,qtdImagem);


    // [EdgeWX] Preview 353x223. Cabe no cartao 380x292. Sem rotulo de zoom nem de fps, a 5 quadros por segundo.
    CameraMipiConfig configCameraMipi0 = {
            .width = 353,
            .height = 223,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    // [EdgeWX] Registra o preview. createTask so sobe a tarefa da camera se startCamera teve sucesso.
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


}

// [EdgeWX] Texto fixo do gerador. O callback roda na tarefa de UI e deve ser curto.
// This function is a callback of this element slidervelocidad.
// You dont need call it. Make sure it is as short and quick as possible.
void slidervelocidad_cb(){
    // [EdgeWX] Copia o valor da trilha para o int que o exportador criou.
    hslider0_val = slidervelocidad.getValue();
    // [manual] Leva o mesmo numero para a agulha. O gerador nao liga slider e gauge sozinho.
    gauge.setValue(hslider0_val);
}
// [EdgeWX] Callback do botao INICIAR.
// This function is a callback of this element btnligar.
// You dont need call it. Make sure it is as short and quick as possible.
void btnligar_cb(){

    // [manual] A chave precisa estar ligada. Com ela desligada, o toque so chega no Serial.
    if(switch0_val){
        // [manual] Libera updateContadores.
        esteiraLigada = true;
        // [manual] Zera a marca de tempo, as pecas, os defeitos e o relogio.
        tempoTotal = 0;
        totalPecas = 0;
        totalPecasDanificadas = 0;
        tempoFuncionamento = 0;
        // [manual] Troca o par de LEDs para verde.
        setStatusLed(true);
    }

    // [EdgeWX] Log que o gerador coloca no corpo de todo TextButton.
    Serial.print("Clicked on: ");Serial.println("btnligar_cb");
}
// [EdgeWX] Callback do botao PARAR.
// This function is a callback of this element btnparar.
// You dont need call it. Make sure it is as short and quick as possible.
void btnparar_cb(){
    // [manual] Trava os contadores. Nao zera os labels; eles ficam no ultimo numero.
    esteiraLigada = false;
    // [manual] Volta o par de LEDs para vermelho.
    setStatusLed(false);
    // [EdgeWX] Log original do export.
    Serial.print("Clicked on: ");Serial.println("btnparar_cb");
}
// [EdgeWX] Callback da chave. O corpo padrao so grava a bool.
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){

    // [EdgeWX] true com a capsula a direita. btnligar_cb le esta variavel.
    switch0_val = toggle1.getStatus();
}
// [EdgeWX] Callback do icone. As duas ultimas linhas sao o texto padrao de imagem do gerador.
// This function is a callback of this element iwebcampng.
// You dont need call it. Make sure it is as short and quick as possible.
void iwebcampng_cb(){
    // [manual] Abre o ajuste de ROI da camera. beginCameraConfig exige a tarefa da camera ja criada.
    cameraMipi.openConfig();
    // [EdgeWX] Comentario que o gerador grava no callback de cada Image.
    // Image Webcampng clicked
    // [EdgeWX] Log padrao do toque na imagem.
    Serial.println("Image Webcampng clicked");
}
