// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <ESP_Video.h>


    /* Project setup:
    * MCU: ESP32P4
    * Display: ILI9881C - DSI
    * Touch: GT911_WAVESHARE
    * Camera: OV5647
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 720;
const int DISPLAY_H = 1280;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 720;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 1280;
const bool TOUCH_SWAP_XY = false;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = false;
const int CAM_SDA = 7;
const int CAM_SCL = 8;
const int CAM_FREQUENCY_SCCB = 400000;
const int CAM_INT = -1;
const int CAM_RST = -1;
const int CAM_WIDTH = 1920;
const int CAM_HEIGHT = 1080;
const int CAM_I2C_PORT = 0;
const bool CAM_MIRROR_X = false;
const bool CAM_MIRROR_Y = false;
const int DISP_RST = -1;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
CameraMipi cameraMipi(200, 105, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

// linha de camera
void setup(){

    Serial.begin(115200);
    // Setup camera
    myDisplay.setRotationCamera(rotationScreen, CAM_MIRROR_X, CAM_MIRROR_Y);
    if(myDisplay.startCamera(CAM_I2C_PORT, CAM_SCL, CAM_SDA, CAM_FREQUENCY_SCCB, CAM_RST, CAM_INT, CAM_WIDTH, CAM_HEIGHT)){
        myDisplay.getCamera().startPpa();
    }else{
        Serial.println("Camera not started");
    }
    if (!myDisplay.waveshareDsiAuxBegin(ILI9881C_WAVESHARE_7_SDA, ILI9881C_WAVESHARE_7_SCL)) {
    Serial.println("AVISO: controlador auxiliar do painel falhou");
    }
    bus = new Arduino_ESP32DSIPanel(
    ILI9881C_WAVESHARE_7_HSYNC_PULSE, ILI9881C_WAVESHARE_7_HSYNC_BACK, ILI9881C_WAVESHARE_7_HSYNC_FRONT,
    ILI9881C_WAVESHARE_7_VSYNC_PULSE, ILI9881C_WAVESHARE_7_VSYNC_BACK, ILI9881C_WAVESHARE_7_VSYNC_FRONT,
    ILI9881C_WAVESHARE_7_PIXEL_CLOCK_HZ, ILI9881C_WAVESHARE_7_LANE_MBPS);
    tft = new Arduino_DSI_Display(
    720 /* width default */, 1280 /* height default */, bus, rotationScreen, true,
    DISP_RST, ili9881c_waveshare_7_init_operations, sizeof(ili9881c_waveshare_7_init_operations) / sizeof(lcd_init_cmd_t));
    tft->begin();
    myDisplay.waveshareDsiAuxBacklight(255);
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911Waveshare(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

    //esp_task_wdt_reset();
    delay(100);
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
            .width = 179,
            .height = 112,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);


}
