// Include for plugins of chip 2
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LinkerWX.h>
#include <string.h>
#include <stdlib.h>

// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 800;
const int DISPLAY_H = 1280;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;

#pragma pack(push, 1)
struct WeatherData_t {
    float temperature;
    float humidity;
    float pressure;
    float windSpeed;
    float windDirection;
    float windGust;
    float windGustDirection;
};
#pragma pack(pop)

#include "esp_system.h"

String getResetReasonString() {
  esp_reset_reason_t reason = esp_reset_reason();
  switch(reason) {
    case ESP_RST_UNKNOWN: return "Desconhecido";
    case ESP_RST_POWERON: return "Reinicio por energia ligada";
    case ESP_RST_EXT: return "Reinicio por fonte externa";
    case ESP_RST_SW: return "Reinicio por software";
    case ESP_RST_PANIC: return "Reinicio por erro de panic (watchdog, etc)";
    case ESP_RST_INT_WDT: return "Reinicio por watchdog interno";
    case ESP_RST_TASK_WDT: return "Reinicio por watchdog de tarefa";
    case ESP_RST_WDT: return "Reinicio por watchdog";
    case ESP_RST_DEEPSLEEP: return "Reinicio por saída do modo de deep sleep";
    case ESP_RST_BROWNOUT: return "Reinicio por brownout (queda de voltagem)";
    case ESP_RST_SDIO: return "Reinicio por SDIO";
    default: return "Motivo desconhecido";
  }
}

const char* ssid     = "Labmaker";
const char* password = "Anglo2020pp";
const int intervalCheckWifi = 30000;
int timeCheckWifi = 0;

constexpr const char* server = LinkerWX::SERVER_LINKERWX_IP;
const int port = 12345;
const int intervalCheckServer = 30000;
int timeCheckServer = 0;

// ============================================
// CONFIGURAÇÕES LINKERWX
// ============================================
const String DEVICE_ID = "esp_p4";        ///< ID único do dispositivo
const String GRUPO = "labmaker";          ///< Grupo do dispositivo
const String TOKEN = "987654321";         ///< Token de autenticação
const String DESTINO_GRUPO = "labmaker";  ///< Grupo de destino
const String DESTINO_ID = "esp_s3";       ///< ID do dispositivo de destino
const uint8_t STRUCT_ID_WEATHER = 0x02;   ///< ID da estrutura WeatherData_t

// ============================================
// OBJETOS
// ============================================
WiFiClient socketClient;                  ///< Cliente WiFi para comunicação TCP
LinkerWX linker;                         ///< Instância da classe LinkerWX

// ============================================
// VARIÁVEIS DE CONTROLE
// ============================================
bool deviceRegistered = false;           ///< Flag indicando se dispositivo está registrado
uint32_t lastReconnectCheck = 0;         ///< Último tempo de verificação de reconexão
const uint32_t RECONNECT_CHECK_INTERVAL = 30000; ///< Intervalo para verificar reconexão (30s)
uint32_t lastRegisterTime = 0;           ///< Último tempo de registro (HI ou AGAIN)
bool deveEnviarDado = false;              ///< Flag para enviar dados de texto
bool deveEnviarPacote = false;           ///< Flag para enviar pacote binário


// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
void btntexto_cb();
void btnstruct1_cb();
#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
Label txtlog(100, 170, 0);
Label txttemp(90, 440, 0);
Label txtumidade(100, 520, 0);
const uint8_t qtdLabel = 3;
Label *arrayLabel[qtdLabel] = {&txtlog, &txttemp, &txtumidade};
Led ledwifi(595, 160, 0);
Led ledlinker(595, 205, 0);
const uint8_t qtdLed = 2;
Led *arrayLed[qtdLed] = {&ledwifi, &ledlinker};
TextButton btntexto(525, 385, 0);
TextButton btnstruct1(525, 520, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btntexto, &btnstruct1};

// ============================================
// FUNÇÕES DE SERIALIZAÇÃO/DESERIALIZAÇÃO
// ============================================

