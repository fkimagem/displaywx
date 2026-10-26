// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define TC_SCL	20
#define TC_SDA	19
#define TC_INT	-1
#define TC_RST	38
#define TOUCH_ROTATION	ROTATION_NORMAL
#define TOUCH_MAP_X0	0
#define TOUCH_MAP_X1	800
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	480
#define TOUCH_SWAP_XY	false
#define TOUCH_INVERT_X	false
#define TOUCH_INVERT_Y	false

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

// Include for plugins of chip 1
#include <ModbusMaster.h>
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
#define DISP_LED 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
GaugeSuper gaugecorrente(660, 244, 0);
GaugeSuper gaugetensao(659, 122, 0);
GaugeSuper gaugecorrente2(276, 220, 0);
const uint8_t qtdGauge = 3;
GaugeSuper *arrayGauge[qtdGauge] = {&gaugecorrente, &gaugetensao, &gaugecorrente2};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {0,15,30,45};
uint16_t colors0[qtdIntervalG0] = {CFK_WHITE,CFK_WHITE,CFK_WHITE,CFK_WHITE};
const uint8_t qtdIntervalG1 = 4;
int range1[qtdIntervalG1] = {0,60,120,180};
uint16_t colors1[qtdIntervalG1] = {CFK_WHITE,CFK_WHITE,CFK_WHITE,CFK_WHITE};
const uint8_t qtdIntervalG2 = 4;
int range2[qtdIntervalG2] = {0,3500,7000,10500};
uint16_t colors2[qtdIntervalG2] = {CFK_COLOR13,CFK_COLOR07,CFK_COLOR03,CFK_COLOR02};
Label line1(328, 234, 0);
Label text(420, 275, 0);
Label text2(425, 315, 0);
Label text3(415, 355, 0);
Label text4(435, 395, 0);
Label text5(430, 435, 0);
Label text6(645, 325, 0);
Label text7(645, 370, 0);
Label text8(645, 415, 0);
const uint8_t qtdLabel = 9;
Label *arrayLabel[qtdLabel] = {&line1, &text, &text2, &text3, &text4, &text5, &text6, &text7, &text8};
LineChart grafico(45, 280, 0);
const uint8_t qtdLineChart = 1;
LineChart *arrayLinechart[qtdLineChart] = {&grafico};
Label *seriesGrafico0[2] = {&line1,nullptr};
const uint8_t qtdLinesChart0 = 2;
uint16_t colorsChart0[qtdLinesChart0] = {CFK_COLOR01,CFK_COLOR11};
const int rxPin_1 = 11;
const int txPin_1 = 12;
const int dePin_1 = 13;
const uint8_t slaveId_1 = 1;
const unsigned long baudRate_1 = 9600;
ModbusMaster modbusNode_1;
TaskHandle_t modbusTaskHandle_1 = NULL;
unsigned long lastRead_1 = 0;
const unsigned long readInterval_1 = 2000;
unsigned long modbusTimer_1 = 0;
const unsigned long modbusDelay_1 = 150;
int modbusStep_1 = 0;
struct {
  float totalKwh_1;
  float exportKwh_1;
  float importKwh_1;
  float voltage_1;
  float current_1;
  float activePower_1;
  float reactivePower_1;
  float powerFactor_1;
  float frequency_1;
  int deviceId_1;
  int baudrate_1;
} readings_1;
void preTransmission_1();
void postTransmission_1();
bool readWithRetry_1(uint16_t startAddress, uint16_t quantity, uint16_t* buffer, unsigned long timeout);
float readRegister32Scaled_1(uint16_t address);
float readRegister16Scaled_1(uint16_t address, float divisor);
int readDeviceId_1(uint16_t address);
int readBaudrate_1(uint16_t address);
void handleModbusStep_1();
void printReadings_1();
void modbusTask_1(void *parameter);
// Texto de exemplo.

