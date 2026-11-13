/**
 * @file linkerwx_wifi_modbus_1to1.ino
 * @brief ESP32 + LinkerWX v2 + Modbus RTU, comunicação 1:1
 *
 * Fluxo geral:
 *  1. Inicia o controle local sem aguardar WiFi ou servidor TLS.
 *  2. Registra o dispositivo via protocolo LinkerWX.
 *  3. Lê telemetria do inversor de frequência via Modbus RTU.
 *  4. Mede RPM por sensor de pulso (interrupção).
 *  5. Envia telemetria ao dispositivo de destino (1:1) via LinkerWX.
 *  6. Aceita comandos via Serial ou remotamente (RECV).
 */ // [MANUAL]

// =============================================================================
// DISPLAY TM1637 — seleção entre 4 ou 6 dígitos via define
// =============================================================================

// Defina SEIS_DIGITOS para usar o display de 6 dígitos
#define SEIS_DIGITOS // [MANUAL]

#ifdef SEIS_DIGITOS // [MANUAL]
#include <TM1637TinyDisplay6.h> // [MANUAL]
TM1637TinyDisplay6 display(18, 19); ///< Display TM1637 de seis digitos; clock no pino 18 e dados no pino 19. // [MANUAL]
#else // [MANUAL]
#include <TM1637Display.h> // [MANUAL]
TM1637Display display(18, 19); ///< Display TM1637 de quatro digitos; clock no pino 18 e dados no pino 19. // [MANUAL]
#endif // [MANUAL]

// =============================================================================
// BIBLIOTECAS
// =============================================================================

#include <LinkerWX.h> // [MANUAL]
#include <LinkerWXWiFi.h> // [MANUAL]
#include <WiFi.h> // [MANUAL]
#include <HardwareSerial.h> // [MANUAL]
#include <ModbusMaster.h> // [MANUAL]
#include <esp_task_wdt.h> // [MANUAL]
#include <freertos/FreeRTOS.h> // [MANUAL]
#include <freertos/queue.h> // [MANUAL]
#include <freertos/task.h> // [MANUAL]
#include "controle_conexao.h" // [MANUAL]

// =============================================================================
// CONFIGURAÇÕES — WiFi, Servidor TLS, LinkerWX
// =============================================================================

// Credenciais da rede WiFi
const char* ssid     = "Labmaker"; ///< Nome da rede Wi-Fi usada pela placa do motor. // [MANUAL]
const char* password = "Anglo2020pp"; ///< Senha da rede Wi-Fi usada pela placa do motor. // [MANUAL]

// Servidor TLS usado nos exemplos atuais da LinkerWX
const char* serverHost = LinkerWX::SERVER_ADDRESS; ///< Endereco do servidor TLS da LinkerWX. // [MANUAL]
const uint16_t serverPort = LinkerWX::SERVER_PORT; ///< Porta do servidor TLS da LinkerWX. // [MANUAL]

// ISRG Root X1 completo, conferido pelo SHA-256 oficial:
// 96BCEC06264976F37460779ACF28C5A7CFE8A3C0AAE11A8FFCEE05C0BDDF08C6
// https://letsencrypt.org/certs/isrgrootx1.pem
const char* ROOT_CA = // [MANUAL]
    "-----BEGIN CERTIFICATE-----\n" // [MANUAL]
    "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAwTzELMAkGA1UE\n" // [MANUAL]
    "BhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNoIEdyb3VwMRUwEwYDVQQD\n" // [MANUAL]
    "EwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQG\n" // [MANUAL]
    "EwJVUzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMT\n" // [MANUAL]
    "DElTUkcgUm9vdCBYMTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54r\n" // [MANUAL]
    "Vygch77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+0TM8ukj1\n" // [MANUAL]
    "3Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6UA5/TR5d8mUgjU+g4rk8K\n" // [MANUAL]
    "b4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sWT8KOEUt+zwvo/7V3LvSye0rgTBIlDHCN\n" // [MANUAL]
    "Aymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyHB5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ\n" // [MANUAL]
    "4Q7e2RCOFvu396j3x+UCB5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf\n" // [MANUAL]
    "1b0SHzUvKBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWnOlFu\n" // [MANUAL]
    "hjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTnjh8BCNAw1FtxNrQH\n" // [MANUAL]
    "usEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbwqHyGO0aoSCqI3Haadr8faqU9GY/r\n" // [MANUAL]
    "OPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CIrU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4G\n" // [MANUAL]
    "A1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY\n" // [MANUAL]
    "9umbbjANBgkqhkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n" // [MANUAL]
    "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ3BebYhtF8GaV\n" // [MANUAL]
    "0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KKNFtY2PwByVS5uCbMiogziUwt\n" // [MANUAL]
    "hDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJw\n" // [MANUAL]
    "TdwJx4nLCgdNbOhdjsnvzqvHu7UrTkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nx\n" // [MANUAL]
    "e5AW0wdeRlN8NwdCjNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZA\n" // [MANUAL]
    "JzVcoyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq4RgqsahD\n" // [MANUAL]
    "YVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPAmRGunUHBcnWEvgJBQl9n\n" // [MANUAL]
    "JEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57demyPxgcYxn/eR44/KJ4EBs+lVDR3veyJ\n" // [MANUAL]
    "m+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n" // [MANUAL]
    "-----END CERTIFICATE-----\n"; ///< Certificado raiz ISRG Root X1, em PEM, usado na conexao TLS. // [MANUAL]

// Identificação deste dispositivo no protocolo LinkerWX
const uint16_t DEVICE_ID = 1; ///< Identificador desta placa no protocolo LinkerWX. // [MANUAL]
const uint16_t GRUPO     = 1; ///< Grupo desta placa no protocolo LinkerWX. // [MANUAL]
const String   TOKEN     = "987654321"; ///< Token de autenticacao desta placa no servidor. // [MANUAL]

// Destino para envio de telemetria (dispositivo de display remoto)
const uint16_t TARGET_DISPLAY_GROUP = 1; ///< Grupo do display que recebe a telemetria. // [MANUAL]
const uint16_t TARGET_DISPLAY_ID    = 2; ///< Identificador do display que recebe a telemetria. // [MANUAL]

// =============================================================================
// OBJETOS GLOBAIS
// =============================================================================

LinkerWXWiFi clientLinker; ///< Cliente LinkerWX usado somente pela tarefa de rede. // [MANUAL]

// =============================================================================
// PINOS DE CONTROLE DO MOTOR (saídas digitais)
// =============================================================================

const int PIN_START     = 32; ///< Saida que gera o pulso do comando de partida. // [MANUAL]
const int PIN_STOP      = 33; ///< Saida que gera o pulso do comando de parada. // [MANUAL]
const int PIN_REVERSE   = 14; ///< Saida que gera o pulso do comando de reversao. // [MANUAL]
const int PIN_EMERGENCY = 12; ///< Saida que gera o pulso do comando de emergencia. // [MANUAL]

// =============================================================================
// SENSOR DE RPM — interrupção por pulso
// =============================================================================

const int    SENSOR_RPM_PIN          = 34; ///< Entrada que recebe os pulsos do sensor de rotacao. // [MANUAL]
const double MICROS_IN_ONE_MINUTE    = 60000000.0; ///< Quantidade de microssegundos em um minuto, usada no calculo de RPM. // [MANUAL]
const uint8_t PULSOS_POR_VOLTA       = 1; ///< Quantidade de pulsos do sensor a cada volta do eixo. // [MANUAL]

// Variáveis compartilhadas entre ISR e loop principal (volatile)
volatile unsigned long lastPulseTime    = 0; ///< Instante, em microssegundos, do pulso anterior do sensor. // [MANUAL]
volatile unsigned long pulseInterval    = 0; ///< Intervalo, em microssegundos, entre os dois ultimos pulsos. // [MANUAL]
volatile unsigned long currentPulseTime = 0; ///< Instante, em microssegundos, do pulso mais recente. // [MANUAL]

