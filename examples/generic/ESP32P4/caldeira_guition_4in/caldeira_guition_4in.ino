// Include for plugins of chip 0
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displayfk.h>


    /* Project setup:
    * MCU: ESP32P4_EMBEDDED
    * Display: ST7701 - DSI
    * Touch: GT911
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 800;
const int DISPLAY_H = 480;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_X1 = 800;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int DISP_RST = 5;
const int TOUCH_SCL = 8;
const int TOUCH_SDA = 7;
const int TOUCH_INT = -1;
const int TOUCH_RST = -1;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = false; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void rectbutton1_cb();
void rectbutton1copy_cb();
void rectbutton1copy2_cb();
void rectbutton1copy3_cb();
void toggle1_cb();
void toggle1copy_cb();
void slider1_cb();
void slider1copy_cb();
void spinbox1_cb();
void spinbox1copy_cb();

#define DISP_LED 23
Arduino_ESP32DSIPanel *bus = nullptr;
Arduino_DSI_Display *tft = nullptr;
DisplayFK myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
HSlider slider1(430, 30, 0);
HSlider slider1copy(430, 95, 0);
const uint8_t qtdHSlider = 2;
HSlider *arrayHslider[qtdHSlider] = {&slider1, &slider1copy};
RectButton rectbutton1(35, 30, 0);
RectButton rectbutton1copy(115, 30, 0);
RectButton rectbutton1copy2(200, 30, 0);
RectButton rectbutton1copy3(295, 30, 0);
const uint8_t qtdRectBtn = 4;
RectButton *arrayRectbtn[qtdRectBtn] = {&rectbutton1, &rectbutton1copy, &rectbutton1copy2, &rectbutton1copy3};
SpinBox spinbox1(405, 230, 0);
SpinBox spinbox1copy(590, 230, 0);
const uint8_t qtdSpinbox = 2;
SpinBox *arraySpinbox[qtdSpinbox] = {&spinbox1, &spinbox1copy};
ToggleButton toggle1(455, 180, 0);
ToggleButton toggle1copy(645, 180, 0);
const uint8_t qtdToggleBtn = 2;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&toggle1, &toggle1copy};
VBar vertbar1(40, 340, 0);
VBar vertbar1copy(115, 340, 0);
VBar vertbar1copy2(200, 340, 0);
VBar vertbar1copy3(300, 340, 0);
const uint8_t qtdVBar = 4;
VBar *arrayVbar[qtdVBar] = {&vertbar1, &vertbar1copy, &vertbar1copy2, &vertbar1copy3};
CircularBar circload1(460, 390, 0);
CircularBar circload1copy(580, 390, 0);
CircularBar circload1copy2(705, 385, 0);
const uint8_t qtdCircBar = 3;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload1, &circload1copy, &circload1copy2};
Thermometer thermometer1(35, 141, 0);
Thermometer thermometer1copy(115, 140, 0);
Thermometer thermometer1copy2(200, 140, 0);
Thermometer thermometer1copy3(300, 140, 0);
const uint8_t qtdThermometer = 4;
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer1, &thermometer1copy, &thermometer1copy2, &thermometer1copy3};

void setup(){

    Serial.begin(115200);
    bus = new Arduino_ESP32DSIPanel(
    12  /* hsync_pulse_width */,  42  /* hsync_back_porch */,  42  /* hsync_front_porch */,
    2   /* vsync_pulse_width */, 8   /* vsync_back_porch */,  166 /* vsync_front_porch */,
    34000000 /* prefer_speed */);
    tft = new Arduino_DSI_Display(
    480 /* width default */, 800 /* height default */, bus, rotationScreen, true,
    DISP_RST, st7701_init_operations, sizeof(st7701_init_operations) / sizeof(lcd_init_cmd_t));
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
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){

    myDisplay.startCustomDraw();
    vertbar1.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    vertbar1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    circload1.setValue(random(0, 100)); //Use this command to change widget value.
    circload1copy.setValue(random(0, 100)); //Use this command to change widget value.
    circload1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    thermometer1.setValue(random(0, 100)); //Use this command to change widget value.
    thermometer1copy.setValue(random(0, 100)); //Use this command to change widget value.
    thermometer1copy2.setValue(random(0, 100)); //Use this command to change widget value.
    thermometer1copy3.setValue(random(0, 100)); //Use this command to change widget value.
    myDisplay.finishCustomDraw();
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_GREY3);
    WidgetBase::backgroundColor = CFK_GREY3;
    tft->fillRoundRect(15, 325, 365, 139, 10, CFK_GREY4);
    tft->drawRoundRect(15, 325, 365, 139, 10, CFK_BLACK);
    tft->fillRoundRect(15, 10, 365, 298, 10, CFK_GREY4);
    tft->drawRoundRect(15, 10, 365, 298, 10, CFK_BLACK);
    tft->fillRoundRect(395, 15, 387, 139, 10, CFK_GREY4);
    tft->drawRoundRect(395, 15, 387, 139, 10, CFK_BLACK);
    tft->fillRoundRect(395, 165, 387, 139, 10, CFK_GREY4);
    tft->drawRoundRect(395, 165, 387, 139, 10, CFK_BLACK);
    tft->fillRoundRect(395, 315, 387, 150, 10, CFK_GREY4);
    tft->drawRoundRect(395, 315, 387, 150, 10, CFK_BLACK);
    //This screen has a/an slider
    //This screen has a/an slider
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    //This screen has a/an circularBar
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an thermometer
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an rectRange
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an rectButton
    //This screen has a/an toggleButton
    //This screen has a/an toggleButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    HSliderConfig configHSlider0 = {
            .callback = slider1_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 18,
            .width = 331,
            .pressedColor = CFK_COLOR03,
            .backgroundColor = CFK_WHITE
        };
    slider1.setup(configHSlider0);
    HSliderConfig configHSlider1 = {
            .callback = slider1copy_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 18,
            .width = 331,
            .pressedColor = CFK_COLOR03,
            .backgroundColor = CFK_WHITE
        };
    slider1copy.setup(configHSlider1);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);


    RectButtonConfig configRectButton0 = {
            .width = 36,
            .height = 91,
            .pressedColor = CFK_COLOR01,
            .callback = rectbutton1_cb
        };