void setup(){
    Serial.begin(115200);
    rgbpanel = new Arduino_ESP32RGBPanel(
    41, 40, 39, 42,
    14, 21, 47, 48, 45,
    9, 46, 3, 8, 16, 1,
    15, 7, 6, 5, 4,
    0, 210, 30, 16,
    0, 22, 13, 10,
    1, 16000000, false,
    0, 0, 0);
    tft = new Arduino_RGB_Display(
    800, 480, rgbpanel,
    0, true);
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, TC_INT, TC_RST);
    //myDisplay.enableTouchLog();
    //myDisplay.recalibrate();
    //myDisplay.checkCalibration();
    loadWidgets(); // This function is used to setup with widget individualy
    Serial.begin(115200);
    pinMode(dePin_1, OUTPUT);
    digitalWrite(dePin_1, LOW);
    Serial2.begin(baudRate_1, SERIAL_8N1, rxPin_1, txPin_1);
    modbusNode_1.begin(slaveId_1, Serial2);
    modbusNode_1.preTransmission(preTransmission_1);
    modbusNode_1.postTransmission(postTransmission_1);
    Serial.println("READY DEVICE 1");
    xTaskCreatePinnedToCore(
      modbusTask_1,
      "ModbusTask",
      4096,
      NULL,
      1,
      &modbusTaskHandle_1,
      1
    );
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(true,1); // Initialize the task to read touch and draw
}

void loop(){
    delay(5000);
    gaugecorrente.setValue(readings_1.current_1); //Use this command to change widget value.
    gaugetensao.setValue(readings_1.voltage_1); //Use this command to change widget value.
    gaugecorrente2.setValue(readings_1.activePower_1); //Use this command to change widget value.
    line1.setTextFloat(1.000, 2); //Use this command to change widget value.
    text.setTextFloat(000.0, 2); //Use this command to change widget value.
    text2.setTextFloat(00.00, 2); //Use this command to change widget value.
    text3.setTextFloat(000.000, 2); //Use this command to change widget value.
    text4.setTextFloat(1.000, 2); //Use this command to change widget value.
    text5.setTextFloat(60.00, 2); //Use this command to change widget value.
    text6.setTextFloat(000000.00, 2); //Use this command to change widget value.
    text7.setTextFloat(000000.00, 2); //Use this command to change widget value.
    text8.setTextFloat(000000.00, 2); //Use this command to change widget value.
    grafico.push(0, random(0, 14)); //Use this line to add value in serie 0.
    
    // Commands for plugin Modbus energy meter 127/220
    if (millis() - lastRead_1 >= readInterval_1) {
      lastRead_1 = millis();
      printReadings_1();
    }
}

