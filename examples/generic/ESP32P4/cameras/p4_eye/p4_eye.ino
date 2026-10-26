// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>
#include <ESP_Video.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7789 - SPIDMA
    * Touch: None
    * Camera: OV2710
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 240;
const int DISPLAY_H = 240;
const int DISP_FREQUENCY = 40000000;
const int CAM_SDA = 14;
const int CAM_SCL = 13;
const int CAM_FREQUENCY_SCCB = 24000000;
const int CAM_INT = -1;
const int CAM_RST = 26;
const int CAM_WIDTH = 1920;
const int CAM_HEIGHT = 1080;
const int CAM_I2C_PORT = 0;
const bool CAM_MIRROR_X = false;
const bool CAM_MIRROR_Y = false;
const int CAM_PIN_CLK = 11;
const int CAM_PIN_PWR_EN = 12;
const int DISP_RST = 15;
const int DISP_DC = 19;
const int DISP_CS = 18;
const int DISP_MOSI = 16;
const int DISP_MISO = -1;
const int DISP_SCLK = 17;
const int DISP_LED = 20;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

const int ZOOM_PIN_1X = 3;
const int ZOOM_PIN_2X = 4;
const int ZOOM_PIN_3X = 5;

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayFK myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
CameraMipi cameraMipi(0, 0, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};


static void beginZoomButtons()
{
    pinMode(ZOOM_PIN_1X, INPUT_PULLUP);
    pinMode(ZOOM_PIN_2X, INPUT_PULLUP);
    pinMode(ZOOM_PIN_3X, INPUT_PULLUP);
}

static void pollZoomButtons()
{
    if (digitalRead(ZOOM_PIN_1X) == LOW) {
        cameraMipi.setZoomLevel(1);
        return;
    }
    if (digitalRead(ZOOM_PIN_2X) == LOW) {
        cameraMipi.setZoomLevel(2);
        return;
    }
    if (digitalRead(ZOOM_PIN_3X) == LOW) {
        cameraMipi.setZoomLevel(4);
    }
}

// linha de camera
void setup(){

    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    // Touch model not defined. No need startTouch() function.
    Serial.begin(115200);
    beginZoomButtons();
    
    // Setup camera
    if (!myDisplay.getCamera().enableBoardPower(CAM_PIN_PWR_EN)){
        Serial.println("failed to enable camera/LCD power (GPIO CAM_PIN_PWR_EN)");
        return;
    }
    if (!myDisplay.getCamera().startXclk(CAM_PIN_CLK, CAM_FREQUENCY_SCCB)){
        return;
    }
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, 400000, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        myDisplay.getCamera().startPpa();
    }else{
        Serial.println("Camera not started");
    }
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, LOW);
    bus = new Arduino_ESP32SPIDMA(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, SPI2_HOST + 1, false);
    tft = new Arduino_ST7789(bus, DISP_RST, rotationScreen, isIPS, DISPLAY_W, DISPLAY_H, 0, 0, 0, 0);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
    
}

void loop(){

    pollZoomButtons();
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an cameraMipi
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    CameraMipiConfig configCameraMipi0 = {
            .width = 240,
            .height = 240,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


}