rectbutton1.setup(configRectButton0);
    RectButtonConfig configRectButton1 = {
            .width = 36,
            .height = 91,
            .pressedColor = CFK_COLOR02,
            .callback = rectbutton1copy_cb
        };

rectbutton1copy.setup(configRectButton1);
    RectButtonConfig configRectButton2 = {
            .width = 36,
            .height = 91,
            .pressedColor = CFK_COLOR03,
            .callback = rectbutton1copy2_cb
        };

rectbutton1copy2.setup(configRectButton2);
    RectButtonConfig configRectButton3 = {
            .width = 36,
            .height = 91,
            .pressedColor = CFK_COLOR04,
            .callback = rectbutton1copy3_cb
        };

rectbutton1copy3.setup(configRectButton3);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);


    SpinBoxConfig configSpinBox0 = {
            .callback = spinbox1_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 174,
            .height = 57,
            .step = 1,
            .color = CFK_GREY11,
            .textColor = CFK_BLACK
        };
    spinbox1.setup(configSpinBox0);
    SpinBoxConfig configSpinBox1 = {
            .callback = spinbox1copy_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 174,
            .height = 57,
            .step = 1,
            .color = CFK_GREY11,
            .textColor = CFK_BLACK
        };
    spinbox1copy.setup(configSpinBox1);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);


    ToggleButtonConfig configToggle0 = {
            .width = 74,
            .height = 37,
            .pressedColor = CFK_COLOR14,
            .callback = toggle1_cb
        };
    toggle1.setup(configToggle0);
    ToggleButtonConfig configToggle1 = {
            .width = 74,
            .height = 37,
            .pressedColor = CFK_COLOR14,
            .callback = toggle1copy_cb
        };
    toggle1copy.setup(configToggle1);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);


    VerticalBarConfig configVBar0 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 32,
            .height = 110,
            .filledColor = CFK_COLOR01
        };
    vertbar1.setup(configVBar0);
    VerticalBarConfig configVBar1 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 34,
            .height = 112,
            .filledColor = CFK_COLOR02
        };
    vertbar1copy.setup(configVBar1);
    VerticalBarConfig configVBar2 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 34,
            .height = 112,
            .filledColor = CFK_COLOR03
        };
    vertbar1copy2.setup(configVBar2);
    VerticalBarConfig configVBar3 = {
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .round = 0,
            .orientation = Orientation::VERTICAL,
            .width = 34,
            .height = 112,
            .filledColor = CFK_COLOR04
        };
    vertbar1copy3.setup(configVBar3);
    myDisplay.setVBar(arrayVbar,qtdVBar);


    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 56,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR27,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY8,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    circload1.setup(configCirculaBar0);
    CircularBarConfig configCirculaBar1 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 56,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR29,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY8,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    circload1copy.setup(configCirculaBar1);
    CircularBarConfig configCirculaBar2 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 56,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR31,
            .backgroundColor = CFK_BLACK,
            .textColor = CFK_BLACK,
            .backgroundText = CFK_GREY8,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    circload1copy2.setup(configCirculaBar2);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);


    ThermometerConfig configThermometer0 = {
            .subtitle = nullptr,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 37,
            .height = 143,
            .filledColor = CFK_COLOR01,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1.setup(configThermometer0);
    ThermometerConfig configThermometer1 = {
            .subtitle = nullptr,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 39,
            .height = 145,
            .filledColor = CFK_COLOR02,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1copy.setup(configThermometer1);
    ThermometerConfig configThermometer2 = {
            .subtitle = nullptr,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 39,
            .height = 145,
            .filledColor = CFK_COLOR03,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1copy2.setup(configThermometer2);
    ThermometerConfig configThermometer3 = {
            .subtitle = nullptr,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 39,
            .height = 145,
            .filledColor = CFK_COLOR04,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer1copy3.setup(configThermometer3);
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);


}

// This function is a callback of this element slider1.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1_cb(){

    int myValue = slider1.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element slider1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void slider1copy_cb(){

    int myValue = slider1copy.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton1.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1_cb(){

    bool myValue = rectbutton1.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1copy_cb(){

    bool myValue = rectbutton1copy.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton1copy2.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1copy2_cb(){

    bool myValue = rectbutton1copy2.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element rectbutton1copy3.
// You dont need call it. Make sure it is as short and quick as possible.
void rectbutton1copy3_cb(){

    bool myValue = rectbutton1copy3.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element spinbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox1_cb(){

    int myValue = spinbox1.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element spinbox1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void spinbox1copy_cb(){

    int myValue = spinbox1copy.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle1.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1_cb(){

    bool myValue = toggle1.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// This function is a callback of this element toggle1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void toggle1copy_cb(){

    bool myValue = toggle1copy.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
