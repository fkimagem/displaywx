// [legenda] [EdgeWX] = esqueleto do display ST7796 e do FT6336. [manual] = fila da Serial1, Wi-Fi e o radar de redes.
// [legenda] O modem nao e aberto aqui. SCAN_REDES nasce comentado, entao o radar nao roda.
// [manual] Constante de preprocessador.
#define P4_WIFI6
// [manual] Linha de codigo desligada. Nao executa.
//#define SCAN_REDES 1


// [manual] Tipo de uma rede achada no scan.
typedef struct
{
    // [manual] Canal Wi-Fi.
    int32_t channel;
    // [manual] Indice do canal, canal menos um.
    int16_t idx;
    // [manual] Potencia recebida, em dBm.
    int32_t rssi;
    // [manual] Nome da rede.
    String ssid;
// [manual] Fecha o tipo Rede_t.
} Rede_t;

// [manual] Constante de preprocessador.
#define MAX_REDES 10
// [manual] Constante de preprocessador.
#define QUEUE_SIZE 10  // Tamanho da fila de strings
// [manual] Constante de preprocessador.
#define LINE_HEIGHT 30

// [manual] Lista das redes vistas no scan.
Rede_t redes[MAX_REDES];
// [manual] Variavel deste sketch.
int lastCountRedes = 0;

// [manual] Buffer circular simples das linhas da Serial1.
// Fila de strings para armazenar dados recebidos via Serial1
// [manual] Fila das linhas que chegaram pela Serial1.
String messageQueue[QUEUE_SIZE];
// [manual] Variavel deste sketch.
int queueCount = 0;  // Número atual de itens na fila
// [manual] Variavel deste sketch.
bool queueUpdated = false;  // Flag para indicar que a fila foi atualizada

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
// [manual] Wi-Fi deste demo. O export do display nao inclui essa biblioteca.
#include "WiFi.h"
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
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
const int TOUCH_MAP_X1 = 320;
// [EdgeWX] Canto do mapa de toque, antes da calibracao.
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
// [manual] Cerca de compilacao.
#if defined(P4_WIFI6)
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SCL = 31;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SDA = 30;
// [EdgeWX] Cerca de compilacao.
#else
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SCL = 18;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_SDA = 17;
// [EdgeWX] Cerca de compilacao.
#endif

// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_INT = 47;
// [EdgeWX] Pino gravado na ficha deste projeto.
const int TOUCH_RST = 33;

// [manual] Constante de preprocessador.
#define RX1_PIN 49
// [manual] Constante de preprocessador.
#define TX1_PIN 50