/**
 * @brief Serializa um WeatherData_t para buffer binário
 * @param weatherData Ponteiro para os dados meteorológicos
 * @param buffer Buffer de saída
 * @param bufferSize Tamanho do buffer
 * @return Tamanho dos dados serializados (0 se erro)
 */
size_t serializarWeatherData(const WeatherData_t* weatherData, uint8_t* buffer, size_t bufferSize) {
    if (weatherData == nullptr || buffer == nullptr) {
        return 0;
    }
    
    size_t requiredSize = sizeof(WeatherData_t);
    if (bufferSize < requiredSize) {
        return 0;
    }
    
    memcpy(buffer, weatherData, sizeof(WeatherData_t));
    return sizeof(WeatherData_t);
}

/**
 * @brief Deserializa um buffer binário para WeatherData_t
 * @param buffer Buffer com dados serializados
 * @param bufferSize Tamanho do buffer
 * @param weatherData Ponteiro para os dados meteorológicos a preencher
 * @return true se deserializou com sucesso
 */
bool deserializarWeatherData(const uint8_t* buffer, size_t bufferSize, WeatherData_t* weatherData) {
    if (buffer == nullptr || weatherData == nullptr || bufferSize < sizeof(WeatherData_t)) {
        return false;
    }
    
    memcpy(weatherData, buffer, sizeof(WeatherData_t));
    return true;
}

// ============================================
// CALLBACKS LINKERWX
// ============================================

/**
 * @brief Callback para mensagens SRV do servidor
 * @param response Estrutura com dados da resposta SRV
 */
void onSRV(const SRVResponse& response) {
    Serial.print("[SRV] Tipo: ");
    
    switch (response.type) {
        case SRVType::HI:
            Serial.print("HI");
            Serial.print(" - Grupo: ");
            Serial.print(response.group);
            Serial.print(", ID: ");
            Serial.println(response.id);
            
            // Verifica se é confirmação do nosso registro
            if (response.group == GRUPO && response.id == DEVICE_ID) {
                deviceRegistered = true;
                lastRegisterTime = millis();
                Serial.println("✅ Dispositivo registrado com sucesso!");
                ledlinker.setState(true);
            }
            break;
            
        case SRVType::AGAIN:
            Serial.println("AGAIN - Dispositivo já estava registrado");
            deviceRegistered = true;
            lastRegisterTime = millis();
            ledlinker.setState(true);
            break;
            
        case SRVType::WELCOME:
            Serial.println("WELCOME - Mensagem de boas-vindas");
            break;
            
        case SRVType::UNKNOWN:
        default:
            Serial.println("UNKNOWN - Tipo desconhecido");
            break;
    }
}

/**
 * @brief Callback para mensagens RECV (dados recebidos - texto)
 * @param response Estrutura com dados recebidos
 */
void onRECV(const RECVResponse& response) {
    Serial.print("[RECV] De ");
    Serial.print(response.sourceGroup);
    Serial.print(":");
    Serial.print(response.sourceId);
    Serial.print(" - ");
    Serial.println(response.data);
    
    // Mostrar texto recebido em txtlog
    txtlog.setText(response.data);
}

/**
 * @brief Callback para mensagens RECV_BINARY (dados binários recebidos)
 * @param response Estrutura com dados binários recebidos
 */
