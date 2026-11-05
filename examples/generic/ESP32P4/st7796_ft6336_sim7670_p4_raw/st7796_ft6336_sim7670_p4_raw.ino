// [legenda] [manual] = sketch inteiro escrito a mao. O display e o modem ficam nos .h e .cpp vizinhos.
// [legenda] setup sobe modem, tela e socket. loop grava no Firebase quando um botao pede.
// [manual] Inclui cores.h.
#include "cores.h"
// [manual] Inclui variaveis.h.
#include "variaveis.h"
// [manual] Inclui touch.h.
#include "touch.h"
// [manual] Inclui modem.h.
#include "modem.h"
// [manual] Inclui display.h.
#include "display.h"
// [manual] Inclui rede_wifi.h.
#include "rede_wifi.h"


// [manual] Abre um bloco comentado. Nao executa.
/* @brief Rotina de inicialização do Arduino: configura periféricos, modem, WiFi e UI. */
// [manual] Sobe serial, painel, toque e a tarefa de UI.
void setup()
{
    // [manual] Configura o pino como entrada ou saida.
    pinMode(POTENCIOMETRO, INPUT);
    // [manual] Abre a serial a 115200.
    Serial.begin(115200);
    

    // [manual] Abre a serial do modem SIM7670.
    startModemSerial();

    // [manual] Sobe o ST7796. A funcao mora em display.h.
    startDisplay();
    // [manual] Sobe o FT6336. A funcao mora em touch.h.
    startTouch();
    
    // [manual] Sobe a tarefa que desenha este demo.
    createTaskTestDisplay();
    // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(100);
    // [manual] Escreve o estado na tela.
    showMessageStatus("Iniciando...");
    // [manual] LED de ocupado enquanto o modem sobe.
    showStatusLed(LedStatus::BUSY);
    
    // [manual] Variavel deste sketch.
    bool allReady = waitModemReady();
    // [manual] Condicao ou repeticao.
    if(!allReady){
        // [manual] Mensagem na serial.
        Serial.println("Modem not ready!");
        // [manual] Reinicia o modem se ele nao respondeu a tempo.
        fullRestartModem();
    }

    // [manual] Configura o modem depois que ele responde.
    setupModem();

    // [manual] Tenta o link. Se ja estiver conectado, a funcao volta na hora.
    conectarWiFi();
    // [manual] Abre o socket de novo se ele caiu.
    conectarSocket();

    // [manual] A tela sai do estado de subida.
    showMessageStatus("Pronto!");
    // [manual] LED de pronto.
    showStatusLed(LedStatus::READY);
}
// [manual] Abre um bloco comentado. Nao executa.
/* @brief Laço principal: trata eventos de UI, comunicação TCP e operações Firebase. */
// [manual] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop()
{
    // [manual] Le o que o modem mandou nesta volta.
    serialModemMonitor();

    // [manual] Condicao ou repeticao.
    if(rectButtons[idButtonSend].pressed || precisaGravarNoFirebase){
        // [manual] Consome o clique de gravar.
        rectButtons[idButtonSend].pressed = false;
        // [manual] Consome o pedido de gravar.
        precisaGravarNoFirebase = false;
        // [manual] Mensagem na serial.
        Serial.println("Gravar novo item...");
        // [manual] IP vazio se o Wi-Fi ainda nao tiver link.
        String ip = "";
        
        // [manual] Condicao ou repeticao.
        if(WiFi.status() == WL_CONNECTED) ip = WiFi.localIP().toString();

        // [manual] JSON com IP, millis e o potenciometro.
        String json = "{\"ip\":\"" + ip + "\", \"millis\":" + String(millis()) + ", \"potenciometro\":" + String(analogRead(POTENCIOMETRO)) + "}";
        // [manual] Comentario que ja estava no arquivo.
        // {ip:"xxxxxx", millis:xxxxx, potenciometro:xxxxx}
        // [manual] Grava essa leitura no Firebase.
        postDataFirebase("/leituras.json", json);
    }
    // [manual] Condicao ou repeticao.
    if(rectButtons[idButtonUpdate].pressed || precisaAtualizarNoFirebase){
        // [manual] Consome o clique de atualizar.
        rectButtons[idButtonUpdate].pressed = false;
        // [manual] Consome o outro pedido de atualizar.
        precisaAtualizarNoFirebase = false;
        // [manual] Mensagem na serial.
        Serial.println("Update item...");
        // [manual] Variavel deste sketch.
        int aleatorio = random(1000);
        // [manual] JSON curto, so com um numero sorteado.
        String jsonIdade = "{\"aleatorio\":" + String(aleatorio) + "}";

        // [manual] Atualiza o campo no Firebase.
        updateDataFirebase("/outrocampo.json", jsonIdade);
    }

    // [manual] Condicao ou repeticao.
    if(rectButtons[idButtonHello].pressed){
        // [manual] Consome o clique de hello.
        rectButtons[idButtonHello].pressed = false;
        // [manual] Manda a frase pelo socket.
        enviarDados("Hello Server!");
    }

    // [manual] Condicao ou repeticao.
    if(rectButtons[idButtonTeste].pressed){
        // [manual] Consome o clique de teste.
        rectButtons[idButtonTeste].pressed = false;
        // [manual] Mensagem na serial.
        Serial.println("Teste MonitorSerial...");
    }



    // [manual] Condicao ou repeticao.
    if (!client.connected()) {
        // [manual] Abre o socket de novo se ele caiu.
        conectarSocket();
    }

    // [manual] Le o que chegou no socket.
    String recebido = lerDados();
    // [manual] Condicao ou repeticao.
    if (recebido.length() > 0) {
        // [manual] Executa o comando que chegou.
        interpretarComando(recebido);
    }

    // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(10);
}