// [EdgeWX] O gerador declara uma funcao por tela mais loadWidgets.
// Prototypes for each screen
// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0();
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets();
// [EdgeWX] Um prototipo por widget que tem callback no editor.
// Prototypes for callback functions
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
// [EdgeWX] Rotacao 0..3 do painel. O comentario em ingles no fim da linha e do gerador.
uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
// [EdgeWX] Objeto unico da biblioteca. Registra widgets, toque, camera e a tarefa.
DisplayWX myDisplay;
// [EdgeWX] O comentario descreve o construtor WidgetBase: x, y e indice da tela.
// Create global objects. Constructor is: xPos, yPos and indexScreen
// [EdgeWX] Sobe serial, painel, toque e a tarefa de UI.
void setup()
{
    // [EdgeWX] Abre a serial a 115200.
    Serial.begin(115200);
    // [manual] UART do modem SIM7670 nos GPIO 49 e 50, 115200 8N1.
    Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
    // [EdgeWX] SPI compartilhado do painel. No S3 o gerador escolhe FSPI; nos outros, HSPI.
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    // [EdgeWX] Barramento SPI do display, com os GPIO da ficha.
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    // [EdgeWX] Driver do painel SPI. isIPS e a rotacao entram aqui.
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, true);
    // [EdgeWX] Inicializa o painel SPI no clock DISP_FREQUENCY.
    tft->begin(DISP_FREQUENCY);
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
    // [EdgeWX] Sobe o FT6336 no I2C indicado.
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    // [EdgeWX] Liga o log de toque. Em varios exports a mesma linha sai comentada; neste ficou ativa.
    myDisplay.enableTouchLog();

    // [manual] Comentario deixado no meio do setup. A tela ja subiu nas linhas de cima.
    // Init Display
    // [manual] Modo estacao, sem subir o display de novo.
    WiFi.mode(WIFI_STA);
    // [manual] Solta uma associacao antiga antes do restante do sketch.
    WiFi.disconnect();
    // [manual] Pausa curta depois de soltar o Wi-Fi.
    delay(100);

    // [manual] A chamada foi comentada a mao. loadWidgets deste arquivo esta vazio.
    // loadWidgets(); // This function is used to setup with widget individualy
    // [EdgeWX] Registra a funcao da tela. A tarefa de UI chama esse desenho.
    myDisplay.loadScreen(screen0);  // Use this line to change between screens
    // [EdgeWX] Cria a tarefa de toque e desenho. false desliga o watchdog; 3 e o timeout em segundos, guardado mas sem WDT.
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}
// [manual] Le a Serial1, ecoa no monitor e guarda cada linha.
void readSerial(){
    // [manual] Junta os bytes ate chegar o fim da linha.
    static String receivedMessage = "";  // Buffer para acumular caracteres
    
    // [manual] Condicao ou repeticao.
    while (Serial1.available())
    {
        // [manual] Variavel deste sketch.
        int byteRecebido = Serial1.read();
        // [manual] Ecoa o byte do modem no monitor.
        Serial.write(byteRecebido);
        
        // [manual] Byte normal entra no buffer.
        // Se não for quebra de linha, adiciona ao buffer
        // [manual] Condicao ou repeticao.
        if (byteRecebido != '\n') {
            // [manual] Anexa o caractere ao buffer da linha.
            receivedMessage += (char)byteRecebido;
        // [manual] Ramo falso da condicao de cima.
        } else {
            // [manual] Fim de linha: trata o que acumulou.
            // Encontrou quebra de linha, processa a mensagem
            // [manual] Tira espaco e o CR que veio antes do LF.
            receivedMessage.trim();  // Remove espaços em branco no início e fim
            
            // [manual] Linha vazia nao entra na fila.
            // Se a mensagem tem caracteres impressos, adiciona à fila
            // [manual] Condicao ou repeticao.
            if (receivedMessage.length() > 0) {
                // [manual] Descarta a mais antiga.
                // Se a fila está cheia, remove o primeiro item e realoca
                // [manual] Condicao ou repeticao.
                if (queueCount >= QUEUE_SIZE) {
                    // [manual] Abre espaco no inicio.
                    // Remove o primeiro item e move todos para cima
                    // [manual] Condicao ou repeticao.
                    for (int i = 0; i < QUEUE_SIZE - 1; i++) {
                        // [manual] Desloca a fila para jogar fora a linha mais antiga.
                        messageQueue[i] = messageQueue[i + 1];
                    }
                    // [manual] A fila fica com uma vaga no fim.
                    queueCount = QUEUE_SIZE - 1;
                }
                
                // [manual] A linha nova fica no fim.
                // Adiciona o novo item no final
                // [manual] A linha nova entra no fim.
                messageQueue[queueCount] = receivedMessage;
                // [manual] Conta a linha que acabou de entrar.
                queueCount++;
                // [manual] printMessages vai redesenhar.
                queueUpdated = true;  // Marca que a fila foi atualizada
            }
            
            // [manual] A proxima linha comeca vazia.
            receivedMessage = "";  // Limpa o buffer para a próxima mensagem
        }
    }

    // [manual] O que for digitado no monitor sai pela Serial1.
    // Se houver dados digitados no monitor serial, envia para a Serial1
    // [manual] Condicao ou repeticao.
    while (Serial.available())
    {
        // [manual] Variavel deste sketch.
        int byteDigitado = Serial.read();
        // [manual] O que for digitado no monitor sai para o modem.
        Serial1.write(byteDigitado);
    }
}
// [manual] Se chegou linha nova, apaga a tela e reescreve a fila.
void printMessages(){
// [manual] So redesenha quando chegou linha nova.
// Verifica se a fila foi atualizada
    // [manual] Condicao ou repeticao.
    if (queueUpdated) {
        // [manual] Segura o desenho para a lista nao rasgar no meio.
        myDisplay.startTransaction();
        // [manual] Variavel deste sketch.
        int fixX = 5;
        // [manual] Variavel deste sketch.
        int startY = 5;
        // [manual] Pinta o fundo da tela com a cor desta chamada.
        tft->fillScreen(CFK_WHITE);
        // [manual] Mensagem na serial.
        Serial.println("=== FILA DE MENSAGENS ===");
        // [manual] Condicao ou repeticao.
        for (int i = 0; i < queueCount; i++) {
            // [manual] Mensagem na serial.
            Serial.print("[" + String(i) + "] ");
            // [manual] Mensagem na serial.
            Serial.println(messageQueue[i]);
            // [manual] Chamada de exemplo deixada comentada.
            //myDisplay.printText("Custom text", 5, 7, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
            // [manual] Lembrete escrito a mao, com typo, do passo vertical.
            // Prin line start x = 5, y = 5, increment  y by variable LINE_HEIGHT
            // [manual] Texto estatico. A ancora fica no ponto passado para printText.
            myDisplay.printText(messageQueue[i].c_str(), fixX, startY, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
            // [manual] Desce uma linha de texto.
            startY += LINE_HEIGHT;

        }
        // [manual] Devolve o desenho para a tarefa de UI.
        myDisplay.finishTransaction();
        // [manual] Mensagem na serial.
        Serial.println("========================");
        // [manual] A tela ja mostra esta fila.
        queueUpdated = false;  // Reseta a flag
    }
}
// [EdgeWX] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop()
{
    // [manual] Drena a Serial1.
    readSerial();
    // [manual] Redesenha se a fila mudou.
    printMessages();
    
    
    // [manual] Folga de 2 ms entre uma leitura da serial e a proxima.
    delay(2);
}

// [manual] Varre o Wi-Fi. So compila se SCAN_REDES estiver ligado.
void scan()
{
    // [manual] Cerca de compilacao.
    #ifdef SCAN_REDES
// [manual] Cerca de compilacao.
#if defined(ESP32)
    // [manual] Variavel deste sketch.
    int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */, true /* passive */, 500 /* max_ms_per_chan */);
// [manual] Cerca de compilacao.
#else
    // [manual] Variavel deste sketch.
    int n = WiFi.scanNetworks(false /* async */, true /* show_hidden */);
// [manual] Cerca de compilacao.
#endif

    // [manual] Variavel deste sketch.
    int minRSSI = -80;
    // [manual] Zera a lista antes de preencher de novo.
    lastCountRedes = 0;

    // [manual] Condicao ou repeticao.
    for (int i = 0; i < n && i < MAX_REDES; i++)
    {

        // [manual] Guarda o canal desta rede.
        redes[lastCountRedes].channel = WiFi.channel(i);
        // [manual] Guarda o canal desta rede.
        redes[lastCountRedes].idx = redes[lastCountRedes].channel - 1;
        // [manual] Guarda a potencia, em dBm.
        redes[lastCountRedes].rssi = WiFi.RSSI(i);
        // [manual] Guarda o nome da rede.
        redes[lastCountRedes].ssid = WiFi.SSID(i);

        // [manual] Condicao ou repeticao.
        if (redes[lastCountRedes].rssi < minRSSI)
            // [manual] Pula esta rede. A potencia ficou abaixo do minimo.
            continue;

        // [manual] Mensagem na serial.
        Serial.printf("%i - Channel: %02i\t", i, redes[lastCountRedes].channel);
        // [manual] Mensagem na serial.
        Serial.printf("Idx: %i\t", redes[lastCountRedes].idx);
        // [manual] Mensagem na serial.
        Serial.printf("RSSI: %i\t", redes[lastCountRedes].rssi);
        // [manual] Mensagem na serial.
        Serial.printf("SSID: %s\n", redes[lastCountRedes].ssid.c_str());

        // [manual] Esta rede entrou na lista.
        lastCountRedes++;
    }
    // [manual] Mensagem na serial.
    Serial.println("------------------");
    // [manual] Pede para a tela desenhar de novo.
    myDisplay.refreshScreen();
    // [manual] Cerca de compilacao.
    #endif
}

