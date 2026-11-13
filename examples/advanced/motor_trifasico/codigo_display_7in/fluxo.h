#pragma once

// Interface e programas pertencem a tarefa grafica; somente tarefaLinker usa
// clientLinker. O loop agenda servicoInterface(), sem executar operacoes de rede.
#include <Arduino.h>
#include <WiFi.h>
#include <LinkerWX.h>
#include <LinkerWXWiFi.h>
#include <displaywx.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <esp_log.h>
#include <atomic>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "controle_conexao.h"
#include "controle_comandos.h"

extern DisplayWX myDisplay; ///< Interface grafica definida no sketch do display.
extern GaugeSuper gaugerpm; ///< Medidor de RPM definido no sketch do display.
extern Label valuefact, valuefset, setrpmvalue, text8copycopy, labelservstat; ///< Textos de frequencia, RPM e estado definidos no sketch.
extern HSlider slidergeral; ///< Slider manual da tela principal definido no sketch.
extern Label labelhsliderge; ///< Texto numerico do slider geral definido no sketch.
extern Led led1; ///< LED de estado da conexao definido no sketch.
extern LineChart graphfreq, graphfreqcopycopy, graphfreqcopycopycopy; ///< Graficos de RPM das tres telas definidos no sketch.
extern VAnalog analogvdc, analogtout, analogcur; ///< Barras de tensao DC, tensao de saida e corrente definidas no sketch.
extern int hslider0_val, hslider1_val, hslider2_val; ///< Ultimos percentuais dos sliders geral, programa 1 e programa 2.
extern float nb0_val, nb1_val, nb2_val, nb3_val, nb4_val; ///< Tempos, frequencia maxima e RPM maximo configurados no programa 1.
extern float nb5_val, nb6_val, nb7_val, nb8_val, nb9_val; ///< Tempos, frequencia maxima e RPM maximo configurados no programa 2.

