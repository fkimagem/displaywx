// Include for plugins of chip 2
// Include external libraries and files
#include <SPI.h>
#include <WiFi.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
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

// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 800;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 480;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 800;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int DISP_RST = 5;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = -1;

const char* ssid     = "Labmaker";
const char* password = "Anglo2020pp";
const int intervalCheckWifi = 30000;
int timeCheckWifi = 0;
bool ligarWifi = false;


// Prototypes for each screen
void screen0();
void loadWidgets();
// Prototypes for callback functions
void rectbutton_cb();
void btntexto_cb();
void btnstruct1_cb();
#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
Label txtlog(110, 110, 0);
Label txttemp(90, 249, 0);
Label txtumidade(100, 300, 0);
const uint8_t qtdLabel = 3;
Label *arrayLabel[qtdLabel] = {&txtlog, &txttemp, &txtumidade};
Led ledwifi(195, 675, 0);
Led ledlinker(195, 720, 0);
const uint8_t qtdLed = 2;
Led *arrayLed[qtdLed] = {&ledwifi, &ledlinker};
RectButton rectbutton(395, 585, 0);
const uint8_t qtdRectBtn = 1;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton};
TextButton btntexto(165, 425, 0);
TextButton btnstruct1(165, 535, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btntexto, &btnstruct1};


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
  ESP_LOGD("MAIN", "Conectando ao WiFi %s", ssid);

  if(WiFi.status() == WL_CONNECTED){
    ESP_LOGD("MAIN", "Ja esta conectado!");
    return;
  }

  ScanWiFi();

  /*ledwifi.setColor(CFK_ORANGE);
  ledwifi.setState(true);

  // try for 5 seconds
  int startTime = millis();
  int maxTime = 5000;

  ESP_LOGD("MAIN", "Definindo modo estacao");
  WiFi.mode(WIFI_STA);  // Define modo estação

  ESP_LOGD("MAIN", "Desligando WiFi");
  WiFi.disconnect(true, true);

  delay(1000);

  ESP_LOGD("MAIN", "Fazendo begin(%s, %s) na WiFi", ssid, password);
  WiFi.begin(ssid, password);
  delay(1000);

  ESP_LOGD("MAIN", "Verificando conexao na WiFi");
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < maxTime) {
    delay(1000);
    Serial.print(".");
  }

  ledwifi.setColor(CFK_COLOR10);
  if(WiFi.status() == WL_CONNECTED){
    ESP_LOGD("MAIN", "Conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  }else{
    ESP_LOGD("MAIN", "Nao conectado!");
  }
  */
}

void checkWifiConnection(){
    ESP_LOGD("MAIN", "Check wifi connection");
    if(WiFi.status() == WL_CONNECTED){
        return;
    }
    if(millis() - timeCheckWifi > intervalCheckWifi){
        conectarWiFi();
        timeCheckWifi = millis();
    }
    ledwifi.setState(WiFi.status() == WL_CONNECTED);
}

void setup(){
    Serial.begin(115200);
    //bus = new Arduino_ESP32DSIPanel(
    //12  /* hsync_pulse_width */,  42  /* hsync_back_porch */,  42  /* hsync_front_porch */,
    //2   /* vsync_pulse_width */, 8   /* vsync_back_porch */,  166 /* vsync_front_porch */,
    //34000000 /* prefer_speed */);
    //tft = new Arduino_DSI_Display(
    //480 /* width default */, 800 /* height default */, bus, rotationScreen, true,
    //DISP_RST, st7701_init_operations, sizeof(st7701_init_operations) / sizeof(lcd_init_cmd_t));
    /*tft->begin();
    
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif*/
    //
    //WiFi.STA.begin();

    
}
void loop(){
    if(ligarWifi){
        ligarWifi = false;
        checkWifiConnection();
    }
    checkWifiConnection();
    /*txtlog.setText("Mensagens"); //Use this command to change widget value.
    txttemp.setText("temp"); //Use this command to change widget value.
    txtumidade.setText("umidade"); //Use this command to change widget value.
    ledwifi.setState(true);
    ledlinker.setState(true);*/
    //
    delay(2000);
}
void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(170, 650, 128, 99, 10, CFK_GREY4);
    tft->drawRoundRect(170, 650, 128, 99, 10, CFK_BLACK);
    tft->fillRoundRect(55, 85, 370, 66, 10, CFK_GREY4);
    tft->drawRoundRect(55, 85, 370, 66, 10, CFK_BLACK);
    myDisplay.printText("Teste LinkerWx", 245, 17, TC_DATUM, CFK_COLOR05, CFK_GREY3, &RobotoRegular10pt7b);
    //This screen has a/an label
    //This screen has a/an textButton
    //This screen has a/an textButton
    myDisplay.printText("Struct", 245, 201, TC_DATUM, CFK_COLOR12, CFK_GREY3, &RobotoRegular8pt7b);
    tft->fillRoundRect(50, 220, 370, 132, 10, CFK_GREY4);
    tft->drawRoundRect(50, 220, 370, 132, 10, CFK_BLACK);
    //This screen has a/an label
    myDisplay.printText("Mensagem", 245, 56, TC_DATUM, CFK_COLOR12, CFK_GREY3, &RobotoRegular8pt7b);
    //This screen has a/an label
    myDisplay.printText("WiFi", 245, 671, TC_DATUM, CFK_COLOR12, CFK_GREY4, &RobotoRegular8pt7b);
    myDisplay.printText("Linker", 250, 716, TC_DATUM, CFK_COLOR12, CFK_GREY4, &RobotoRegular8pt7b);
    //This screen has a/an led
    //This screen has a/an led
    //This screen has a/an rectButton
    myDisplay.drawWidgetsOnScreen(0);
}
// Configure each widgtes to be used
void loadWidgets(){
    String text = getResetReasonString();
    LabelConfig configLabel0 = {
            .text = text.c_str(),
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TC_DATUM,
            .fontColor = CFK_COLOR12,
            .backgroundColor = CFK_GREY4
        };
    txtlog.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "temp",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TC_DATUM,
            .fontColor = CFK_COLOR12,
            .backgroundColor = CFK_GREY4
        };
    txttemp.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "umidade",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular8pt7b,
            .datum = TC_DATUM,
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

    RectButtonConfig configRectButton0 = {
            .callback = rectbutton_cb,
            .width = 75,
            .height = 52,
            .pressedColor = CFK_COLOR18
        };
rectbutton.setup(configRectButton0);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);

    TextButtonConfig configTextButton0 = {
            .text = "Enviar texto",
            .callback = btntexto_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 161,
            .height = 35,
            .radius = 10,
            .backgroundColor = CFK_COLOR08,
            .textColor = CFK_BLACK
        };

    btntexto.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Enviar struct",
            .callback = btnstruct1_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 163,
            .height = 37,
            .radius = 10,
            .backgroundColor = CFK_COLOR15,
            .textColor = CFK_BLACK
        };

    btnstruct1.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

}
// This function is a callback of this element rectbutton.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton_cb(){
    bool myValue = rectbutton.getStatus();
    if(myValue){
        ligarWifi = true;
    }
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element btntexto.
// You dont need call it. Make sure it is as short and quick as possible.
void btntexto_cb(){
    Serial.print("Clicked on: ");Serial.println("btntexto_cb");
}
// This function is a callback of this element btnstruct1.
// You dont need call it. Make sure it is as short and quick as possible.
void btnstruct1_cb(){
    ESP_LOGD("MAIN", "Clicked on: btnstruct1_cb");
    Serial.print("Clicked on: ");Serial.println("btnstruct1_cb");
}