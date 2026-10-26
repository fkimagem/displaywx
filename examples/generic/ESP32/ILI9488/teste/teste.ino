#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480

#include <displaywx.h>

void screen0();
// Prototypes for callback functions
void cbwifi0cb_cb();

DisplayWX myDisplay;
uint8_t rotationScreen = 1;

CheckBox checkbox1(10,10, 0);
CheckBox checkbox2(50,50, 0);
CheckBox checkbox3(100,100, 0);
const uint8_t qtdCheckbox = 3;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox1, &checkbox2, &checkbox3};


#define TFT_BL 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;

void setup(){

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
        #if defined(TFT_BL)
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
        #endif
        WidgetBase::objTFT = tft; // Reference to object to draw on screen
        myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST);


    CheckBoxConfig configCheckbox1 = {
        .size = 100,
        .checkedColor = CFK_RED,
        .callback = nullptr,
    };

    CheckBoxConfig configCheckbox2 = {
        .size = 100,
        .checkedColor = CFK_BLUE,
        .callback = nullptr,
    };


    CheckBoxConfig configCheckbox3 = {
        .size = 100,
        .checkedColor = CFK_GREEN,
        .callback = cbwifi0cb_cb,
    };

    checkbox1.setup(configCheckbox1);
    checkbox2.setup(configCheckbox2);
    checkbox3.setup(configCheckbox3);

    
    myDisplay.setCheckbox(arrayCheckbox, qtdCheckbox);


    WidgetBase::loadScreen = screen0; // Use this line to change between screens

    myDisplay.createTask();
}

void screen0(){
    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    
    tft->fillRect(63, 73, 221, 175, CFK_COLOR04);
    tft->drawRect(63, 73, 221, 175, CFK_BLACK);
    myDisplay.drawWidgetsOnScreen(0);
}

void loop(){

}

void cbwifi0cb_cb(){
    bool myValue = checkbox3.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}