uint32_t rpmAtual      = 0; ///< Rotacao calculada por ultimo, em RPM. // [MANUAL]
uint32_t lastRpmCalcMs = 0; ///< Instante, em milissegundos, do ultimo calculo de RPM. // [MANUAL]

// =============================================================================
// MODBUS RTU — configuração de hardware e temporização
// =============================================================================

const uint8_t  MODBUS_SLAVE_ID       = 1; ///< Endereco do inversor no barramento Modbus. // [MANUAL]
const int      MODBUS_RX_PIN         = 26; ///< Pino RX da UART usada no Modbus RTU. // [MANUAL]
const int      MODBUS_TX_PIN         = 27; ///< Pino TX da UART usada no Modbus RTU. // [MANUAL]
const int      MODBUS_DERE_PIN       = 25; ///< Pino DE/RE que escolhe transmissao ou recepcao no RS-485. // [MANUAL]
const uint32_t MODBUS_CYCLE_INTERVAL = 1000; ///< Intervalo entre leituras Modbus e tentativas de envio, em milissegundos. // [MANUAL]

uint32_t lastModbusCycle = 0; ///< Instante, em milissegundos, da ultima leitura Modbus. // [MANUAL]

HardwareSerial serialPort_1(1); ///< UART1 usada pelo mestre Modbus. // [MANUAL]
ModbusMaster   modbusNode; ///< Mestre Modbus RTU que fala com o inversor. // [MANUAL]

// =============================================================================
// TEMPORIZAÇÃO DA TELEMETRIA APÓS REGISTRO LinkerWX
// =============================================================================

uint32_t lastRegisterTime       = 0; ///< Instante, em milissegundos, do ultimo registro HI ou AGAIN. // [MANUAL]

// =============================================================================
// ESTRUTURA DE TELEMETRIA
// =============================================================================

/**
 * @brief Snapshot dos dados lidos do inversor + sensor de RPM.
 *
 * Campos uint32_t (4 bytes), float (4 bytes) e uint16_t (2 bytes)
 * são agrupados para minimizar padding. Total ~40 bytes.
 */ // [MANUAL]
struct TelemetryData { // [MANUAL]
    // --- inteiros 32 bits ---
    uint32_t rpm          = 0; // [MANUAL] RPM medido pelo sensor de pulso
    uint32_t lastUpdateMs = 0; // [MANUAL] timestamp da última leitura Modbus (ms)

    // --- ponto flutuante ---
    float freqAtual    = 0.0f; // [MANUAL] frequência de saída atual (Hz)
    float freqSetpoint = 0.0f; // [MANUAL] setpoint de frequência (Hz)
    float tensaoDC     = 0.0f; // [MANUAL] tensão do barramento DC (V)
    float tensaoSaida  = 0.0f; // [MANUAL] tensão de saída (V)
    float corrente     = 0.0f; // [MANUAL] corrente de saída (A)
    float torque       = 0.0f; // [MANUAL] torque estimado (%)

    // --- inteiro 16 bits ---
    uint16_t estadoDO = 0; // [MANUAL] estado das saídas digitais do inversor
}; // [MANUAL]

TelemetryData telemetryData; ///< Ultima amostra lida do inversor e do sensor de RPM. // [MANUAL]
bool telemetryDataFresh  = false; ///< Verdadeiro quando ha uma amostra nova pronta para a fila de telemetria. // [MANUAL]
bool modbusInitialized   = false; ///< Verdadeiro depois que a UART e o mestre Modbus foram configurados. // [MANUAL]

// Rede e controle local trocam COPIAS por filas, nunca String ou ponteiros.
const uint32_t RECONNECT_INTERVAL_MS = 60000UL; ///< Espera minima entre tentativas de reconexao, em milissegundos. // [MANUAL]
const uint32_t REGISTER_TIMEOUT_MS = 10000UL; ///< Prazo para receber o registro depois do TLS, em milissegundos. // [MANUAL]
const uint32_t TELEMETRY_MAX_AGE_MS = 5000UL; ///< Idade maxima da telemetria ainda enviada, em milissegundos. // [MANUAL]
// Prazo comum do TWDT: inclui WiFi, DNS, TCP e handshake TLS (ate 120 s).
// Nao reduzir sem medir os piores tempos da biblioteca instalada.
const uint32_t TASK_WDT_TIMEOUT_MS = 240000UL; ///< Prazo do watchdog de tarefas, em milissegundos. // [MANUAL]
const uint32_t NETWORK_TASK_STACK_BYTES = 12288; ///< Pilha reservada para a tarefa de rede, em bytes. // [MANUAL]
const size_t MAX_COMMAND_LENGTH = 100; ///< Tamanho maximo de um comando remoto, sem contar o terminador. // [MANUAL]
const UBaseType_t REMOTE_COMMAND_QUEUE_LENGTH = 8; ///< Quantidade de comandos remotos que a fila consegue guardar. // [MANUAL]

struct ComandoRemoto { // [MANUAL]
    char texto[MAX_COMMAND_LENGTH + 1]; // [MANUAL]
}; // [MANUAL]

QueueHandle_t filaComandosRemotos = nullptr; ///< Fila de comandos recebidos pela rede e executados no loop. // [MANUAL]
QueueHandle_t filaTelemetria = nullptr; ///< Fila de uma posicao com a ultima amostra pronta para envio. // [MANUAL]
QueueHandle_t filaEstadoRede = nullptr; ///< Fila de uma posicao com a ultima mudanca de estado da rede. // [MANUAL]
bool loopWatchdogAtivo = false; ///< Indica que o loop esta inscrito no watchdog. // [MANUAL]

// =============================================================================
// PROTÓTIPOS DE FUNÇÕES
// =============================================================================

// Controle de pinos e motor
void configurarPinosControle(); // [MANUAL]
void executarAcaoMotor(const String& comando); // [MANUAL]
void handleRemoteCommand(const String& dados); // [MANUAL]

// Telemetria
String gerarPacoteTelemetriaA(const TelemetryData& snapshot); // [MANUAL]
void sendTelemetryData(); // [MANUAL]
bool   telemetryReady(); // [MANUAL]

// Tarefas e comunicacao entre elas.
bool inicializarWatchdog(); // [MANUAL]
void iniciarTarefaRede(); // [MANUAL]
void tarefaLinker(void* parametro); // [MANUAL]
void publicarEstadoRede(EstadoRedeMotor estado); // [MANUAL]
void atenderComandoRemoto(); // [MANUAL]
void mostrarEstadoRede(); // [MANUAL]
void enviarTelemetriaNaRede(); // [MANUAL]

// Modbus
bool setupModbus(); // [MANUAL]
void preTxModbus(); // [MANUAL]
void postTxModbus(); // [MANUAL]
void pollModbusTelemetry(); // [MANUAL]
bool sendCommandToDrive(const String& comando); // [MANUAL]
bool writeRegisterWithMsg(uint16_t address, uint16_t value, // [MANUAL]
                          const char* okMsg, const char* errMsg); // [MANUAL]
void showStatusDrive(); // [MANUAL]

// Serial
void serialCommandInterpreter(); // [MANUAL]
void processSerialCommand(const String& comando); // [MANUAL]

// Sensor de RPM
void IRAM_ATTR detectPulseRPM(); // [MANUAL]
void configurarSensorRPM(); // [MANUAL]
void atualizarRPM(); // [MANUAL]
void atualizarDisplayRPM(); // [MANUAL]

// =============================================================================
// CALLBACKS LinkerWX
// =============================================================================

/**
 * @brief Chamado quando o servidor responde ao registro do dispositivo.
 *        HI = registro novo; AGAIN = já estava registrado.
 * @param response Resposta de registro enviada pelo servidor.
 */ // [MANUAL]