namespace Fluxo {

// Credenciais da rede WiFi
inline const char*    ssid       = "Labmaker"; ///< Nome da rede Wi-Fi usada pelo display.
inline const char*    password   = "Anglo2020pp"; ///< Senha da rede Wi-Fi usada pelo display.

// Servidor TLS usado nos exemplos atuais da LinkerWX
inline const char*    serverHost = LinkerWX::SERVER_ADDRESS; ///< Endereco do servidor TLS da LinkerWX.
inline const uint16_t serverPort = LinkerWX::SERVER_PORT; ///< Porta do servidor TLS da LinkerWX.

// ISRG Root X1 completo, conferido pelo SHA-256:
// 96BCEC06264976F37460779ACF28C5A7CFE8A3C0AAE11A8FFCEE05C0BDDF08C6
// https://letsencrypt.org/certs/isrgrootx1.pem
inline const char* ROOT_CA =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "m+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
    "-----END CERTIFICATE-----\n"; ///< Certificado raiz ISRG Root X1, em PEM, usado na conexao TLS.

// Identificação deste dispositivo no protocolo LinkerWX
// Este display é o destino 1:2 (grupo 1, ID 2)
inline const uint16_t DEVICE_ID = 2; ///< Identificador deste display no protocolo LinkerWX.
inline const uint16_t GRUPO     = 1; ///< Grupo deste display no protocolo LinkerWX.
inline const String   TOKEN     = "987654321"; ///< Token de autenticacao deste display no servidor.


inline constexpr uint16_t TARGET_COMMAND_GROUP = 1; ///< Grupo que recebe os comandos enviados em broadcast.
// A origem serve apenas para filtrar telemetria; comandos usam broadcast.
inline constexpr uint16_t MOTOR_SOURCE_GROUP = 1; ///< Grupo de origem aceito na telemetria do motor.
inline constexpr uint16_t MOTOR_SOURCE_ID = 1; ///< Identificador de origem aceito na telemetria do motor.
inline constexpr uint32_t COMMAND_SEND_INTERVAL_MS = 500; ///< Intervalo minimo entre broadcasts de comando, em milissegundos.
inline constexpr uint32_t RECONNECT_INTERVAL_MS = 60000; ///< Espera minima entre tentativas de reconexao, em milissegundos.
inline constexpr uint32_t REGISTER_TIMEOUT_MS = 10000; ///< Prazo para receber o registro depois do TLS, em milissegundos.
// O TWDT tem timeout global. Mantem margem para o handshake TLS padrao de
// ate 120 s sem confundir indisponibilidade da rede com travamento da tarefa.
inline constexpr uint32_t TASK_WDT_TIMEOUT_MS = 240000; ///< Prazo do watchdog de tarefas, em milissegundos.
inline constexpr uint16_t TASK_WDT_TIMEOUT_S = TASK_WDT_TIMEOUT_MS / 1000; ///< Mesmo prazo do watchdog, expresso em segundos.
inline constexpr uint32_t TELEMETRY_MAX_AGE_MS = 5000; ///< Idade maxima da telemetria ainda considerada valida, em milissegundos.
inline constexpr uint32_t UI_SERVICE_INTERVAL_MS = 50; ///< Intervalo em que o loop pede uma atualizacao da interface, em milissegundos.
inline constexpr uint32_t NETWORK_TASK_STACK_BYTES = 12288; ///< Pilha reservada para a tarefa de rede, em bytes.

inline LinkerWXWiFi clientLinker; ///< Cliente LinkerWX usado somente pela tarefa de rede.
inline FilaComandosGrupo comandos; ///< Fila de comandos compartilhada entre a interface e a rede.
inline portMUX_TYPE comandosMux = portMUX_INITIALIZER_UNLOCKED; ///< Bloqueio que protege a fila e o estado da sessao.
inline std::atomic<bool> quedaDetectada{false}; ///< Sinaliza que a rede caiu e a tarefa deve desconectar.
inline std::atomic<bool> servicoUiPendente{false}; ///< Indica que uma atualizacao da interface ja foi agendada.
inline std::atomic<bool> watchdogUiAtivo{false}; ///< Indica que a tarefa grafica esta inscrita no watchdog.
inline bool watchdogLoopAtivo = false; ///< Indica que o loop esta inscrito no watchdog.
// Rede publica; tarefa grafica consome. Protegidos por comandosMux.
inline bool sliderRemotoPendente = false; ///< Ha um valor de slider recebido de outro display aguardando a tela.
inline int sliderRemotoValor = 0; ///< Percentual de slider recebido de outro display.
inline uint32_t sliderRemotoGeracao = 0; ///< Geracao da sessao em que o slider remoto foi recebido.

/**
 * @brief Le o estado e a geracao da sessao sob o bloqueio da fila.
 * @return Copia da sessao observada no momento da chamada.
 */
inline EstadoSessao lerSessao() {
    portENTER_CRITICAL(&comandosMux);
    const EstadoSessao sessao = comandos.sessao();
    portEXIT_CRITICAL(&comandosMux);
    return sessao;
}

/**
 * @brief Publica o estado da conexao e avanca a geracao quando ele muda.
 * @param estado Novo estado observado pela tarefa de rede.
 */
inline void publicarEstado(EstadoConexao estado) {
    portENTER_CRITICAL(&comandosMux);
    // Um evento de queda nao pode ser sobrescrito por um poll() que acabou
    // de retornar com um estado de enlace ainda desatualizado.
    if (estado != EstadoConexao::CONECTADO || !quedaDetectada.load()) {
        comandos.mudarEstado(estado);
    }
    portEXIT_CRITICAL(&comandosMux);
}

/**
 * @brief Confere se a geracao ainda pertence a sessao conectada.
 * @param geracao Geracao capturada quando o comando ou a amostra foi criada.
 * @return true quando a sessao continua conectada com essa geracao.
 */
inline bool sessaoValida(uint32_t geracao) {
    portENTER_CRITICAL(&comandosMux);
    const bool valida = comandos.pertenceASessao(geracao);
    portEXIT_CRITICAL(&comandosMux);
    return valida;
}

// O evento WiFi e os callbacks de erro fecham imediatamente a entrada da fila.
// Nenhum deles desenha widgets nem alimenta o watchdog de outra tarefa.
/**
 * @brief Marca a sessao como offline e pede que a tarefa de rede desconecte.
 */
inline void sinalizarQueda() {
    portENTER_CRITICAL(&comandosMux);
    comandos.mudarEstado(EstadoConexao::OFFLINE);
    quedaDetectada.store(true);
    portEXIT_CRITICAL(&comandosMux);
}

struct MotorData {
    float    fa  = 0.0f;  // frequência atual (Hz)
    float    fs  = 0.0f;  // frequência setada (Hz)
    float    v   = 0.0f;  // velocidade
    uint32_t rpm = 0;     // rotações por minuto
    float    tq  = 0.0f;  // torque (%)
    float    p   = 0.0f;  // potência (W)
    float    vdc = 0.0f;  // tensão do barramento DC (V)
    float    vs  = 0.0f;  // tensão de saída (V)
    float    i   = 0.0f;  // corrente de saída (A)
    float    tm  = 0.0f;  // temperatura do motor (°C)
    float    a1  = 0.0f;  // analógico 1
    float    a2  = 0.0f;  // analógico 2
    float    a3  = 0.0f;  // analógico 3
    uint16_t edo = 0;     // estado das saídas digitais
    uint16_t er  = 0;     // código de erro
    uint32_t tl  = 0;     // tempo ligado (s)
    float    tr  = 0.0f;  // tempo restante
    uint16_t cf  = 0;     // código de falha
    uint16_t inf = 0;     // informação adicional
    bool     fresh       = false; // true quando há dados novos não consumidos
    uint32_t lastUpdateMs = 0;   // timestamp da última atualização (ms)
};


struct AmostraMotor {
    MotorData dados;
    uint32_t geracao;
};
inline QueueHandle_t filaTelemetria = nullptr; ///< Fila de uma posicao que leva a amostra do motor ate a interface.
inline MotorData motorData; ///< Ultima amostra apresentada na tela; somente servicoInterface a utiliza.

// Estes estados representam comandos solicitados, nao confirmacao fisica.
enum ProgState { PROG_IDLE, PROG_WAITING, PROG_RUNNING, PROG_STOPPED };
struct ProgConfig {
    float freqMax;
    float rpmMax;
    uint32_t startDelay;
    uint32_t runTime;
    uint32_t stopTime;
};
inline ProgState progState = PROG_IDLE; ///< Etapa atual do programa local de partida e parada.
inline ProgConfig progCfg = {}; ///< Tempos e limites copiados quando o programa local comeca.
inline uint32_t progStateMs = 0; ///< Instante, em milissegundos, em que a etapa atual do programa comecou.
inline uint32_t progGeracao = 0; ///< Geracao da sessao valida para os comandos deste programa.
inline bool progActive = false; ///< Verdadeiro enquanto um programa local ainda controla o motor.
inline int currentProg = 1; ///< Numero do programa local em execucao, 1 ou 2.
inline bool manualPendente = false; ///< Ha um ajuste manual ainda nao enviado ao motor.
inline uint32_t manualGeracao = 0; ///< Geracao da sessao em que o ajuste manual foi pedido.
inline int manualValor = 0; ///< Percentual manual que sera enviado ao motor.

/**
 * @brief Aceita um comando textual ou um numero de frequencia maior que zero.
 * @param cmd Texto do comando, terminado em zero.
 * @return true quando o texto cabe na fila e tem conteudo aceito.
 */
inline bool comandoValido(const char* cmd) {
    if (!cmd || !cmd[0] || strlen(cmd) >= sizeof(CmdQueueItem::text)) return false;
    const bool numerico = isdigit(static_cast<unsigned char>(cmd[0])) || cmd[0] == '-';
    return !numerico || atof(cmd) > 0.0f;
}

/**
 * @brief Enfileira um comando se ele for valido e pertencer a sessao informada.
 * @param cmd Texto do comando enviado ao motor.
 * @param geracao Geracao da sessao em que o comando foi criado.
 * @return true quando o comando entrou na fila.
 */
inline bool sendCommandToMotor(const char* cmd, uint32_t geracao) {
    if (!comandoValido(cmd)) return false;
    portENTER_CRITICAL(&comandosMux);
    const bool aceito = comandos.pertenceASessao(geracao) && comandos.enfileirar(cmd);
    portEXIT_CRITICAL(&comandosMux);
    if (!aceito) ESP_LOGW("QUEUE", "Comando nao enfileirado (offline/conectando/fila cheia): %s", cmd);
    return aceito;
}

/**
 * @brief Enfileira um comando usando a geracao da sessao lida agora.
 * @param cmd Texto do comando enviado ao motor.
 * @return true quando o comando entrou na fila.
 */
inline bool sendCommandToMotor(const char* cmd) {
    return sendCommandToMotor(cmd, lerSessao().geracao);
}

/**
 * @brief Guarda um ajuste manual para envio posterior, sem comandar a rede na hora do toque.
 * @param valor Percentual escolhido no slider geral.
 */
inline void atualizarSliderManual(int valor) {
    const EstadoSessao sessao = lerSessao();
    manualPendente = !progActive && sessao.estado == EstadoConexao::CONECTADO && valor > 0;
    manualValor = valor;
    manualGeracao = sessao.geracao;
    // Uma edicao offline fica visivel, mas nao vira comando ao reconectar.
}

/**
 * @brief Encerra o programa local e volta o estado para ocioso.
 */
inline void cancelarProgramaLocal() {
    if (progActive) ESP_LOGW("PROG", "Programa local interrompido; nova partida deve ser solicitada.");
    progActive = false;
    progState = PROG_IDLE;
}

// Executadas somente pelos callbacks/servico da tarefa grafica.
/**
 * @brief Copia a configuracao da tela e comeca a espera do programa informado.
 * @param progNum Programa escolhido: 1 ou 2.
 */
inline void progStart(int progNum) {
    const EstadoSessao sessao = lerSessao();
    if (sessao.estado != EstadoConexao::CONECTADO) {
        ESP_LOGW("PROG", "Partida recusada: Linker nao conectado.");
        return;
    }
    currentProg = progNum;
    if (progNum == 1) {
        progCfg = {nb3_val, nb4_val, (uint32_t)nb0_val, (uint32_t)nb1_val, (uint32_t)nb2_val};
    } else {
        progCfg = {nb8_val, nb9_val, (uint32_t)nb5_val, (uint32_t)nb6_val, (uint32_t)nb7_val};
    }
    progGeracao = sessao.geracao;
    progState = PROG_WAITING;
    progStateMs = millis();
    progActive = true;
    manualPendente = false;
    ESP_LOGI("PROG", "Programa %d iniciado localmente.", progNum);
}

/**
 * @brief Cancela o programa local e tenta enfileirar o comando STOP.
 */
inline void progStop() {
    cancelarProgramaLocal();
    manualPendente = false;
    if (!sendCommandToMotor("STOP")) ESP_LOGW("PROG", "STOP nao foi enfileirado para transmissao.");
}

/**
 * @brief Coloca o setpoint e o comando START na fila como um unico lote.
 * @param setpoint Percentual de velocidade que antecede a partida.
 * @return true quando os dois comandos foram aceitos na sessao do programa.
 */
inline bool enfileirarPartida(int setpoint) {
    if (setpoint <= 0) return false;
    char texto[16];
    snprintf(texto, sizeof(texto), "%d", setpoint);
    const char* lote[] = {texto, "START"};
    portENTER_CRITICAL(&comandosMux);
    const bool aceito = comandos.pertenceASessao(progGeracao) && comandos.enfileirarLote(lote, 2);
    portEXIT_CRITICAL(&comandosMux);
    return aceito;
}

/**
 * @brief Avanca espera, partida, funcionamento e parada do programa local conforme os tempos.
 */
inline void progRoutine() {
    if (!progActive) return;
    if (!sessaoValida(progGeracao)) {
        cancelarProgramaLocal();
        return;
    }
    const uint32_t elapsed = (millis() - progStateMs) / 1000;
    const int setpoint = currentProg == 1 ? hslider1_val : hslider2_val;
    if ((progState == PROG_WAITING && elapsed >= progCfg.startDelay)
        || (progState == PROG_STOPPED && elapsed >= progCfg.stopTime)) {
        // Lote indivisivel na fila; os broadcasts seguem separados por 500 ms.
        // Dispensa delay(100) entre duas insercoes locais.
        if (!enfileirarPartida(setpoint)) {
            ESP_LOGW("PROG", "Partida nao enfileirada: setpoint/fila/conexao indisponivel.");
            cancelarProgramaLocal();
            return;
        }
        progState = PROG_RUNNING;
        progStateMs = millis();
    } else if (progState == PROG_RUNNING && elapsed >= progCfg.runTime) {
        if (!sendCommandToMotor("STOP", progGeracao)) {
            cancelarProgramaLocal();
            return;
        }
        progState = PROG_STOPPED;
        progStateMs = millis();
    }
}

// Parsing somente na tarefa de rede, sobre uma copia privada da sessao atual.
/**
 * @brief Interpreta a telemetria chave=valor e publica a amostra da sessao atual.
 * @param dados Texto recebido do motor, com campos separados por ponto e virgula.
 */
inline void parseTelemetry(String dados) {
    const EstadoSessao sessao = lerSessao();
    if (sessao.estado != EstadoConexao::CONECTADO) return;
    static MotorData recebidos;
    static uint32_t geracaoDados = 0;
    if (geracaoDados != sessao.geracao) {
        recebidos = MotorData{};
        geracaoDados = sessao.geracao;
    }
    bool reconhecido = false;
    int start = 0;
    while (start < (int)dados.length()) {
        int sep = dados.indexOf(';', start);
        if (sep < 0) sep = dados.length();
        const String item = dados.substring(start, sep);
        const int eq = item.indexOf('=');
        if (eq > 0) {
            const String key = item.substring(0, eq);
            const String value = item.substring(eq + 1);
            if (key == "fa") { recebidos.fa = value.toFloat(); reconhecido = true; }
            else if (key == "fs") { recebidos.fs = value.toFloat(); reconhecido = true; }
            else if (key == "v") { recebidos.v = value.toFloat(); reconhecido = true; }
            else if (key == "rpm") { recebidos.rpm = value.toInt(); reconhecido = true; }
            else if (key == "tq") { recebidos.tq = value.toFloat(); reconhecido = true; }
            else if (key == "p") { recebidos.p = value.toFloat(); reconhecido = true; }
            else if (key == "vdc") { recebidos.vdc = value.toFloat(); reconhecido = true; }
            else if (key == "vs") { recebidos.vs = value.toFloat(); reconhecido = true; }
            else if (key == "i") { recebidos.i = value.toFloat(); reconhecido = true; }
            else if (key == "tm") { recebidos.tm = value.toFloat(); reconhecido = true; }
            else if (key == "a1") { recebidos.a1 = value.toFloat(); reconhecido = true; }
            else if (key == "a2") { recebidos.a2 = value.toFloat(); reconhecido = true; }
            else if (key == "a3") { recebidos.a3 = value.toFloat(); reconhecido = true; }
            else if (key == "edo") { recebidos.edo = value.toInt(); reconhecido = true; }
            else if (key == "er") { recebidos.er = value.toInt(); reconhecido = true; }
            else if (key == "tl") { recebidos.tl = value.toInt(); reconhecido = true; }
            else if (key == "tr") { recebidos.tr = value.toFloat(); reconhecido = true; }
            else if (key == "cf") { recebidos.cf = value.toInt(); reconhecido = true; }
            else if (key == "if") { recebidos.inf = value.toInt(); reconhecido = true; }
        }
        start = sep + 1;
    }
    if (!reconhecido) return; // Um cmd=... recebido nao e telemetria.
    recebidos.lastUpdateMs = millis();
    recebidos.fresh = true;
    const AmostraMotor amostra = {recebidos, sessao.geracao};
    if (filaTelemetria) xQueueOverwrite(filaTelemetria, &amostra);
}

/**
 * @brief Confirma o registro do display quando o servidor responde HI ou AGAIN.
 * @param response Resposta de registro enviada pelo servidor.
 */
inline void onSRV(const SRVResponse& response) {
    if (response.type == SRVType::HI || response.type == SRVType::AGAIN) {
        if (!quedaDetectada.load() && clientLinker.isWiFiUp() && clientLinker.isLinkUp()) {
            publicarEstado(EstadoConexao::CONECTADO);
            ESP_LOGI("LINKER", "Display registrado no servidor.");
        }
    }
}
/**
 * @brief Trata telemetria do motor ou o slider publicado por outro display do grupo.
 * @param response Mensagem recebida, com origem e conteudo.
 */
inline void onRECV(const RECVResponse& response) {
    // Os displays usam IDs 2 e 3 no mesmo grupo; nao aceitar eco proprio.
    const bool outroDisplay = response.sourceGroup == GRUPO
        && response.sourceId == (DEVICE_ID == 2 ? 3 : 2);
    const bool origemMotor = response.sourceGroup == MOTOR_SOURCE_GROUP
        && response.sourceId == MOTOR_SOURCE_ID;
    if (!origemMotor && !outroDisplay) return;
    String dados(response.data);
    dados.trim();
    if (outroDisplay) {
        if (!dados.startsWith("cmd=") || dados.length() == 4) return;
        int valor = 0;
        for (size_t i = 4; i < dados.length(); ++i) {
            if (dados[i] < '0' || dados[i] > '9') return;
            valor = valor * 10 + (dados[i] - '0');
            if (valor > 100) return; // Faixa do slider geral: 0 a 100.
        }
        portENTER_CRITICAL(&comandosMux);
        const EstadoSessao sessao = comandos.sessao();
        if (sessao.estado == EstadoConexao::CONECTADO) {
            sliderRemotoValor = valor;
            sliderRemotoGeracao = sessao.geracao;
            sliderRemotoPendente = true;
        }
        portEXIT_CRITICAL(&comandosMux);
        return; // Atualizacao visual: nao enfileirar comando nem telemetria.
    }
    if (dados.length()) parseTelemetry(dados);
}
/**
 * @brief Registra o erro do servidor e derruba a sessao nos codigos de queda.
 * @param response Erro reportado pelo servidor, com codigo e mensagem.
 */
inline void onERR(const ERRResponse& response) {
    ESP_LOGE("LINKER", "ERR %d: %s", response.code, response.message);
    if (response.code == 1 || response.code == 10) {
        sinalizarQueda();
        clientLinker.disconnectServer();
    }
}
/**
 * @brief Registra o IP e a porta de destino informados pelo servidor.
 * @param response Destino resolvido pelo servidor.
 */
inline void onDEST(const DESTResponse& response) {
    ESP_LOGI("LINKER", "DEST %s:%s", response.ip, response.port);
}
/**
 * @brief Registra um erro interno da biblioteca LinkerWX.
 * @param code Codigo do erro interno.
 * @param command Comando associado ao erro.
 */
inline void onError(ErrorCode code, const char* command) {
    ESP_LOGE("LINKER", "Erro %d em %s", (int)code, command);
}
/**
 * @brief Registra a confirmacao de recebimento enviada pelo servidor.
 */
inline void onRECVOk() { ESP_LOGI("LINKER", "RECV_OK do servidor."); }

/**
 * @brief Prepara credenciais, servidor, certificado e callbacks do cliente LinkerWX.
 * @return true quando a identificacao inicial do dispositivo foi aceita.
 */
inline bool configurarConexao() {
    if (!clientLinker.setup(DEVICE_ID, GRUPO, TOKEN)) return false;
    const uint8_t cryptKey[] = {0xFF, 0xFE, 0x84};
    clientLinker.setCryptKey(cryptKey, sizeof(cryptKey));
    clientLinker.setWiFiCredentials(ssid, password);
    clientLinker.setServer(serverHost, serverPort);
    clientLinker.setRootCA(ROOT_CA);
    clientLinker.setReconnectInterval(RECONNECT_INTERVAL_MS);
    clientLinker.registerSRVCallback(onSRV);
    clientLinker.registerRECVCallback(onRECV);
    clientLinker.registerERRCallback(onERR);
    clientLinker.registerDESTCallback(onDEST);
    clientLinker.registerErrorCallback(onError);
    clientLinker.registerRECVOkCallback(onRECVOk);
    return true;
}

/**
 * @brief Envia o proximo comando da fila, respeitando o intervalo entre broadcasts.
 */
inline void flushCommandQueue() {
    static bool enviou = false;
    static uint32_t ultimoEnvio = 0;
    if (enviou && uint32_t(millis() - ultimoEnvio) < COMMAND_SEND_INTERVAL_MS) return;
    if (!clientLinker.isWiFiUp() || !clientLinker.isLinkUp()) {
        sinalizarQueda();
        return;
    }
    CmdQueueItem item;
    portENTER_CRITICAL(&comandosMux);
    const bool retirado = comandos.retirar(item);
    portEXIT_CRITICAL(&comandosMux);
    if (!retirado || !sessaoValida(item.geracao)) return;
    char payload[40];
    snprintf(payload, sizeof(payload), "cmd=%s", item.text);
    const bool enviado = clientLinker.broadcastText(TARGET_COMMAND_GROUP, String(payload), SendType::ONLY_SEND);
    ultimoEnvio = millis();
    enviou = true;
    if (enviado) {
        ESP_LOGI("LINKER", "Broadcast grupo %u: %s", TARGET_COMMAND_GROUP, payload);
    } else {
        ESP_LOGE("LINKER", "Broadcast falhou; comando nao sera repetido: %s", payload);
        sinalizarQueda();
    }
}

/**
 * @brief Tarefa de rede: conecta, registra o display e transmite a fila de comandos.
 */
inline void tarefaLinker(void*) {
    if (esp_task_wdt_add(nullptr) != ESP_OK) {
        ESP_LOGE("WDT", "Nao foi possivel proteger a tarefa de rede.");
        publicarEstado(EstadoConexao::OFFLINE);
        vTaskDelete(nullptr);
        return;
    }
    esp_task_wdt_reset();
    if (!configurarConexao()) {
        ESP_LOGE("LINKER", "Configuracao invalida; interface continua offline.");
        publicarEstado(EstadoConexao::OFFLINE);
        esp_task_wdt_delete(nullptr);
        vTaskDelete(nullptr);
        return;
    }
    /**
     * @brief Invalida a sessao conectada quando a estacao Wi-Fi desconecta.
     */
    WiFi.onEvent([](arduino_event_id_t) {
        // Ignora eventos de uma tentativa ainda nao registrada. Uma queda
        // online invalida a fila mesmo enquanto poll() aguarda internamente.
        portENTER_CRITICAL(&comandosMux);
        if (comandos.sessao().estado == EstadoConexao::CONECTADO) {
            comandos.mudarEstado(EstadoConexao::OFFLINE);
            quedaDetectada.store(true);
        }
        portEXIT_CRITICAL(&comandosMux);
    }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.mode(WIFI_STA);
    ControleConexaoDisplay conexao(RECONNECT_INTERVAL_MS, REGISTER_TIMEOUT_MS);
    ESP_LOGI("WDT", "Tarefa de rede protegida.");
    for (;;) {
        esp_task_wdt_reset();
        if (quedaDetectada.exchange(false)) {
            conexao.desconectar(clientLinker, []() -> uint32_t { return millis(); });
            publicarEstado(EstadoConexao::OFFLINE);
        }
        if (conexao.deveTentarConectar(millis())) publicarEstado(EstadoConexao::CONECTANDO);
        conexao.atualizar(clientLinker, []() -> uint32_t { return millis(); });
        esp_task_wdt_reset();
        if (quedaDetectada.exchange(false)) {
            conexao.desconectar(clientLinker, []() -> uint32_t { return millis(); });
        }
        switch (conexao.estado()) {
            case EstadoTransporte::ONLINE:
                publicarEstado(EstadoConexao::CONECTADO);
                flushCommandQueue();
                break;
            case EstadoTransporte::CONECTANDO:
            case EstadoTransporte::AGUARDANDO_REGISTRO:
                publicarEstado(EstadoConexao::CONECTANDO);
                break;
            default:
                publicarEstado(EstadoConexao::OFFLINE);
                break;
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief Cria a fila de uma posicao usada para levar a telemetria ate a interface.
 */
inline void prepararFluxo() {
    filaTelemetria = xQueueCreate(1, sizeof(AmostraMotor));
    if (!filaTelemetria) ESP_LOGE("QUEUE", "Sem fila de telemetria; rede permanecera desativada.");
}

/**
 * @brief Configura o prazo do watchdog e inscreve o loop e a tarefa grafica.
 * @return true quando o loop e a interface ficam protegidos.
 */
inline bool inicializarWatchdog() {
    esp_task_wdt_config_t config = {};
    config.timeout_ms = TASK_WDT_TIMEOUT_MS;
    config.idle_core_mask = (1U << portNUM_PROCESSORS) - 1;
    config.trigger_panic = true;
    esp_err_t resultado = esp_task_wdt_status(nullptr);
    resultado = resultado == ESP_ERR_INVALID_STATE ? esp_task_wdt_init(&config) : esp_task_wdt_reconfigure(&config);
    if (resultado != ESP_OK) return false;
    resultado = esp_task_wdt_status(nullptr);
    if (resultado != ESP_OK) resultado = esp_task_wdt_add(nullptr);
    watchdogLoopAtivo = resultado == ESP_OK;
    if (!watchdogLoopAtivo) return false;
    const TaskHandle_t tarefaUi = myDisplay.getTaskHandle();
    if (!tarefaUi || !WidgetBase::xFilaCallback) return false;
    resultado = esp_task_wdt_status(tarefaUi);
    if (resultado != ESP_OK) resultado = esp_task_wdt_add(tarefaUi);
    watchdogUiAtivo.store(resultado == ESP_OK);
    ESP_LOGI("WDT", "Loop e interface: timeout comum %lu ms; UI=%s.",
             (unsigned long)TASK_WDT_TIMEOUT_MS, watchdogUiAtivo.load() ? "protegida" : "falha");
    return watchdogUiAtivo.load();
}

/**
 * @brief Cria a tarefa de rede depois que o watchdog e a fila de telemetria existem.
 */
inline void iniciarTarefas() {
    esp_log_level_set("LINKER", ESP_LOG_INFO);
    if (!inicializarWatchdog() || !filaTelemetria) {
        ESP_LOGE("MAIN", "Rede desativada por falha de watchdog/recursos; interface local continua.");
        return;
    }
    if (xTaskCreate(tarefaLinker, "LinkerDisplay", NETWORK_TASK_STACK_BYTES,
                    nullptr, 1, nullptr) != pdPASS) {
        ESP_LOGE("MAIN", "Falha ao criar tarefa de rede; interface local continua.");
    }
}

// Callback periodico na tarefa grafica, mesmo sem toque. Todos os widgets,
// valores de entrada e estados dos programas sao acessados nesse contexto.
/**
 * @brief Atualiza programas, slider remoto, estado da conexao e widgets com a telemetria valida.
 */
inline void servicoInterface() {
    static EstadoConexao ultimoEstado = static_cast<EstadoConexao>(255);
    static uint32_t ultimoManual = 0;
    static uint32_t geracaoAmostra = 0;
    static bool recebeuAmostra = false;
    static bool mostravaDadosValidos = true;
    const EstadoSessao sessao = lerSessao();
    if (progActive && !sessaoValida(progGeracao)) cancelarProgramaLocal();
    if (manualPendente && !sessaoValida(manualGeracao)) manualPendente = false;
    if (uint32_t(millis() - ultimoManual) >= 1000) {
        ultimoManual = millis();
        if (manualPendente && !progActive) {
            char texto[16];
            snprintf(texto, sizeof(texto), "%d", manualValor);
            // A tentativa local e consumida, mesmo se a fila estiver cheia.
            sendCommandToMotor(texto, manualGeracao);
            manualPendente = false;
        }
    }
    progRoutine();
    AmostraMotor amostra;
    if (filaTelemetria && xQueueReceive(filaTelemetria, &amostra, 0) == pdPASS
        && sessaoValida(amostra.geracao)) {
        motorData = amostra.dados;
        geracaoAmostra = amostra.geracao;
        recebeuAmostra = true;
    }
    const bool dadosValidos = recebeuAmostra && sessaoValida(geracaoAmostra)
        && uint32_t(millis() - motorData.lastUpdateMs) <= TELEMETRY_MAX_AGE_MS;

    myDisplay.startCustomDraw();
    portENTER_CRITICAL(&comandosMux);
    const bool atualizarSlider = sliderRemotoPendente
        && comandos.pertenceASessao(sliderRemotoGeracao);
    const int valorSliderRemoto = sliderRemotoValor;
    sliderRemotoPendente = false;
    portEXIT_CRITICAL(&comandosMux);
    if (atualizarSlider) {
        slidergeral.setValue(valorSliderRemoto, false);
        hslider0_val = slidergeral.getValue();
        labelhsliderge.setTextInt(hslider0_val);
    }
    if (sessao.estado != ultimoEstado) {
        labelservstat.setText(textoConexao(sessao.estado));
        led1.setColor(corConexao(sessao.estado));
        led1.setState(true); // Offline tambem fica visivel, em vermelho.
        ultimoEstado = sessao.estado;
    }
    // Valores escolhidos pelo usuario continuam atualizando sem telemetria.
    setrpmvalue.setTextInt((int)((hslider1_val / 100.0) * nb4_val));
    text8copycopy.setTextInt((int)((hslider2_val / 100.0) * nb9_val));
    if (dadosValidos && motorData.fresh) {
        gaugerpm.setValue(motorData.rpm);
        valuefact.setText(String(motorData.fa));
        valuefset.setText(String(motorData.fs));
        graphfreq.push(0, motorData.rpm);
        graphfreqcopycopy.push(0, motorData.rpm);
        graphfreqcopycopycopy.push(0, motorData.rpm);
        analogvdc.setValue((int)motorData.vdc, true);
        analogtout.setValue((int)motorData.vs, true);
        analogcur.setValue((int)motorData.i, true);
        motorData.fresh = false;
    } else if (!dadosValidos && mostravaDadosValidos) {
        valuefact.setText("--");
        valuefset.setText("--");
    }
    mostravaDadosValidos = dadosValidos;
    myDisplay.finishCustomDraw();
    if (watchdogUiAtivo.load()) esp_task_wdt_reset();
    servicoUiPendente.store(false);
}

/**
 * @brief Agenda o servico da interface no intervalo configurado e alimenta o watchdog do loop.
 */
inline void executar() {
    static uint32_t ultimoServico = 0;
    if (uint32_t(millis() - ultimoServico) >= UI_SERVICE_INTERVAL_MS
        && !servicoUiPendente.exchange(true)) {
        functionCB_t callback = servicoInterface;
        if (!WidgetBase::xFilaCallback
            || xQueueSend(WidgetBase::xFilaCallback, &callback, 0) != pdPASS) {
            servicoUiPendente.store(false); // Tentar novamente; nunca bloquear o loop.
        } else {
            ultimoServico = millis();
        }
    }
    if (watchdogLoopAtivo) esp_task_wdt_reset();
    delay(10);
}

} // namespace Fluxo