// [EdgeWX] Desenho estatico desta tela. A tarefa chama ao carregar.
void screen0()
{
    // [EdgeWX] Pinta o fundo e guarda a cor para apagar texto.
    myDisplay.setBackgroundColor(CFK_WHITE);
    // [manual] Texto estatico. A ancora fica no ponto passado para printText.
    myDisplay.printText("Aguardando comandos", 5, 5, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    // [EdgeWX] Abre um bloco comentado. Nao executa.
    /*tft->fillCircle(25, 25, 19, CFK_COLOR01);
    tft->drawCircle(25, 25, 19, CFK_COLOR01);
    tft->fillCircle(40, 35, 19, CFK_COLOR11);
    tft->drawCircle(40, 35, 19, CFK_COLOR01);
    tft->fillCircle(55, 45, 19, CFK_COLOR22);
    tft->drawCircle(55, 45, 19, CFK_COLOR01);*/

    

    // [manual] Cerca de compilacao.
    #ifdef SCAN_REDES

    // [EdgeWX] Variavel deste sketch.
    int raioMin = 10;
    // [EdgeWX] Variavel deste sketch.
    int raioMax = 150;
    // [EdgeWX] Variavel deste sketch.
    int quantidade = 5;
    // [EdgeWX] Variavel deste sketch.
    int passo = (raioMax - raioMin) / quantidade;
    // [EdgeWX] Condicao ou repeticao.
    for (auto i = 0; i <= quantidade; i++)
    {
        // [EdgeWX] Variavel deste sketch.
        int raio = raioMin + passo * i;
        // [EdgeWX] Contorna um circulo estatico.
        tft->drawCircle(240, 160, raio, CFK_COLOR01);
    }
    
    // [manual] Condicao ou repeticao.
    for (auto i = 0; i < lastCountRedes; i++)
    {
        // [manual] Copia uma rede para calcular a posicao no radar.
        Rede_t item = redes[i];
        // [EdgeWX] Variavel deste sketch.
        int rndAngle = random(0, 360);
        // [EdgeWX] Variavel deste sketch.
        int prop = map(item.rssi, -10, -80, raioMin, raioMax);
        // [EdgeWX] Variavel deste sketch.
        float anguloRad = rndAngle * PI / 180.0;

        // [EdgeWX] Variavel deste sketch.
        int calcX = cos(anguloRad) * prop;
        // [EdgeWX] Variavel deste sketch.
        int calcY = sin(anguloRad) * prop;

        // [EdgeWX] Variavel deste sketch.
        int posX = 240 + calcX;
        // [EdgeWX] Variavel deste sketch.
        int posY = 160 + calcY;

        // [EdgeWX] Preenche um circulo estatico.
        tft->fillCircle(posX, posY, 2, CFK_COLOR22);
        // [manual] Texto estatico. A ancora fica no ponto passado para printText.
        myDisplay.printText(item.ssid.c_str(), posX, posY, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular5pt7b);
    }
    // [EdgeWX] Cerca de compilacao.
    #endif

    // [EdgeWX] Instancia e desenha so os widgets cujo indice de tela e o desta chamada.
    myDisplay.drawWidgetsOnScreen(0);
}
// [EdgeWX] Comentario fixo do gerador, inclusive o typo widgtes.
// Configure each widgtes to be used
// [EdgeWX] Preenche a struct de cada widget e registra os arrays.
void loadWidgets()
{
}