void onSRV(const SRVResponse& response) { // [MANUAL]
    Serial.print("[SRV] Tipo: "); // [MANUAL]

    switch (response.type) { // [MANUAL]
        case SRVType::HI: // [MANUAL]
            Serial.println("HI - Registro bem-sucedido"); // [MANUAL]
            lastRegisterTime  = millis(); // [MANUAL]
            Serial.println("✅ Dispositivo registrado com sucesso!"); // [MANUAL]
            break; // [MANUAL]

        case SRVType::AGAIN: // [MANUAL]
            Serial.println("AGAIN - Dispositivo já estava registrado"); // [MANUAL]
            lastRegisterTime  = millis(); // [MANUAL]
            break; // [MANUAL]

        case SRVType::WELCOME: // [MANUAL]
            Serial.println("WELCOME - Mensagem de boas-vindas"); // [MANUAL]
            break; // [MANUAL]

        case SRVType::UNKNOWN: // [MANUAL]
        default: // [MANUAL]
            Serial.println("UNKNOWN - Tipo desconhecido"); // [MANUAL]
            break; // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Chamado quando este dispositivo recebe uma mensagem de outro.
 *        Extrai o payload (removendo prefixo "cmd=" se presente) e
 *        enfileira para o loop executar processSerialCommand().
 * @param response Mensagem recebida, com origem e conteudo.
 */ // [MANUAL]
void onRECV(const RECVResponse& response) { // [MANUAL]
    Serial.print("[RECV] De "); // [MANUAL]
    Serial.print(response.sourceGroup); // [MANUAL]
    Serial.print(":"); // [MANUAL]
    Serial.print(response.sourceId); // [MANUAL]
    Serial.print(" - "); // [MANUAL]
    Serial.println(response.data); // [MANUAL]

    handleRemoteCommand(String(response.data)); // [MANUAL]
} // [MANUAL]

/**
 * @brief Chamado em caso de erro reportado pelo servidor LinkerWX.
 *        Código 1  = dispositivo não registrado.
 *        Código 10 = desconexão forçada pelo servidor.
 *        Código 11 = token inválido para o destino.
 * @param response Erro reportado pelo servidor, com codigo e mensagem.
 */ // [MANUAL]
void onERR(const ERRResponse& response) { // [MANUAL]
    Serial.print("[ERR] Código: "); // [MANUAL]
    Serial.print(response.code); // [MANUAL]
    Serial.print(" - "); // [MANUAL]
    Serial.println(response.message); // [MANUAL]

    if (response.code == 1) { // [MANUAL]
        clientLinker.disconnectServer(); // [MANUAL]
    } // [MANUAL]

    if (response.code == 10) { // [MANUAL]
        Serial.println("[REDE] Desconexao forcada; a tarefa agendara a reconexao."); // [MANUAL]
        // O LinkerWXWiFi ja encerrou a sessao.
    } // [MANUAL]

    if (response.code == 11) { // [MANUAL]
        Serial.println("⚠️ Token inválido para o destino."); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Registra o IP e a porta de destino informados pelo servidor.
 * @param response Destino resolvido pelo servidor.
 */ // [MANUAL]
void onDEST(const DESTResponse& response) { // [MANUAL]
    Serial.print("[DEST] IP: "); // [MANUAL]
    Serial.print(response.ip); // [MANUAL]
    Serial.print(", Porta: "); // [MANUAL]
    Serial.println(response.port); // [MANUAL]
} // [MANUAL]

/**
 * @brief Registra um erro interno da biblioteca LinkerWX.
 * @param errorCode Codigo do erro interno.
 * @param command Comando associado ao erro.
 */ // [MANUAL]
void onError(ErrorCode errorCode, const char* command) { // [MANUAL]
    Serial.print("[ERROR] Código: "); // [MANUAL]
    Serial.print((int)errorCode); // [MANUAL]
    Serial.print(" - Comando: "); // [MANUAL]
    Serial.println(command); // [MANUAL]
} // [MANUAL]

/** @brief Chamado quando o servidor confirma o recebimento de um pacote enviado. */ // [MANUAL]
void onRECVOk() { // [MANUAL]
    Serial.println("✅ [RECV_OK] Servidor confirmou recebimento do pacote!"); // [MANUAL]
} // [MANUAL]

// =============================================================================
// SENSOR DE RPM
// =============================================================================

/**
 * @brief ISR — executada a cada borda de subida no pino do sensor.
 *        Registra o intervalo entre pulsos consecutivos em microssegundos.
 *        Atributo IRAM_ATTR garante execução na RAM (mais rápido e seguro em ISR).
 */ // [MANUAL]
void IRAM_ATTR detectPulseRPM() { // [MANUAL]
    currentPulseTime = micros(); // [MANUAL]

    if (lastPulseTime > 0) { // [MANUAL]
        pulseInterval = currentPulseTime - lastPulseTime; // [MANUAL]
    } // [MANUAL]

    lastPulseTime = currentPulseTime; // [MANUAL]
} // [MANUAL]

/** @brief Configura o pino do sensor e anexa a interrupção de borda de subida. */ // [MANUAL]
void configurarSensorRPM() { // [MANUAL]
    pinMode(SENSOR_RPM_PIN, INPUT); // [MANUAL]
    attachInterrupt(digitalPinToInterrupt(SENSOR_RPM_PIN), detectPulseRPM, RISING); // [MANUAL]
    Serial.println("✅ Sensor de RPM configurado"); // [MANUAL]
} // [MANUAL]

/**
 * @brief Recalcula o RPM a partir do intervalo entre pulsos.
 *        Executado a cada 1 segundo. Zera o RPM se não houver pulso
 *        por mais de 5 segundos (timeout).
 */ // [MANUAL]
void atualizarRPM() { // [MANUAL]
    if (millis() - lastRpmCalcMs < 1000) return; // [MANUAL]
    lastRpmCalcMs = millis(); // [MANUAL]

    // Copia atômica das variáveis voláteis (desabilita interrupções brevemente)
    unsigned long intervalCopy = 0; // [MANUAL]
    unsigned long lastPulseCopy = 0; // [MANUAL]

    noInterrupts(); // [MANUAL]
    intervalCopy  = pulseInterval; // [MANUAL]
    lastPulseCopy = lastPulseTime; // [MANUAL]
    interrupts(); // [MANUAL]

    unsigned long deltaTime = micros() - lastPulseCopy; // [MANUAL]
    const unsigned long TIMEOUT_US = 5000000UL; // [MANUAL] 5 segundos em microssegundos

    if (intervalCopy > 0 && // [MANUAL]
        intervalCopy <= MICROS_IN_ONE_MINUTE && // [MANUAL]
        deltaTime <= TIMEOUT_US) { // [MANUAL]

        // RPM = (60.000.000 µs/min) / (intervalo µs/pulso) / (pulsos/volta)
        rpmAtual = (uint32_t)(MICROS_IN_ONE_MINUTE / intervalCopy / PULSOS_POR_VOLTA); // [MANUAL]

    } else { // [MANUAL]
        rpmAtual = 0; // [MANUAL]

        // Limpa o intervalo salvo se o sensor ficou silencioso por muito tempo
        if (deltaTime > TIMEOUT_US) { // [MANUAL]
            noInterrupts(); // [MANUAL]
            pulseInterval = 0; // [MANUAL]
            interrupts(); // [MANUAL]
        } // [MANUAL]
    } // [MANUAL]

    telemetryData.rpm = rpmAtual; // [MANUAL]
    atualizarDisplayRPM(); // [MANUAL]
} // [MANUAL]

/** @brief Exibe o RPM atual no display TM1637 (4 ou 6 dígitos). */ // [MANUAL]
void atualizarDisplayRPM() { // [MANUAL]
#ifdef SEIS_DIGITOS // [MANUAL]
    display.showNumber((int)rpmAtual, false); // [MANUAL]
#else // [MANUAL]
    display.showNumberDec((int)rpmAtual, false); // [MANUAL]
#endif // [MANUAL]
} // [MANUAL]

// =============================================================================
// SETUP
// =============================================================================

/**
 * @brief Inicializa saidas, sensor, display TM1637, Modbus, watchdog e a tarefa de rede.
 */ // [MANUAL]
void setup() { // [MANUAL]
    Serial.begin(115200); // [MANUAL]
    delay(1000); // [MANUAL]

    // Inicializa pinos de controle do motor e sensor de RPM
    configurarPinosControle(); // [MANUAL]
    configurarSensorRPM(); // [MANUAL]

    // Inicializa o display TM1637 e exibe zero
#ifdef SEIS_DIGITOS // [MANUAL]
    display.begin(); // [MANUAL]
    display.setBrightness(BRIGHT_HIGH); // [MANUAL]
    // Testa todos os digitos antes de iniciar WiFi/TLS e a leitura Modbus.
    Serial.println("[DISPLAY] Teste Tiny6: deve aparecer 888888 por 1 segundo (CLK=18, DIO=19)"); // [MANUAL]
    display.showNumber(888888, true); // [MANUAL]
    delay(1000); // [MANUAL]
    display.showNumber(0, false); // [MANUAL]
#else // [MANUAL]
    display.setBrightness(0x0f); // [MANUAL]
    display.showNumberDec(0, false); // [MANUAL]
#endif // [MANUAL]
    Serial.println("✅ Display TM1637 configurado"); // [MANUAL]

    Serial.println("\n========================================"); // [MANUAL]
    Serial.println("LinkerWX + Modbus RTU (1:1)"); // [MANUAL]
    Serial.println("========================================\n"); // [MANUAL]

    // Exibe resumo da topologia de comunicação
    Serial.println("\n📡 Configuração de Comunicação:"); // [MANUAL]
    Serial.print("  Dispositivo: "); // [MANUAL]
    Serial.print(GRUPO); // [MANUAL]
    Serial.print(":"); // [MANUAL]
    Serial.println(DEVICE_ID); // [MANUAL]
    Serial.print("  Destino: "); // [MANUAL]
    Serial.print(TARGET_DISPLAY_GROUP); // [MANUAL]
    Serial.print(":"); // [MANUAL]
    Serial.println(TARGET_DISPLAY_ID); // [MANUAL]
    Serial.println("  ✅ Comunicação 1:1\n"); // [MANUAL]

    // Inicializa o barramento Modbus RTU
    Serial.println("Configurando Modbus..."); // [MANUAL]
    modbusInitialized = setupModbus(); // [MANUAL]
    if (modbusInitialized) { // [MANUAL]
        Serial.println("✅ Modbus pronto"); // [MANUAL]
    } else { // [MANUAL]
        Serial.println("❌ Falha ao configurar Modbus!"); // [MANUAL]
    } // [MANUAL]

    if (inicializarWatchdog()) { // [MANUAL]
        iniciarTarefaRede(); // [MANUAL]
    } else { // [MANUAL]
        Serial.println("[WDT] Falha ao ativar protecao; rede desativada. Controle local disponivel."); // [MANUAL]
    } // [MANUAL]

    Serial.println("\nSetup completo! Serial e controle local disponiveis.\n"); // [MANUAL]
} // [MANUAL]

// =============================================================================
// WATCHDOG E TAREFA DE REDE — nenhuma operacao Modbus nesta tarefa
// =============================================================================

/**
 * @brief Configura o prazo do watchdog e inscreve o loop.
 * @return true quando o loop fica protegido.
 */ // [MANUAL]
bool inicializarWatchdog() { // [MANUAL]
    esp_task_wdt_config_t config = {}; // [MANUAL]
    config.timeout_ms = TASK_WDT_TIMEOUT_MS; // [MANUAL]
    config.idle_core_mask = 1; // [MANUAL] Supervisiona tambem a tarefa idle do core 0.
    config.trigger_panic = true; // [MANUAL]

    esp_err_t resultado = esp_task_wdt_status(nullptr); // [MANUAL]
    if (resultado == ESP_ERR_INVALID_STATE) { // [MANUAL]
        resultado = esp_task_wdt_init(&config); // [MANUAL]
    } else { // [MANUAL]
        resultado = esp_task_wdt_reconfigure(&config); // [MANUAL]
    } // [MANUAL]
    if (resultado != ESP_OK) { // [MANUAL]
        Serial.printf("[WDT] Falha na configuracao: %d\n", (int)resultado); // [MANUAL]
        return false; // [MANUAL]
    } // [MANUAL]

    resultado = esp_task_wdt_status(nullptr); // [MANUAL]
    if (resultado != ESP_OK) resultado = esp_task_wdt_add(nullptr); // [MANUAL]
    loopWatchdogAtivo = resultado == ESP_OK; // [MANUAL]
    if (loopWatchdogAtivo) { // [MANUAL]
        esp_task_wdt_reset(); // [MANUAL]
        Serial.printf("[WDT] Loop protegido; timeout comum de %lu ms.\n", // [MANUAL]
                      (unsigned long)TASK_WDT_TIMEOUT_MS); // [MANUAL]
    } else { // [MANUAL]
        Serial.printf("[WDT] Falha ao registrar loop: %d\n", (int)resultado); // [MANUAL]
    } // [MANUAL]
    return loopWatchdogAtivo; // [MANUAL]
} // [MANUAL]

/**
 * @brief Cria as filas e a tarefa que cuida do Wi-Fi e do servidor.
 */ // [MANUAL]
void iniciarTarefaRede() { // [MANUAL]
    filaComandosRemotos = xQueueCreate(REMOTE_COMMAND_QUEUE_LENGTH, sizeof(ComandoRemoto)); // [MANUAL]
    filaTelemetria = xQueueCreate(1, sizeof(TelemetryData)); // [MANUAL]
    filaEstadoRede = xQueueCreate(1, sizeof(EstadoRedeMotor)); // [MANUAL]

    BaseType_t criada = pdFAIL; // [MANUAL]
    if (filaComandosRemotos && filaTelemetria && filaEstadoRede) { // [MANUAL]
#if CONFIG_FREERTOS_UNICORE // [MANUAL]
        criada = xTaskCreate(tarefaLinker, "LinkerMotor", NETWORK_TASK_STACK_BYTES, // [MANUAL]
                            nullptr, 1, nullptr); // [MANUAL]
#else // [MANUAL]
        criada = xTaskCreatePinnedToCore(tarefaLinker, "LinkerMotor", NETWORK_TASK_STACK_BYTES, // [MANUAL]
                                        nullptr, 1, nullptr, 0); // [MANUAL]
#endif // [MANUAL]
    } // [MANUAL]

    if (criada != pdPASS) { // [MANUAL]
        // A tarefa nao existe: e seguro liberar as filas antes de entrar no loop.
        if (filaComandosRemotos) vQueueDelete(filaComandosRemotos); // [MANUAL]
        if (filaTelemetria) vQueueDelete(filaTelemetria); // [MANUAL]
        if (filaEstadoRede) vQueueDelete(filaEstadoRede); // [MANUAL]
        filaComandosRemotos = nullptr; // [MANUAL]
        filaTelemetria = nullptr; // [MANUAL]
        filaEstadoRede = nullptr; // [MANUAL]
        Serial.println("[REDE] Sem recursos para tarefa/filas. Controle local disponivel."); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Publica uma mudanca de estado da rede para o loop imprimir.
 * @param estado Estado novo da maquina de conexao.
 */ // [MANUAL]
void publicarEstadoRede(EstadoRedeMotor estado) { // [MANUAL]
    static EstadoRedeMotor anterior = EstadoRedeMotor::DESATIVADA; // [MANUAL]
    static bool publicado = false; // [MANUAL]
    if (!publicado || estado != anterior) { // [MANUAL]
        xQueueOverwrite(filaEstadoRede, &estado); // [MANUAL]
        anterior = estado; // [MANUAL]
        publicado = true; // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Tarefa de rede: configura o LinkerWX, reconecta e envia a telemetria pendente.
 * @param parametro Argumento do FreeRTOS; esta tarefa nao o utiliza.
 */ // [MANUAL]
void tarefaLinker(void* parametro) { // [MANUAL]
    (void)parametro; // [MANUAL]
    const esp_err_t resultadoWdt = esp_task_wdt_add(nullptr); // [MANUAL]
    if (resultadoWdt != ESP_OK) { // [MANUAL]
        Serial.printf("[WDT] Falha ao proteger tarefa de rede: %d\n", (int)resultadoWdt); // [MANUAL]
        publicarEstadoRede(EstadoRedeMotor::DESATIVADA); // [MANUAL]
        vTaskDelete(nullptr); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]
    Serial.println("[WDT] Tarefa de rede protegida."); // [MANUAL]
    esp_task_wdt_reset(); // [MANUAL]

    if (!clientLinker.setup(DEVICE_ID, GRUPO, TOKEN)) { // [MANUAL]
        Serial.println("[REDE] Configuracao Linker invalida. Controle local disponivel."); // [MANUAL]
        publicarEstadoRede(EstadoRedeMotor::DESATIVADA); // [MANUAL]
        esp_task_wdt_delete(nullptr); // [MANUAL]
        vTaskDelete(nullptr); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    const uint8_t cryptKey[] = {0xFF, 0xFE, 0x84}; // [MANUAL]
    clientLinker.setCryptKey(cryptKey, sizeof(cryptKey)); // [MANUAL]
    clientLinker.setWiFiCredentials(ssid, password); // [MANUAL]
    clientLinker.setServer(serverHost, serverPort); // [MANUAL]
    clientLinker.setRootCA(ROOT_CA); // [MANUAL]
    clientLinker.setReconnectInterval(RECONNECT_INTERVAL_MS); // [MANUAL]
    clientLinker.registerSRVCallback(onSRV); // [MANUAL]
    clientLinker.registerRECVCallback(onRECV); // [MANUAL]
    clientLinker.registerERRCallback(onERR); // [MANUAL]
    clientLinker.registerDESTCallback(onDEST); // [MANUAL]
    clientLinker.registerErrorCallback(onError); // [MANUAL]
    clientLinker.registerRECVOkCallback(onRECVOk); // [MANUAL]

    // LinkerWXWiFi chama disconnect() antes de begin(); inicializa o driver aqui.
    if (!WiFi.mode(WIFI_STA)) { // [MANUAL]
        Serial.println("[REDE] Falha ao inicializar WiFi; novas tentativas permanecem habilitadas."); // [MANUAL]
    } // [MANUAL]

    ControleConexaoMotor conexao(RECONNECT_INTERVAL_MS, REGISTER_TIMEOUT_MS); // [MANUAL]
    for (;;) { // [MANUAL]
        esp_task_wdt_reset(); // [MANUAL]
        if (conexao.deveTentarConectar(millis())) { // [MANUAL]
            publicarEstadoRede(EstadoRedeMotor::CONECTANDO); // [MANUAL]
        } // [MANUAL]
        conexao.atualizar(clientLinker, []() -> uint32_t { return millis(); }); // [MANUAL]
        esp_task_wdt_reset(); // [MANUAL] A chamada anterior pode ter aguardado WiFi/TLS.
        publicarEstadoRede(conexao.estado()); // [MANUAL]

        if (conexao.estado() == EstadoRedeMotor::ONLINE) { // [MANUAL]
            enviarTelemetriaNaRede(); // [MANUAL]
            esp_task_wdt_reset(); // [MANUAL]
        } // [MANUAL]
        // A espera de 1 minuto e feita pelo agendador, mantendo a tarefa viva.
        vTaskDelay(pdMS_TO_TICKS(10)); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Imprime na Serial a ultima mudanca de estado publicada pela tarefa de rede.
 */ // [MANUAL]
void mostrarEstadoRede() { // [MANUAL]
    EstadoRedeMotor estado; // [MANUAL]
    if (!filaEstadoRede || xQueueReceive(filaEstadoRede, &estado, 0) != pdPASS) return; // [MANUAL]
    switch (estado) { // [MANUAL]
        case EstadoRedeMotor::DESATIVADA: // [MANUAL]
            Serial.println("[REDE] Desativada; operacao local continua."); // [MANUAL]
            break; // [MANUAL]
        case EstadoRedeMotor::OFFLINE: // [MANUAL]
            Serial.println("[REDE] Offline; nova tentativa em 60 s. Operacao local continua."); // [MANUAL]
            break; // [MANUAL]
        case EstadoRedeMotor::CONECTANDO: // [MANUAL]
            Serial.println("[REDE] Tentando conectar em segundo plano..."); // [MANUAL]
            break; // [MANUAL]
        case EstadoRedeMotor::AGUARDANDO_REGISTRO: // [MANUAL]
            Serial.println("[REDE] TLS conectado; aguardando HI/AGAIN por ate 10 s."); // [MANUAL]
            break; // [MANUAL]
        case EstadoRedeMotor::ONLINE: // [MANUAL]
            Serial.println("[REDE] Online; comandos remotos e telemetria habilitados."); // [MANUAL]
            break; // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Executa um comando remoto pendente, no maximo um por passagem do loop.
 */ // [MANUAL]
void atenderComandoRemoto() { // [MANUAL]
    ComandoRemoto comando; // [MANUAL]
    // Um comando por passagem: nao esgotar uma fila que pode continuar recebendo.
    if (filaComandosRemotos && xQueueReceive(filaComandosRemotos, &comando, 0) == pdPASS) { // [MANUAL]
        processSerialCommand(String(comando.texto)); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

// =============================================================================
// TELEMETRIA — geração e envio 1:1
// =============================================================================

/**
 * @brief Publica a ultima amostra para a tarefa de rede, sem aguardar conexao.
 */ // [MANUAL]
void sendTelemetryData() { // [MANUAL]
    if (!filaTelemetria || !telemetryDataFresh) return; // [MANUAL]
    if (xQueueOverwrite(filaTelemetria, &telemetryData) == pdPASS) { // [MANUAL]
        telemetryDataFresh = false; // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/** @brief Somente a tarefa de rede acessa o Linker e esta amostra privada. */ // [MANUAL]
void enviarTelemetriaNaRede() { // [MANUAL]
    static uint32_t sendCount = 0; // [MANUAL]
    static TelemetryData snapshot; // [MANUAL]
    static bool pendente = false; // [MANUAL]
    static bool envioTentado = false; // [MANUAL]
    static uint32_t ultimoEnvioTentado = 0; // [MANUAL]

    if (xQueueReceive(filaTelemetria, &snapshot, 0) == pdPASS) pendente = true; // [MANUAL]
    const uint32_t agora = millis(); // [MANUAL]
    if (pendente && uint32_t(agora - snapshot.lastUpdateMs) > TELEMETRY_MAX_AGE_MS) { // [MANUAL]
        pendente = false; // [MANUAL]
    } // [MANUAL]

    if (!clientLinker.isLinkUp()) return; // [MANUAL]
    if (!pendente || uint32_t(agora - lastRegisterTime) <= 300) return; // [MANUAL]
    if (envioTentado && uint32_t(agora - ultimoEnvioTentado) < MODBUS_CYCLE_INTERVAL) return; // [MANUAL]

    envioTentado = true; // [MANUAL]
    ultimoEnvioTentado = agora; // [MANUAL]
    String dadosA = gerarPacoteTelemetriaA(snapshot); // [MANUAL]
    if (clientLinker.broadcastText(TARGET_DISPLAY_GROUP, // [MANUAL]
                      dadosA, SendType::REQ_ACK)) { // [MANUAL]
        pendente = false; // [MANUAL]
        sendCount++; // [MANUAL]
        Serial.printf("[SEND #%lu] %u bytes payload enviados\n", // [MANUAL]
                      (unsigned long)sendCount, (unsigned)dadosA.length()); // [MANUAL]
    } else { // [MANUAL]
        Serial.println("Falha ao enviar telemetria; os dados continuam pendentes"); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Serializa os dados de telemetryData em uma string chave=valor
 *        separada por ponto-e-vírgula, pronta para envio via LinkerWX.
 *
 * Formato: "fa=X.X;fs=X.X;vdc=X.X;vs=X.X;i=X.X;tq=X.X;edo=N;rpm=N"
 * @param snapshot Copia da telemetria que sera serializada.
 * @return Texto chave=valor, separado por ponto e virgula, pronto para o LinkerWX.
 */ // [MANUAL]
String gerarPacoteTelemetriaA(const TelemetryData& snapshot) { // [MANUAL]
    char temp[24]; // [MANUAL]
    String payload; // [MANUAL]
    payload.reserve(80); // [MANUAL]

    snprintf(temp, sizeof(temp), "fa=%.1f",   snapshot.freqAtual);    payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";fs=%.1f",  snapshot.freqSetpoint); payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";vdc=%.1f", snapshot.tensaoDC);     payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";vs=%.1f",  snapshot.tensaoSaida);  payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";i=%.1f",   snapshot.corrente);     payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";tq=%.1f",  snapshot.torque);       payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";edo=%u",   snapshot.estadoDO);     payload += temp; // [MANUAL]
    snprintf(temp, sizeof(temp), ";rpm=%lu",  snapshot.rpm);          payload += temp; // [MANUAL]

    return payload; // [MANUAL]
} // [MANUAL]

/**
 * @brief Verifica se há telemetria válida e recente disponível.
 * @return true se Modbus inicializado, dados frescos e atualizados há menos de 5s.
 */ // [MANUAL]
bool telemetryReady() { // [MANUAL]
    if (!modbusInitialized || !telemetryDataFresh) return false; // [MANUAL]
    return (millis() - telemetryData.lastUpdateMs) <= 5000; // [MANUAL]
} // [MANUAL]

// =============================================================================
// COMANDOS REMOTOS E CONTROLE DO MOTOR
// =============================================================================

/**
 * @brief Ponto de entrada para comandos recebidos remotamente via LinkerWX.
 *        Remove prefixo "cmd=" e copia para a fila. Modbus fica no loop.
 * @param dados Payload recebido, com ou sem o prefixo cmd=.
 */ // [MANUAL]
void handleRemoteCommand(const String& dados) { // [MANUAL]
    String comando = dados; // [MANUAL]
    comando.trim(); // [MANUAL]

    if (comando.startsWith("cmd=") || comando.startsWith("CMD=")) { // [MANUAL]
        comando = comando.substring(4); // [MANUAL]
        comando.trim(); // [MANUAL]
    } // [MANUAL]

    if (comando.length() == 0) return; // [MANUAL]

    if (comando.length() > MAX_COMMAND_LENGTH) { // [MANUAL]
        Serial.println("[RECV] Comando descartado: excedeu 100 caracteres."); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]
    ComandoRemoto mensagem = {}; // [MANUAL]
    comando.toCharArray(mensagem.texto, sizeof(mensagem.texto)); // [MANUAL]
    if (!filaComandosRemotos || xQueueSend(filaComandosRemotos, &mensagem, 0) != pdPASS) { // [MANUAL]
        Serial.println("[RECV] Comando descartado: fila indisponivel ou cheia."); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Aciona fisicamente um pino de controle do motor por 150 ms.
 *        Comandos válidos: START, STOP, REVERSE, EMERGENCY.
 *        O pulso simula o pressionamento de um botão no painel físico.
 * @param comando Acao fisica: START, STOP, REVERSE ou EMERGENCY.
 */ // [MANUAL]
void executarAcaoMotor(const String& comando) { // [MANUAL]
    int pinAtivo = -1; // [MANUAL]

    if      (comando == "START")     pinAtivo = PIN_START; // [MANUAL]
    else if (comando == "STOP")      pinAtivo = PIN_STOP; // [MANUAL]
    else if (comando == "REVERSE")   pinAtivo = PIN_REVERSE; // [MANUAL]
    else if (comando == "EMERGENCY") pinAtivo = PIN_EMERGENCY; // [MANUAL]

    if (pinAtivo == -1) { // [MANUAL]
        Serial.println("[CMD] Comando desconhecido."); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    // Pulso de 150 ms para simular acionamento de botão
    digitalWrite(pinAtivo, HIGH); // [MANUAL]
    delay(150); // [MANUAL]
    digitalWrite(pinAtivo, LOW); // [MANUAL]
    Serial.println("[CMD] Saída acionada."); // [MANUAL]
} // [MANUAL]

/** @brief Configura os pinos de controle do motor como saída e os inicializa em LOW. */ // [MANUAL]
void configurarPinosControle() { // [MANUAL]
    pinMode(PIN_START,     OUTPUT); // [MANUAL]
    pinMode(PIN_STOP,      OUTPUT); // [MANUAL]
    pinMode(PIN_REVERSE,   OUTPUT); // [MANUAL]
    pinMode(PIN_EMERGENCY, OUTPUT); // [MANUAL]

    digitalWrite(PIN_START,     LOW); // [MANUAL]
    digitalWrite(PIN_STOP,      LOW); // [MANUAL]
    digitalWrite(PIN_REVERSE,   LOW); // [MANUAL]
    digitalWrite(PIN_EMERGENCY, LOW); // [MANUAL]
} // [MANUAL]

// =============================================================================
// MODBUS RTU — inicialização, polling e envio de comandos
// =============================================================================

/**
 * @brief Inicializa a UART1 e o mestre Modbus RTU.
 *        Configura o pino DE/RE do transceiver RS-485 e registra os
 *        callbacks de pré/pós transmissão para controle de direção do barramento.
 * @return true (sempre, pois não há verificação de hardware neste ponto).
 */ // [MANUAL]
bool setupModbus() { // [MANUAL]
    pinMode(MODBUS_DERE_PIN, OUTPUT); // [MANUAL]
    digitalWrite(MODBUS_DERE_PIN, LOW); // [MANUAL] inicia em modo recepção

    serialPort_1.begin(9600, SERIAL_8N1, MODBUS_RX_PIN, MODBUS_TX_PIN); // [MANUAL]
    modbusNode.begin(MODBUS_SLAVE_ID, serialPort_1); // [MANUAL]
    modbusNode.preTransmission(preTxModbus); // [MANUAL]
    modbusNode.postTransmission(postTxModbus); // [MANUAL]

    telemetryData = TelemetryData{}; // [MANUAL] zera a estrutura de telemetria
    return true; // [MANUAL]
} // [MANUAL]

/** @brief Coloca o transceiver RS-485 em modo transmissão antes de enviar. */ // [MANUAL]
void preTxModbus() { // [MANUAL]
    digitalWrite(MODBUS_DERE_PIN, HIGH); // [MANUAL]
    delayMicroseconds(50); // [MANUAL] aguarda estabilização do barramento
} // [MANUAL]

/** @brief Retorna o transceiver RS-485 para modo recepção após transmitir. */ // [MANUAL]
void postTxModbus() { // [MANUAL]
    serialPort_1.flush(); // [MANUAL] garante que todos os bytes foram enviados
    delayMicroseconds(500); // [MANUAL] aguarda o último bit sair do buffer
    digitalWrite(MODBUS_DERE_PIN, LOW); // [MANUAL]
} // [MANUAL]

/**
 * @brief Lê 9 registradores a partir de 0x7000 (telemetria do inversor).
 *        Executado a cada MODBUS_CYCLE_INTERVAL ms.
 *        Em caso de sucesso, atualiza telemetryData e dispara sendTelemetryData().
 *
 * Mapa de registradores (base 0x7000):
 *   +0 freqAtual    (×0.01 Hz)
 *   +1 freqSetpoint (×0.01 Hz)
 *   +2 tensaoDC     (×0.1 V)
 *   +3 tensaoSaida  (×1 V)
 *   +4 corrente     (×0.1 A)
 *   +5 torque       (×0.1 %)
 *   +8 estadoDO     (bitmask saídas digitais)
 */ // [MANUAL]
void pollModbusTelemetry() { // [MANUAL]
    if (millis() - lastModbusCycle < MODBUS_CYCLE_INTERVAL) return; // [MANUAL]
    lastModbusCycle = millis(); // [MANUAL]

    uint32_t t0 = millis(); // [MANUAL] marca tempo para medir duração da leitura

    uint8_t result = modbusNode.readHoldingRegisters(0x7000, 9); // [MANUAL]

    if (result == modbusNode.ku8MBSuccess) { // [MANUAL]
        // Converte os valores brutos aplicando os fatores de escala do inversor
        telemetryData.freqAtual    = modbusNode.getResponseBuffer(0) * 0.01f; // [MANUAL]
        telemetryData.freqSetpoint = modbusNode.getResponseBuffer(1) * 0.01f; // [MANUAL]
        telemetryData.tensaoDC     = modbusNode.getResponseBuffer(2) * 0.1f; // [MANUAL]
        telemetryData.tensaoSaida  = modbusNode.getResponseBuffer(3) * 1.0f; // [MANUAL]
        telemetryData.corrente     = modbusNode.getResponseBuffer(4) * 0.1f; // [MANUAL]
        telemetryData.torque       = modbusNode.getResponseBuffer(5) * 0.1f; // [MANUAL]
        telemetryData.estadoDO     = modbusNode.getResponseBuffer(8); // [MANUAL]

        telemetryData.rpm          = rpmAtual; // [MANUAL] captura RPM junto com a leitura Modbus
        telemetryData.lastUpdateMs = millis(); // [MANUAL]
        telemetryDataFresh         = true; // [MANUAL]

        Serial.printf("✅ Modbus OK em %lums\n", millis() - t0); // [MANUAL]

        // Disponibiliza a amostra sem esperar pela rede.
        sendTelemetryData(); // [MANUAL]

    } else { // [MANUAL]
        Serial.printf("❌ Modbus erro=0x%02X\n", result); // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Traduz um comando de texto em escrita Modbus no inversor.
 *
 * Comandos suportados:
 *   START     → 0x2000 = 1  (forward)
 *   REVERSE   → 0x2000 = 2  (reverse)
 *   STOP      → 0x2000 = 6  (soft stop)
 *   EMERGENCY → 0x2000 = 5  (parada de emergência)
 *   SET2000N  → 0x2000 = N  (valor arbitrário)
 *   STATUS    → leitura de diagnóstico (sem escrita)
 *   <número>  → 0x1000 = valor×100 (setpoint de frequência em Hz)
 *
 * @return true se o comando foi executado com sucesso.
 * @param comando Comando de texto traduzido para uma escrita Modbus.
 */ // [MANUAL]
bool sendCommandToDrive(const String& comando) { // [MANUAL]
    if (!modbusInitialized) { // [MANUAL]
        Serial.println("Modbus não inicializado. Comando ignorado."); // [MANUAL]
        return false; // [MANUAL]
    } // [MANUAL]

    String trimmed = comando; // [MANUAL]
    trimmed.trim(); // [MANUAL]
    if (trimmed.length() == 0) return false; // [MANUAL]

    String normalized = trimmed; // [MANUAL]
    normalized.toUpperCase(); // [MANUAL]

    if (normalized == "START") { // [MANUAL]
        return writeRegisterWithMsg(0x2000, 1, "Motor iniciado (forward)", "Falha ao iniciar motor"); // [MANUAL]
    } else if (normalized == "REVERSE") { // [MANUAL]
        return writeRegisterWithMsg(0x2000, 2, "Motor iniciado (reverse)", "Falha ao reverter motor"); // [MANUAL]
    } else if (normalized == "STOP") { // [MANUAL]
        return writeRegisterWithMsg(0x2000, 6, "Motor parado (soft)", "Falha ao parar motor"); // [MANUAL]
    } else if (normalized == "EMERGENCY") { // [MANUAL]
        return writeRegisterWithMsg(0x2000, 5, "Parada de emergência acionada", "Falha na emergência"); // [MANUAL]
    } else if (normalized.startsWith("SET2000")) { // [MANUAL]
        int val = trimmed.substring(7).toInt(); // [MANUAL]
        return writeRegisterWithMsg(0x2000, val, "Valor escrito em 0x2000", "Falha no SET2000"); // [MANUAL]
    } else if (normalized == "STATUS") { // [MANUAL]
        showStatusDrive(); // [MANUAL]
        return true; // [MANUAL]
    } else { // [MANUAL]
        // Tenta interpretar como frequência numérica (ex: "45.5")
        float freq = trimmed.toFloat(); // [MANUAL]
        if (freq > 0.0f) { // [MANUAL]
            uint16_t freqRaw = static_cast<uint16_t>(freq * 100.0f); // [MANUAL]
            return writeRegisterWithMsg(0x1000, freqRaw, // [MANUAL]
                                        "Setpoint de frequência atualizado", // [MANUAL]
                                        "Falha ao escrever frequência"); // [MANUAL]
        } // [MANUAL]
    } // [MANUAL]

    return false; // [MANUAL]
} // [MANUAL]

/**
 * @brief Escreve um único registrador Modbus e imprime mensagem de resultado.
 * @param address  Endereço do registrador.
 * @param value    Valor a escrever.
 * @param okMsg    Mensagem exibida em caso de sucesso.
 * @param errMsg   Mensagem exibida em caso de falha (inclui código de erro).
 * @return true se a escrita foi bem-sucedida.
 */ // [MANUAL]
bool writeRegisterWithMsg(uint16_t address, uint16_t value, // [MANUAL]
                          const char* okMsg, const char* errMsg) { // [MANUAL]
    uint8_t result = modbusNode.writeSingleRegister(address, value); // [MANUAL]
    if (result == modbusNode.ku8MBSuccess) { // [MANUAL]
        Serial.printf("✓ %s\n", okMsg); // [MANUAL]
        return true; // [MANUAL]
    } // [MANUAL]

    Serial.printf("✗ %s (Code: %d)\n", errMsg, result); // [MANUAL]
    return false; // [MANUAL]
} // [MANUAL]

/**
 * @brief Lê e exibe registradores de diagnóstico do inversor via Serial.
 *        Registradores lidos: Run State (0x703D), Fault Code (0x703E),
 *        frequência atual (0x7000), corrente de saída (0x7004) e RPM do sensor.
 */ // [MANUAL]
void showStatusDrive() { // [MANUAL]
    Serial.println("\n=== ESTADO DO INVERSOR ==="); // [MANUAL]

    if (modbusNode.readHoldingRegisters(0x703D, 1) == modbusNode.ku8MBSuccess) { // [MANUAL]
        uint16_t state = modbusNode.getResponseBuffer(0); // [MANUAL]
        Serial.printf("Run State (0x703D): %d (BIN: %s)\n", state, String(state, BIN).c_str()); // [MANUAL]
    } else { // [MANUAL]
        Serial.println("Falha ao ler Run State."); // [MANUAL]
    } // [MANUAL]

    if (modbusNode.readHoldingRegisters(0x703E, 1) == modbusNode.ku8MBSuccess) { // [MANUAL]
        Serial.printf("Fault Code (0x703E): %d\n", modbusNode.getResponseBuffer(0)); // [MANUAL]
    } else { // [MANUAL]
        Serial.println("Falha ao ler Fault Code."); // [MANUAL]
    } // [MANUAL]

    if (modbusNode.readHoldingRegisters(0x7000, 1) == modbusNode.ku8MBSuccess) { // [MANUAL]
        Serial.printf("Current Frequency: %.2f Hz\n", modbusNode.getResponseBuffer(0) / 100.0f); // [MANUAL]
    } // [MANUAL]

    if (modbusNode.readHoldingRegisters(0x7004, 1) == modbusNode.ku8MBSuccess) { // [MANUAL]
        Serial.printf("Output Current: %.2f A\n", modbusNode.getResponseBuffer(0) / 10.0f); // [MANUAL]
    } // [MANUAL]

    Serial.printf("RPM Sensor: %lu\n", telemetryData.rpm); // [MANUAL]

    Serial.println("==========================\n"); // [MANUAL]
} // [MANUAL]

// =============================================================================
// LOOP PRINCIPAL
// =============================================================================

/**
 * @brief Loop principal — executado continuamente pelo ESP32.
 *
 * Ordem de execução:
 *  1. Lê comandos da Serial.
 *  2. Executa um comando remoto da fila e mostra mudancas do estado da rede.
 *  3. Atualiza cálculo de RPM.
 *  4. Faz polling Modbus (que internamente chama sendTelemetryData).
 */ // [MANUAL]
void loop() { // [MANUAL]
    serialCommandInterpreter(); // [MANUAL]

    atenderComandoRemoto(); // [MANUAL]
    mostrarEstadoRede(); // [MANUAL]

    atualizarRPM(); // [MANUAL]

    if (modbusInitialized) pollModbusTelemetry(); // [MANUAL] já chama sendTelemetryData internamente
    if (loopWatchdogAtivo) esp_task_wdt_reset(); // [MANUAL]
    delay(1); // [MANUAL]
} // [MANUAL]

// =============================================================================
// INTERPRETADOR DE COMANDOS SERIAL
// =============================================================================

/**
 * @brief Lê caracteres da Serial e monta linhas de comando.
 *        Descarta '\r' e processa a linha ao receber '\n'.
 *        Buffer limitado a 100 caracteres para evitar overflow.
 */ // [MANUAL]
void serialCommandInterpreter() { // [MANUAL]
    static String inputLine = ""; // [MANUAL]
    size_t bytesLidos = 0; // [MANUAL]

    // Limita trabalho por passagem, inclusive se a Serial receber continuamente.
    while (Serial.available() > 0 && bytesLidos < 64) { // [MANUAL]
        char c = (char)Serial.read(); // [MANUAL]
        bytesLidos++; // [MANUAL]

        if (c == '\r') continue; // [MANUAL] ignora retorno de carro

        if (c == '\n') { // [MANUAL]
            inputLine.trim(); // [MANUAL]

            if (inputLine.length() > 0) { // [MANUAL]
                processSerialCommand(inputLine); // [MANUAL]
            } // [MANUAL]

            inputLine = ""; // [MANUAL]
            return; // [MANUAL] No maximo uma linha por passagem do loop.
        } // [MANUAL]

        inputLine += c; // [MANUAL]

        // Proteção contra linhas excessivamente longas
        if (inputLine.length() > 100) { // [MANUAL]
            inputLine = ""; // [MANUAL]
            Serial.println("⚠️ Linha muito longa. Buffer descartado."); // [MANUAL]
        } // [MANUAL]
    } // [MANUAL]
} // [MANUAL]

/**
 * @brief Processa um comando de texto (Serial ou remoto) e executa a ação correspondente.
 *
 * Fluxo de decisão:
 *  1. HELP       → exibe lista de comandos.
 *  2. SET2000=N  → escreve N diretamente em 0x2000.
 *  3. START / STOP / REVERSE / EMERGENCY / STATUS
 *                → envia via Modbus e aciona pino físico (exceto STATUS).
 *  4. Número     → interpreta como frequência (Hz) e escreve em 0x1000.
 *  5. Desconhecido → avisa e sugere HELP.
 * @param comandoRecebido Texto vindo da Serial ou da fila de comandos remotos.
 */ // [MANUAL]
void processSerialCommand(const String& comandoRecebido) { // [MANUAL]
    String comando = comandoRecebido; // [MANUAL]
    comando.trim(); // [MANUAL]

    if (comando.length() == 0) return; // [MANUAL]

    Serial.print("[SERIAL CMD] Recebido: "); // [MANUAL]
    Serial.println(comando); // [MANUAL]

    String comandoUpper = comando; // [MANUAL]
    comandoUpper.toUpperCase(); // [MANUAL]

    if (comandoUpper == "RPMSTATUS") { // [MANUAL]
        unsigned long intervalCopy; // [MANUAL]
        unsigned long lastPulseCopy; // [MANUAL]
        noInterrupts(); // [MANUAL]
        intervalCopy = pulseInterval; // [MANUAL]
        lastPulseCopy = lastPulseTime; // [MANUAL]
        interrupts(); // [MANUAL]
        Serial.printf("[RPM] rpm=%lu GPIO%d=%d intervalo_us=%lu ultimo_pulso_us=%lu agora_us=%lu\n", // [MANUAL]
                      (unsigned long)rpmAtual, SENSOR_RPM_PIN, digitalRead(SENSOR_RPM_PIN), // [MANUAL]
                      intervalCopy, lastPulseCopy, micros()); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    if (comandoUpper == "HELP") { // [MANUAL]
        Serial.println("\n=== COMANDOS DISPONIVEIS ==="); // [MANUAL]
        Serial.println("START        -> inicia motor"); // [MANUAL]
        Serial.println("STOP         -> para motor"); // [MANUAL]
        Serial.println("REVERSE      -> reverso"); // [MANUAL]
        Serial.println("EMERGENCY    -> emergencia"); // [MANUAL]
        Serial.println("STATUS       -> le status do inversor + rpm"); // [MANUAL]
        Serial.println("RPMSTATUS    -> diagnostico do sensor de RPM no GPIO34"); // [MANUAL]
        Serial.println("SET2000=VAL  -> escreve VAL no registrador 0x2000"); // [MANUAL]
        Serial.println("30 ou 45.5   -> escreve frequencia em 0x1000"); // [MANUAL]
        Serial.println("============================\n"); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    if (!modbusInitialized) { // [MANUAL]
        Serial.println("❌ Modbus nao inicializado."); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    // Comando SET2000=N — extrai o valor após '=' e escreve em 0x2000
    if (comandoUpper.startsWith("SET2000=")) { // [MANUAL]
        String valorStr = comando.substring(comando.indexOf('=') + 1); // [MANUAL]
        valorStr.trim(); // [MANUAL]
        int valor = valorStr.toInt(); // [MANUAL]

        bool ok = writeRegisterWithMsg(0x2000, (uint16_t)valor, // [MANUAL]
                                       "Valor escrito em 0x2000", // [MANUAL]
                                       "Falha ao escrever em 0x2000"); // [MANUAL]
        if (!ok) Serial.println("❌ Comando SET2000 falhou."); // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    // Comandos de controle do motor — envia via Modbus e aciona pino físico
    if (comandoUpper == "START"     || comandoUpper == "STOP" || // [MANUAL]
        comandoUpper == "REVERSE"   || comandoUpper == "EMERGENCY" || // [MANUAL]
        comandoUpper == "STATUS") { // [MANUAL]

        bool ok = sendCommandToDrive(comandoUpper); // [MANUAL]
        if (!ok) { // [MANUAL]
            Serial.println("❌ Falha ao executar comando."); // [MANUAL]
            return; // [MANUAL]
        } // [MANUAL]

        // STATUS não aciona pino físico, apenas lê registradores
        if (comandoUpper != "STATUS") { // [MANUAL]
            executarAcaoMotor(comandoUpper); // [MANUAL]
        } // [MANUAL]
        return; // [MANUAL]
    } // [MANUAL]

    // Verifica se o comando contém dígitos para tentar interpretar como frequência
    bool temDigito = false; // [MANUAL]
    for (size_t i = 0; i < comando.length(); i++) { // [MANUAL]
        if (isDigit(comando[i])) { temDigito = true; break; } // [MANUAL]
    } // [MANUAL]

    if (temDigito) { // [MANUAL]
        float freq = comando.toFloat(); // [MANUAL]
        if (freq > 0.0f) { // [MANUAL]
            uint16_t freqRaw = (uint16_t)(freq * 100.0f); // [MANUAL]
            bool ok = writeRegisterWithMsg(0x1000, freqRaw, // [MANUAL]
                                           "Setpoint de frequencia atualizado", // [MANUAL]
                                           "Falha ao escrever frequencia"); // [MANUAL]
            if (!ok) Serial.println("❌ Falha ao escrever frequencia."); // [MANUAL]
            return; // [MANUAL]
        } // [MANUAL]
    } // [MANUAL]

    Serial.println("❌ Comando desconhecido. Digite HELP."); // [MANUAL]
} // [MANUAL]