void onRECVBinary(const RECVBinaryResponse& response) {
    Serial.print("[RECV_BINARY] De ");
    Serial.print(response.sourceGroup);
    Serial.print(":");
    Serial.print(response.sourceId);
    Serial.print(" - StructID: 0x");
    Serial.print(response.structId, HEX);
    Serial.print(", Tamanho: ");
    Serial.print(response.binaryDataLen);
    Serial.println(" bytes");
    
    // Verificar STRUCT_ID para determinar o tipo de dados
    if (response.structId == STRUCT_ID_WEATHER) {
        // Tentar deserializar como WeatherData_t
        WeatherData_t weatherData;
        if (deserializarWeatherData(response.binaryData, response.binaryDataLen, &weatherData)) {
            /*
            struct WeatherData_t {
                float temperature;
                float humidity;
                float pressure;
                float windSpeed;
                float windDirection;
                float windGust;
                float windGustDirection;
            };
            */
            Serial.println("📦 Dados meteorológicos recebidos:");
            Serial.print("  Temperatura: ");
            Serial.println(weatherData.temperature);
            Serial.print("  Umidade: ");
            Serial.println(weatherData.humidity);
            Serial.print("  Pressão: ");
            Serial.println(weatherData.pressure);
            Serial.print("  Velocidade do vento: ");
            Serial.println(weatherData.windSpeed);
            Serial.print("  Direção do vento: ");
            Serial.println(weatherData.windDirection);
            Serial.print("  Forca do vento: ");
            Serial.println(weatherData.windGust);
            Serial.print("  Direção da forca do vento: ");
            Serial.println(weatherData.windGustDirection);
            
            // Mostrar temperatura e umidade nos labels
            txttemp.setText(String(weatherData.temperature, 1) + "°C");
            txtumidade.setText(String(weatherData.humidity, 1) + "%");
        } else {
            Serial.println("❌ Erro ao deserializar WeatherData_t");
        }
    } else {
        Serial.print("StructID desconhecido: 0x");
        Serial.println(response.structId, HEX);
    }
}

/**
 * @brief Callback para mensagens ERR (erros do servidor)
 * @param response Estrutura com informações do erro
 */
void onERR(const ERRResponse& response) {
    Serial.print("[ERR] Código: ");
    Serial.print(response.code);
    Serial.print(" - ");
    Serial.println(response.message);
    
    // Se erro 1 (não registrado), marca como não registrado
    if (response.code == 1) {
        deviceRegistered = false;
        ledlinker.setState(false);
    }
    
    // Se erro 10 (ERR_FORCED_DISCONNECT), reconecta automaticamente
    if (response.code == 10) {
        Serial.println("⚠️ Desconexão forçada detectada!");
        deviceRegistered = false;
        ledlinker.setState(false);
        socketClient.stop();
        delay(1000);
    }
}

/**
 * @brief Callback genérico para erros de interpretação
 * @param errorCode Código do erro
 * @param command Comando que causou o erro
 */
void onError(ErrorCode errorCode, const char* command) {
    Serial.print("[ERROR] Código: ");
    Serial.print((int)errorCode);
    Serial.print(" - Comando: ");
    Serial.println(command);
}

// ============================================
// FUNÇÕES DE CONEXÃO E GERENCIAMENTO
// ============================================

bool isServerConnected() {
    return socketClient.connected();
}

bool conectarServidorTCP() {
    Serial.print("\nConectando ao servidor ");
    Serial.print(server);
    Serial.print(":");
    Serial.println(port);
    
    if (!socketClient.connect(server, port)) {
        Serial.println("❌ Falha ao conectar ao servidor!");
        return false;
    }
    
    Serial.println("✅ Conectado ao servidor!");
    return true;
}

void sendRegisterCommand() {
    if(!isServerConnected()) {
        Serial.println("❌ Servidor desconectado! Tentando reconectar...");
        return;
    }
    
    uint8_t buffer[256];
    size_t packetSize = linker.getCommandRegisterBytes(buffer, sizeof(buffer));
    if (packetSize > 0) {
        socketClient.write(buffer, packetSize);
        Serial.print("Comando MYID enviado (");
        Serial.print(packetSize);
        Serial.println(" bytes)");
    } else {
        Serial.println("❌ Erro ao gerar comando de registro!");
    }
}

void checkReconnectServer() {
    if (!isServerConnected() && millis() - lastReconnectCheck >= RECONNECT_CHECK_INTERVAL) {
        lastReconnectCheck = millis();
        if(conectarServidorTCP()){
            delay(1000);
            sendRegisterCommand();
        }
    }
}

void sendKeepAliveCommand() {
    // Verifica se precisa enviar keep-alive
    if (linker.needSendKeepAlive() && (millis() - lastRegisterTime > 300)) {
        linker.updateKeepAliveTime();
        uint8_t buffer[16];
        size_t packetSize = linker.getCommandAliveBytes(buffer, sizeof(buffer));
        if (packetSize > 0) {
            socketClient.write(buffer, packetSize);
            Serial.println("[Keep-Alive] Enviado");
        }
    }
}

