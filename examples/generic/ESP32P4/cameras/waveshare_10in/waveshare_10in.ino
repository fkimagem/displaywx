// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <ESP_Video.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: JD9365 - DSI
    * Touch: GSL3680
    * Camera: OV02C10
    */
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
const int DISP_RST = 27;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = 21;
const int TOUCH_RST = 22;
const uint8_t rotationScreen = 0; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void textbutton1_cb();

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
CameraMipi cameraMipi(135, 50, 0);
const uint8_t qtdCameraMipi = 1;
CameraMipi *arrayCameraMipi[qtdCameraMipi] = {&cameraMipi};

TextButton textbutton1(216, 915, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton1};

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
            .width = 213,
            .height = 166,
            .showZoomLevel = false,
            .showFps = false,
            .fps = 5
        };

    cameraMipi.setup(configCameraMipi0);
    myDisplay.setCameraMipi(arrayCameraMipi,qtdCameraMipi);

    TextButtonConfig configTextButton0 = {
        .text = "Edit",
        .callback = textbutton1_cb,
        .fontFamily = &RobotoBold10pt7b,
        .width = 194,
        .height = 93,
        .radius = 10,
        .backgroundColor = CFK_COLOR030,
        .textColor = CFK_WHITE
    };

textbutton1.setup(configTextButton0);
myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

void textbutton1_cb(){

    Serial.print("Clicked on: ");Serial.println("textbutton1_cb");
    cameraMipi.openConfig();

}