void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(531, 265, 242, 200, 10, CFK_GREY5);
    tft->drawRoundRect(531, 265, 242, 200, 10, CFK_BLACK);
    tft->fillRoundRect(101, 229, 340, 33, 10, CFK_GREY5);
    tft->drawRoundRect(101, 229, 340, 33, 10, CFK_BLACK);
    //This screen has a/an grafico
    //This screen has a/an ponteiroAnalog
    //This screen has a/an ponteiroAnalog
    //This screen has a/an ponteiroAnalog
    myDisplay.printText("Total", 560, 327, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("Import", 545, 372, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("Export", 545, 417, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    tft->fillRoundRect(300, 350, 194, 33, 10, CFK_GREY5);
    tft->drawRoundRect(300, 350, 194, 33, 10, CFK_BLACK);
    tft->fillRoundRect(300, 390, 196, 33, 10, CFK_GREY5);
    tft->drawRoundRect(300, 390, 196, 33, 10, CFK_BLACK);
    tft->fillRoundRect(300, 430, 195, 33, 10, CFK_GREY5);
    tft->drawRoundRect(300, 430, 195, 33, 10, CFK_BLACK);
    myDisplay.printText("kWh", 635, 277, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("W", 53, 183, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular15pt7b);
    myDisplay.printText("A", 535, 217, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("V", 535, 92, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    tft->fillRoundRect(300, 310, 191, 33, 10, CFK_GREY5);
    tft->drawRoundRect(300, 310, 191, 33, 10, CFK_BLACK);
    tft->fillRoundRect(300, 270, 193, 33, 10, CFK_GREY5);
    tft->drawRoundRect(300, 270, 193, 33, 10, CFK_BLACK);
    myDisplay.printText("Active Power:", 138, 236, TL_DATUM, CFK_COLOR01, CFK_GREY5, &RobotoRegular10pt7b);
    //This screen has a/an label
    myDisplay.printText("Voltage:", 310, 277, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("Current:", 310, 317, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("Reactive P.:", 310, 357, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("P. Factor:", 310, 397, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    myDisplay.printText("Frequency:", 310, 437, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular10pt7b);
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    myDisplay.printText("kW", 230, 286, TL_DATUM, CFK_WHITE, CFK_GREY5, &RobotoRegular5pt7b);
    //This screen has a/an label
    //This screen has a/an label
    //This screen has a/an label
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    GaugeConfig configGauge0 = {
            .title = "Current",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 60,
            .width = 260,
            .borderColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5,
            .titleColor = CFK_WHITE,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    gaugecorrente.setup(configGauge0);
    GaugeConfig configGauge1 = {
            .title = "Voltage",
            .intervals = range1,
            .colors = colors1,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 240,
            .width = 261,
            .borderColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5,
            .titleColor = CFK_WHITE,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG1,
            .showLabels = true
        };
    gaugetensao.setup(configGauge1);
    GaugeConfig configGauge2 = {
            .title = "Active Power",
            .intervals = range2,
            .colors = colors2,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 14000,
            .width = 460,
            .borderColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5,
            .titleColor = CFK_WHITE,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG2,
            .showLabels = true
        };
    gaugecorrente2.setup(configGauge2);
    myDisplay.setGauge(arrayGauge,qtdGauge);
    LabelConfig configLabel0 = {
            .text = "1.000",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR01,
            .backgroundColor = CFK_GREY5
        };
    line1.setup(configLabel0);
    LabelConfig configLabel1 = {
            .text = "000.0",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text.setup(configLabel1);
    LabelConfig configLabel2 = {
            .text = "00.00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text2.setup(configLabel2);
    LabelConfig configLabel3 = {
            .text = "000.000",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text3.setup(configLabel3);
    LabelConfig configLabel4 = {
            .text = "1.000",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text4.setup(configLabel4);
    LabelConfig configLabel5 = {
            .text = "60.00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text5.setup(configLabel5);
    LabelConfig configLabel6 = {
            .text = "000000.00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text6.setup(configLabel6);
    LabelConfig configLabel7 = {
            .text = "000000.00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text7.setup(configLabel7);
    LabelConfig configLabel8 = {
            .text = "000000.00",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_WHITE,
            .backgroundColor = CFK_GREY5
        };
    text8.setup(configLabel8);
    myDisplay.setLabel(arrayLabel,qtdLabel);
    LineChartConfig configLineChart0 = {
            .colorsSeries = colorsChart0,
            .subtitles = seriesGrafico0,
            .font = &RobotoRegular5pt7b,
            .minValue = 0,
            .maxValue = 14,
            .width = 240,
            .height = 181,
            .gridColor = CFK_BLACK,
            .borderColor = CFK_BLACK,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_WHITE,
            .verticalDivision = 5,
            .maxPointsAmount = LineChart::SHOW_ALL,
            .amountSeries = qtdLinesChart0,
            .workInBackground = false,
            .showZeroLine = true,
            .boldLine = true,
            .showDots = false
        };
    grafico.setup(configLineChart0);
    myDisplay.setLineChart(arrayLinechart,qtdLineChart);
}
// Function for plugin Modbus energy meter 127/220
void modbusTask_1(void *parameter) {
  while (true) {
    handleModbusStep_1();
    delay(50);
  }
}
void preTransmission_1() {
  digitalWrite(dePin_1, HIGH);
}
void postTransmission_1() {
  digitalWrite(dePin_1, LOW);
}
bool readWithRetry_1(uint16_t startAddress, uint16_t quantity, uint16_t* buffer, unsigned long timeout) {
  const int maxRetries = 3;
  for (int i = 0; i < maxRetries; i++) {
    unsigned long start = millis();
    uint8_t result = modbusNode_1.readHoldingRegisters(startAddress, quantity);
    while (result != modbusNode_1.ku8MBSuccess && millis() - start < timeout) {
      delay(10);
      result = modbusNode_1.readHoldingRegisters(startAddress, quantity);
    }
    if (result == modbusNode_1.ku8MBSuccess) {
      for (int j = 0; j < quantity; j++) {
        buffer[j] = modbusNode_1.getResponseBuffer(j);
      }
      return true;
    }
    delay(50);
  }
  return false;
}
float readRegister32Scaled_1(uint16_t address) {
  uint16_t buffer[2];
  if (readWithRetry_1(address, 2, buffer, 100)) {
    uint32_t val = ((uint32_t)buffer[0] << 16) | buffer[1];
    return val / 100.0;
  }
  return -1.0;
}
float readRegister16Scaled_1(uint16_t address, float divisor) {
  uint16_t buffer[1];
  if (readWithRetry_1(address, 1, buffer, 100)) {
    return buffer[0] / divisor;
  }
  return -1.0;
}
int readDeviceId_1(uint16_t address) {
  uint16_t buffer[1];
  if (readWithRetry_1(address, 1, buffer, 100)) {
    return (buffer[0] >> 8) & 0xFF;
  }
  return -1;
}
int readBaudrate_1(uint16_t address) {
  uint16_t buffer[1];
  if (readWithRetry_1(address, 1, buffer, 100)) {
    uint8_t baudCode = buffer[0] & 0xFF;
    switch (baudCode) {
      case 1: return 9600;
      case 2: return 4800;
      case 3: return 2400;
      case 4: return 1200;
      default: return 9600;
    }
  }
  return -1;
}
void handleModbusStep_1() {
  if (millis() - modbusTimer_1 < modbusDelay_1) return;
  modbusTimer_1 = millis();
  switch (modbusStep_1) {
    case 0: readings_1.totalKwh_1 = readRegister32Scaled_1(0x0000); break;
    case 1: readings_1.exportKwh_1 = readRegister32Scaled_1(0x0008); break;
    case 2: readings_1.importKwh_1 = readRegister32Scaled_1(0x000A); break;
    case 3: readings_1.voltage_1 = readRegister16Scaled_1(0x000C, 10.0); break;
    case 4: readings_1.current_1 = readRegister16Scaled_1(0x000D, 100.0); break;
    case 5: readings_1.activePower_1 = readRegister16Scaled_1(0x000E, 1000.0); break;
    case 6: readings_1.reactivePower_1 = readRegister16Scaled_1(0x000F, 1.0); break;
    case 7: readings_1.powerFactor_1 = readRegister16Scaled_1(0x0010, 1000.0); break;
    case 8: readings_1.frequency_1 = readRegister16Scaled_1(0x0011, 100.0); break;
    case 9: readings_1.deviceId_1 = readDeviceId_1(0x0015); break;
    case 10: readings_1.baudrate_1 = readBaudrate_1(0x0015); break;
  }
  modbusStep_1++;
  if (modbusStep_1 > 10) modbusStep_1 = 0;
}
void printReadings_1() {
  Serial.println("\n=== Leitura Dispositivo ===");
  Serial.printf("Total kWh: %.2f\n", readings_1.totalKwh_1);
  Serial.printf("Export kWh: %.2f\n", readings_1.exportKwh_1);
  Serial.printf("Import kWh: %.2f\n", readings_1.importKwh_1);
  Serial.printf("Tensão: %.1f V\n", readings_1.voltage_1);
  Serial.printf("Corrente: %.2f A\n", readings_1.current_1);
  Serial.printf("Potência ativa: %.3f kW\n", readings_1.activePower_1);
  Serial.printf("Potência reativa: %.1f VAR\n", readings_1.reactivePower_1);
  Serial.printf("Fator potência: %.3f\n", readings_1.powerFactor_1);
  Serial.printf("Frequência: %.2f Hz\n", readings_1.frequency_1);
  Serial.printf("ID do dispositivo: %d\n", readings_1.deviceId_1);
  Serial.printf("Baudrate: %d\n", readings_1.baudrate_1);
  Serial.println("===============================");
}