void enviarDado() {
    if (!deveEnviarDado || !deviceRegistered || !isServerConnected() || (millis() - lastRegisterTime <= 300)) {
        return;
    }
    
    // Enviar dados de texto aleatórios
    String dados = "DadoAleatorio" + String(random(0, 1000));
    
    uint8_t buffer[256];
    size_t packetSize = linker.getCommandSendToBytes(DESTINO_GRUPO, DESTINO_ID, dados, buffer, sizeof(buffer));
    if (packetSize > 0) {
        socketClient.write(buffer, packetSize);
        Serial.print("[SEND] Enviado: ");
        Serial.println(dados);
        deveEnviarDado = false; // Resetar flag após envio
    } else {
        Serial.println("❌ Erro ao gerar comando SEND!");
    }
}

void enviarPacote() {
    if (!deveEnviarPacote || !deviceRegistered || !isServerConnected() || (millis() - lastRegisterTime <= 300)) {
        return;
    }
    
    // Criar dados meteorológicos aleatórios
    WeatherData_t weatherData;
    weatherData.temperature = random(150, 350) / 10.0; // 15.0 a 35.0
    weatherData.humidity = random(300, 900) / 10.0;    // 30.0 a 90.0
    weatherData.pressure = random(9500, 10500) / 10.0; // 950.0 a 1050.0
    weatherData.windSpeed = random(0, 500) / 10.0;     // 0.0 a 50.0
    weatherData.windDirection = random(0, 360);        // 0 a 360
    weatherData.windGust = random(0, 800) / 10.0;      // 0.0 a 80.0
    weatherData.windGustDirection = random(0, 360);    // 0 a 360
    
    // Serializar dados
    uint8_t serialBuffer[256];
    size_t serialSize = serializarWeatherData(&weatherData, serialBuffer, sizeof(serialBuffer));
    
    if (serialSize > 0) {
        // Enviar usando SEND_BINARY para dispositivo específico
        uint8_t packetBuffer[256];
        size_t packetSize = linker.getCommandSendBinaryBytes(DESTINO_GRUPO, DESTINO_ID, STRUCT_ID_WEATHER, 
                                                                 serialBuffer, serialSize, 
                                                                 packetBuffer, sizeof(packetBuffer));
        
        if (packetSize > 0) {
            /*
            struct WeatherData_t {
                float temperature;
                float humidity;
                float pressure;
                float windSpeed;
                float windDirection;
                float windGust;
                float windGustDirection;
            };
            */
            socketClient.write(packetBuffer, packetSize);
            Serial.print("[PACOTE] Enviado - StructID: 0x");
            Serial.println(STRUCT_ID_WEATHER, HEX);
            Serial.printf("%0.2f°C, %0.2f%%, %0.2fhPa, %0.2f m/s, %0.2f°, %0.2f m/s, %0.2f°\n", weatherData.temperature, weatherData.humidity, weatherData.pressure, weatherData.windSpeed, weatherData.windDirection, weatherData.windGust, weatherData.windGustDirection);



            deveEnviarPacote = false; // Resetar flag após envio
        } else {
            Serial.println("❌ Erro ao gerar comando SEND_BINARY!");
        }
    } else {
        Serial.println("❌ Erro ao serializar WeatherData_t!");
    }
}

void readReceivedData() {
    // Lê dados do servidor (protocolo binário)
    if (socketClient.available()) {
        uint8_t buffer[256];
        size_t bytesRead = socketClient.readBytes(buffer, sizeof(buffer));
        
        if (bytesRead > 0) {
            // Processa dados recebidos
            linker.receiveData(buffer, bytesRead);
        }
    }
}

