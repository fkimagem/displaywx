// [legenda] [manual] = sketch inteiro escrito a mao. Nao passou pelo exportador do EdgeWX.
// [legenda] So conecta o Wi-Fi. Nao ha display nem widget.
// [manual] Wi-Fi deste exemplo. O export do display nao traz essa biblioteca.
#include <WiFi.h>

// [manual] Nome da rede.
const char* ssid = "Labmaker";
// [manual] Senha da rede. Fica escrita neste exemplo.
const char* password = "Anglo2020pp";

// [manual] Liga o Wi-Fi se ainda nao houver link. Desiste em 5 s.
void conectarWiFi() {
  // [manual] Ja conectado: nao tenta de novo.
  if (WiFi.status() == WL_CONNECTED) return;

  // [manual] Mensagem na serial.
  Serial.print("Conectando ao WiFi: ");
  // [manual] Mensagem na serial.
  Serial.println(ssid);

  // [manual] Senha da rede. Fica escrita neste exemplo.
  WiFi.begin(ssid, password);

  // [manual] Desiste depois de 5 s.
  int timeout = 5000;
  // [manual] Marca o inicio da espera.
  unsigned long start = millis();
  // [manual] Desiste depois de 5 s.
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
    delay(500);
    // [manual] Mensagem na serial.
    Serial.print(".");
  }

  // [manual] Condicao ou repeticao.
  if (WiFi.status() == WL_CONNECTED) {
    // [manual] Mensagem na serial.
    Serial.println("\nConectado!");
    // [manual] Mensagem na serial.
    Serial.print("IP: ");
    // [manual] Mostra o IP quando o link sobe.
    Serial.println(WiFi.localIP());
  // [manual] Ramo falso da condicao de cima.
  } else {
    // [manual] Avisa que o prazo acabou sem link.
    Serial.println("\nFalha na conexão.");
  }
}

// [manual] Sobe serial, painel, toque e a tarefa de UI.
void setup() {
  // [manual] Abre a serial a 115200.
  Serial.begin(115200);
  // [manual] Modo estacao.
  WiFi.mode(WIFI_STA);
  // [manual] Limpa uma associacao anterior. O comentario do loop pede para nao repetir isto.
  WiFi.disconnect();
  // [manual] Espera este tempo. Toque e desenho, se createTask rodou, seguem na outra tarefa.
  delay(100);
  // [manual] Tenta o link. Se ja estiver conectado, a funcao volta na hora.
  conectarWiFi();
}

// [manual] Laco do Arduino. Com createTask, o desenho nao mora aqui.
void loop() {
  // [manual] Comentario que ja estava no arquivo.
  // Aqui, para reconectar só chame conectarWiFi() se desconectado,
  // [manual] Comentario que ja estava no arquivo.
  // sem repetir WiFi.mode() ou WiFi.disconnect()
  // [manual] Condicao ou repeticao.
  if (WiFi.status() != WL_CONNECTED) {
    // [manual] Tenta o link. Se ja estiver conectado, a funcao volta na hora.
    conectarWiFi();
  }
}