void ScanWiFi() {
  Serial.println("Scan start");
  // WiFi.scanNetworks will return the number of networks found.
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.printf("%2d", i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4ld", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2ld", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:            Serial.print("open"); break;
        case WIFI_AUTH_WEP:             Serial.print("WEP"); break;
        case WIFI_AUTH_WPA_PSK:         Serial.print("WPA"); break;
        case WIFI_AUTH_WPA2_PSK:        Serial.print("WPA2"); break;
        case WIFI_AUTH_WPA_WPA2_PSK:    Serial.print("WPA+WPA2"); break;
        case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
        case WIFI_AUTH_WPA3_PSK:        Serial.print("WPA3"); break;
        case WIFI_AUTH_WPA2_WPA3_PSK:   Serial.print("WPA2+WPA3"); break;
        case WIFI_AUTH_WAPI_PSK:        Serial.print("WAPI"); break;
        default:                        Serial.print("unknown");
      }
      Serial.println();
      delay(10);
    }
  }

  // Delete the scan result to free memory for code below.
  WiFi.scanDelete();
  Serial.println("-------------------------------------");
}

void conectarWiFi() {
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Ja esta conectado!");
    return;
  }

  

  //ScanWiFi();

  ledwifi.setColor(CFK_ORANGE);
  ledwifi.setState(true);

  // try for 5 seconds
  int startTime = millis();
  int maxTime = 5000;
  
  WiFi.begin(ssid, password);    
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < maxTime) {
    delay(1000);
    Serial.print(".");
  }

  ledwifi.setColor(CFK_COLOR10);
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("\nConectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    
    // Tentar conectar ao servidor se ainda não estiver conectado
    if (!isServerConnected()) {
      delay(500);
      if (conectarServidorTCP()) {
        delay(1000);
        sendRegisterCommand();
      }
    }
  }else{
    Serial.println("\nNao conectado!");
  }
  
}

void checkWifiConnection(){
    /*if(WiFi.status() == WL_CONNECTED){
        return;
    }*/
   //Serial.println("Check wifi connection");
    if(millis() - timeCheckWifi > intervalCheckWifi){
        Serial.println("Limite de tempo");
        conectarWiFi();
        timeCheckWifi = millis();
    }
    ledwifi.setState(WiFi.status() == WL_CONNECTED);
}

void setup(){
    Serial.begin(115200);
    bus = new Arduino_ESP32DSIPanel(
    20 /* hsync_pulse_width */, 20 /* hsync_back_porch */, 40 /* hsync_front_porch */,
    4 /* vsync_pulse_width */, 8 /*vsync_back_porch  */, 20 /* vsync_front_porch */,
    60000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    800 /* width default */, 1280 /* height default */, bus, rotationScreen, true,
    DISP_RST, jd9365_init_operations, sizeof(jd9365_init_operations) / sizeof(lcd_init_cmd_t));
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
    myDisplay.startTouchGSL3680(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    WiFi.STA.begin();
    
    // Configura LinkerWX
    Serial.println("Configurando LinkerWX...");
    if (!linker.setup(DEVICE_ID, GRUPO, TOKEN)) {
        Serial.println("❌ Erro ao configurar LinkerWX!");
        while(1) delay(1000);
    }
    Serial.println("✅ LinkerWX configurado");
    
    // Registra callbacks
    linker.registerSRVCallback(onSRV);
    linker.registerRECVCallback(onRECV);
    linker.registerRECVBinaryCallback(onRECVBinary);
    linker.registerERRCallback(onERR);
    linker.registerErrorCallback(onError);
    Serial.println("✅ Callbacks registrados");
}
void loop(){
    checkWifiConnection();
    
    // Verificar e reconectar ao servidor se necessário
    checkReconnectServer();
    
    // Enviar keep-alive se necessário
    sendKeepAliveCommand();
    
    // Enviar dados de texto se flag estiver ativa
    enviarDado();
    
    // Enviar pacote binário se flag estiver ativa
    enviarPacote();
    
    // Ler dados recebidos do servidor
    readReceivedData();
    
    delay(10); // Pequeno delay para não sobrecarregar
}

void drawStripe(int x, int y,  uint16_t cor1, uint16_t cor2, uint8_t levels){
    int tamanho = 20;
    uint16_t cores[levels];
    blendColorsRGB(cor1, cor2, levels, cores, levels);

    for (uint8_t i = 0; i < levels; i++) {
        tft->fillRect(x + (i * tamanho), y, tamanho, tamanho, cores[i]);
    }
}
void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(565, 135, 128, 99, 10, CFK_GREY4);
    tft->drawRoundRect(565, 135, 128, 99, 10, CFK_BLACK);
    tft->fillRoundRect(40, 145, 370, 66, 10, CFK_GREY4);
    tft->drawRoundRect(40, 145, 370, 66, 10, CFK_BLACK);
    myDisplay.printText("Teste LinkerWx", 410, 32, TC_DATUM, CFK_COLOR05, CFK_GREY3, &RobotoRegular10pt7b);
    //This screen has a/an label
    //This screen has a/an textButton
    //This screen has a/an textButton
    myDisplay.printText("Struct", 200, 361, TC_DATUM, CFK_COLOR12, CFK_GREY3, &RobotoRegular8pt7b);
    tft->fillRoundRect(35, 400, 370, 202, 10, CFK_GREY4);
    tft->drawRoundRect(35, 400, 370, 202, 10, CFK_BLACK);
    //This screen has a/an label
    myDisplay.printText("Mensagem", 225, 86, TC_DATUM, CFK_COLOR12, CFK_GREY3, &RobotoRegular8pt7b);
    //This screen has a/an label
    myDisplay.printText("WiFi", 635, 151, TC_DATUM, CFK_COLOR12, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Linker", 640, 196, TC_DATUM, CFK_COLOR12, CFK_GREY4, &RobotoRegular8pt7b);
    //This screen has a/an led
    //This screen has a/an led

    uint8_t levels = 32;
    drawStripe(0, 800, 0x0, 0xFFFF, levels);
    drawStripe(0, 820, 0xf800, 0xFFFF, levels);
    drawStripe(0, 840, 0xf81c, 0x07e1, levels);
    drawStripe(0, 860, 0x7511, 0x01a4, levels);
    drawStripe(0, 880, 0x9f1f, 0xccff, levels);
    drawStripe(0, 900, 0xFFFF, 0x0, levels);
    drawStripe(0, 920, 0x7280, 0x07ff, levels);
    drawStripe(0, 940, 0xf800, 0xf81b, levels);
    drawStripe(0, 960, 0xFFFF, 0xFFFF, levels);

    
    drawStripe(0, 980, 0xf81b, darken565(0xf81b, 1), levels);
    drawStripe(0, 1000, 0xf81b, darken565(0xf81b, 0.5), levels);
    drawStripe(0, 1020, 0xb420, lighten565(0xb420, 1), levels);
    drawStripe(0, 1040, 0xb420, lighten565(0xb420, 0.5), levels);

    


    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets(){
    LabelConfig configLabel0 = {
            .text = "Mensagens",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR12,
            .backgroundColor = CFK_GREY4
        };
    txtlog.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "temp",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR12,
            .backgroundColor = CFK_GREY4
        };
    txttemp.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "umidade",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR12,
            .backgroundColor = CFK_GREY4
        };
    txtumidade.setup(configLabel2);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR10,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    ledwifi.setup(configLed0);
    LedConfig configLed1 = {
            .radius = 16,
            .colorOn = CFK_COLOR10,
            .colorOff = CFK_GREY3,
            .initialState = false
        };
    ledlinker.setup(configLed1);
    myDisplay.setLed(arrayLed,qtdLed);

    TextButtonConfig configTextButton0 = {
            .text = "Enviar texto",
            .callback = btntexto_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 162,
            .height = 36,
            .radius = 10,
            .backgroundColor = CFK_COLOR08,
            .textColor = CFK_BLACK
        };

    btntexto.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Enviar struct",
            .callback = btnstruct1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 164,
            .height = 38,
            .radius = 10,
            .backgroundColor = CFK_COLOR15,
            .textColor = CFK_BLACK
        };

    btnstruct1.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

}
// This function is a callback of this element btntexto.
// You dont need call it. Make sure it is as short and quick as possible.
void btntexto_cb(){
    Serial.print("Clicked on: ");Serial.println("btntexto_cb");
    deveEnviarDado = true; // Ativar flag para enviar dados de texto
}
// This function is a callback of this element btnstruct1.
// You dont need call it. Make sure it is as short and quick as possible.
void btnstruct1_cb(){
    Serial.print("Clicked on: ");Serial.println("btnstruct1_cb");
    deveEnviarPacote = true; // Ativar flag para enviar pacote binário
}