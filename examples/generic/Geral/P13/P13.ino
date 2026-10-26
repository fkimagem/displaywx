// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define ROBOTO_10 "ROBOTO_10"

// Prototypes for each screen
void screen0();

// Prototypes for callback functions
void touchaarea0_cb();
void cirbtn0_cb();
void btnteste0_cb();
void wifi0_cb();
void slider0_cb();
void textbox0_cb();
void checkbox0_cb();
void checkbox1_cb();
void numberbox0_cb();
void temperature0_cb();
void velocidade1_cb();
void alerta0_cb();
void callbackOfGroup_grupo1();
void callbackOfGroup_grupo2();

// Include for plugins of chip 2
// Include external libraries and files
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#include <SPI.h>

// Create global objects. Constructor is: xPos, yPos and indexScreen
#define TFT_BL 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
CheckBox checkbox0(70, 65, 0);
CheckBox checkbox1(70, 100, 0);
const uint8_t qtdCheckbox = 2;
CheckBox *arrayCheckbox[qtdCheckbox] = {&checkbox0, &checkbox1};
CircleButton cirbtn0(175, 100, 0);
const uint8_t qtdCircleBtn = 1;
CircleButton *arrayCirclebtn[qtdCircleBtn] = {&cirbtn0};
HSlider slider0(263, 55, 0);
const uint8_t qtdHSlider = 1;
HSlider *arrayHslider[qtdHSlider] = {&slider0};
Label mudar0(100, 395, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&mudar0};
Led led0(170, 220, 0);
const uint8_t qtdLed = 1;
Led *arrayLed[qtdLed] = {&led0};
NumberBox numberbox0(285, 185, 0);
const uint8_t qtdNumberbox = 1;
NumberBox *arrayNumberbox[qtdNumberbox] = {&numberbox0};
RadioGroup grupo1(0);
RadioGroup grupo2(0);
const uint8_t qtdRadioGroup = 2;
RadioGroup *arrayRadio[qtdRadioGroup] = {&grupo1, &grupo2};
radio_t radiosOfGroup_grupo1[3] = {{355, 288, CFK_COLOR01, 1},{355, 330, CFK_COLOR14, 2},{355, 370, CFK_COLOR21, 3}};
radio_t radiosOfGroup_grupo2[2] = {{430, 281, CFK_COLOR13, 1},{430, 325, CFK_COLOR16, 2}};
RectButton btnteste0(29, 170, 0);
const uint8_t qtdRectBtn = 1;
RectButton *arrayRectbtn[qtdRectBtn] = {&btnteste0};
SpinBox temperature0(270, 115, 0);
SpinBox velocidade1(480, 110, 0);
const uint8_t qtdSpinbox = 2;
SpinBox *arraySpinbox[qtdSpinbox] = {&temperature0, &velocidade1};
TextBox textbox0(475, 185, 0);
const uint8_t qtdTextbox = 1;
TextBox *arrayTextbox[qtdTextbox] = {&textbox0};
TextButton alerta0(20, 262, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&alerta0};
ToggleButton wifi0(180, 265, 0);
const uint8_t qtdToggleBtn = 1;
ToggleButton *arrayTogglebtn[qtdToggleBtn] = {&wifi0};
TouchArea touchaarea0(741, 397, 0);
const uint8_t qtdTouchArea = 1;
TouchArea *arrayToucharea[qtdTouchArea] = {&touchaarea0};
VAnalog vertalog0(740, 235, 0);
const uint8_t qtdVAnalog = 1;
VAnalog *arrayVanalog[qtdVAnalog] = {&vertalog0};
VBar vertbar0(690, 235, 0);
const uint8_t qtdVBar = 1;
VBar *arrayVbar[qtdVBar] = {&vertbar0};
CircularBar circload0(615, 405, 0);
const uint8_t qtdCircBar = 1;
CircularBar *arrayCircularbar[qtdCircBar] = {&circload0};
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
    #if defined(TFT_BL)
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(); // Initialize the task to read touch and draw
}

void loop(){
    Serial.println("LOOP");
    mudar0.setText("Custom text"); //Use this command to change widget value.
    led0.setState(true);
    vertalog0.setValue(random(0, 100), true); //Use this command to change widget value.
    vertbar0.setValue(random(0, 100)); //Use this command to change widget value.
    circload0.setValue(random(0, 100)); //Use this command to change widget value.
    delay(2000);
}

void screen0(){
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillCircle(29, 34, 24, CFK_COLOR06);
    tft->drawCircle(29, 34, 24, CFK_COLOR01);
    tft->fillRect(737, 14, 52, 46, CFK_COLOR06);
    tft->drawRect(737, 14, 52, 46, CFK_COLOR01);
    tft->fillEllipse(770, 435, 12, 33, CFK_COLOR06);
    tft->drawEllipse(770, 435, 12, 33, CFK_COLOR01);
    tft->fillTriangle(14, 414, 14, 468, 63, 465, CFK_COLOR06);
    tft->drawTriangle(14, 414, 14, 468, 63, 465, CFK_COLOR01);
    //This screen has a/an checkbox
    //This screen has a/an checkbox
    //This screen has a/an circleButton
    //This screen has a/an led
    //This screen has a/an rectButton
    //This screen has a/an slider
    //This screen has a/an spinbox
    //This screen has a/an spinbox
    //This screen has a/an caixaNumero
    //This screen has a/an caixaTexto
    //This screen has a/an textButton
    //This screen has a/an toggleButton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an radiobutton
    //This screen has a/an touchArea
    //This screen has a/an verticalAnalog
    //This screen has a/an rectRange
    //This screen has a/an circularBar
    myDisplay.printText("Custom text", 100, 347, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an label
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
    CheckBoxConfig configCheckbox0 = {
            .callback = checkbox0_cb,
            .size = 21,
            .checkedColor = CFK_COLOR16
        };
    checkbox0.setup(configCheckbox0);
    CheckBoxConfig configCheckbox1 = {
            .callback = checkbox1_cb,
            .size = 21,
            .checkedColor = CFK_COLOR16
        };
    checkbox1.setup(configCheckbox1);
    myDisplay.setCheckbox(arrayCheckbox,qtdCheckbox);
    CircleButtonConfig configCircleButton0 = {
            .callback = cirbtn0_cb,
            .radius = 40,
            .pressedColor = CFK_COLOR16
        };
    cirbtn0.setup(configCircleButton0);
    myDisplay.setCircleButton(arrayCirclebtn,qtdCircleBtn);
    HSliderConfig configHSlider0 = {
            .callback = slider0_cb,
            .subtitle = nullptr,
            .minValue = 0,
            .maxValue = 100,
            .radius = 16,
            .width = 360,
            .pressedColor = CFK_COLOR25,
            .backgroundColor = CFK_WHITE
        };
    slider0.setup(configHSlider0);
    myDisplay.setHSlider(arrayHslider,qtdHSlider);
    LabelConfig configLabel0 = {
            .text = "Custom text",
            .prefix = "pref",
            .suffix = "suf",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR28,
            .backgroundColor = CFK_WHITE
        };
    mudar0.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);
    LedConfig configLed0 = {
            .radius = 16,
            .colorOn = CFK_COLOR18
        };
    led0.setup(configLed0);
    myDisplay.setLed(arrayLed,qtdLed);
    NumberBoxConfig configNumberBox0 = {
            .funcPtr = screen0,
            .callback = numberbox0_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = 123,
            .width = 101,
            .height = 20,
            .letterColor = CFK_COLOR24,
            .backgroundColor = CFK_WHITE
        };
    numberbox0.setup(configNumberBox0);
    myDisplay.setNumberbox(arrayNumberbox,qtdNumberbox);
    RadioGroupConfig configRadioGroup0 = {
            .buttons = radiosOfGroup_grupo1,
            .callback = callbackOfGroup_grupo1,
            .radius = 10,
            .group = 1,
            .amount = 3,
            .defaultClickedId = 1
        };
    grupo1.setup(configRadioGroup0);
    RadioGroupConfig configRadioGroup1 = {
            .buttons = radiosOfGroup_grupo2,
            .callback = callbackOfGroup_grupo2,
            .radius = 10,
            .group = 2,
            .amount = 2,
            .defaultClickedId = 1
        };
    grupo2.setup(configRadioGroup1);
    myDisplay.setRadioGroup(arrayRadio,qtdRadioGroup);
    RectButtonConfig configRectButton0 = {
            .callback = btnteste0_cb,
            .width = 88,
            .height = 41,
            .pressedColor = CFK_COLOR25
        };btnteste0.setup(configRectButton0);
    myDisplay.setRectButton(arrayRectbtn,qtdRectBtn);
    SpinBoxConfig configSpinBox0 = {
            .callback = temperature0_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 138,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR25,
            .textColor = CFK_BLACK
        };
    temperature0.setup(configSpinBox0);
    SpinBoxConfig configSpinBox1 = {
            .callback = velocidade1_cb,
            .minValue = 0,
            .maxValue = 100,
            .startValue = 50,
            .width = 138,
            .height = 51,
            .step = 1,
            .color = CFK_COLOR24,
            .textColor = CFK_WHITE
        };
    velocidade1.setup(configSpinBox1);
    myDisplay.setSpinbox(arraySpinbox,qtdSpinbox);
    TextBoxConfig configTextBox0 = {
            .startValue = "YOUR TEXT HERE...",
            .funcPtr = screen0,
            .cb = textbox0_cb,
            .font = &RobotoRegular10pt7b,
            .width = 142,
            .height = 25,
            .letterColor = CFK_COLOR24,
            .backgroundColor = CFK_WHITE
        };
    textbox0.setup(configTextBox0);
    myDisplay.setTextbox(arrayTextbox,qtdTextbox);
    TextButtonConfig configTextButton0 = {
            .text = "alert",
            .callback = alerta0_cb,
            .width = 129,
            .height = 47,
            .radius = 10,
            .backgroundColor = CFK_COLOR31,
            .textColor = CFK_WHITE
        };
    alerta0.setup(configTextButton0);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);
    ToggleButtonConfig configToggle0 = {
            .callback = wifi0_cb,
            .width = 94,
            .height = 47,
            .pressedColor = CFK_COLOR31
        };
    wifi0.setup(configToggle0);
    myDisplay.setToggle(arrayTogglebtn,qtdToggleBtn);
    TouchAreaConfig configTouchArea0 = {
            .callback = touchaarea0_cb,
            .width = 46,
            .height = 71
        };
    touchaarea0.setup(configTouchArea0);
    myDisplay.setTouchArea(arrayToucharea,qtdTouchArea);
    VerticalAnalogConfig configVAnalog0 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 41,
            .height = 151,
            .arrowColor = CFK_COLOR01,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .borderColor = CFK_BLACK,
            .steps = 10
        };
    vertalog0.setup(configVAnalog0);
    myDisplay.setVAnalog(arrayVanalog,qtdVAnalog);
    VerticalBarConfig configVBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .width = 31,
            .height = 152,
            .filledColor = CFK_COLOR01
        };
    vertbar0.setup(configVBar0);
    myDisplay.setVBar(arrayVbar,qtdVBar);
    CircularBarConfig configCirculaBar0 = {
            .minValue = 0,
            .maxValue = 100,
            .radius = 40,
            .startAngle = 0,
            .endAngle = 360,
            .color = CFK_COLOR04,
            .backgroundColor = CFK_GREY12,
            .textColor = CFK_BLACK,
            .thickness = 20,
            .showValue = true,
            .inverted = false
        };
    circload0.setup(configCirculaBar0);
    myDisplay.setCircularBar(arrayCircularbar,qtdCircBar);
}

// This function is a callback of this element checkbox0.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox0_cb(){
    bool myValue = checkbox0.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}
// This function is a callback of this element checkbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void checkbox1_cb(){
    bool myValue = checkbox1.getStatus();
    Serial.print("New value for checkbox is: ");Serial.println(myValue);
}
// This function is a callback of this element cirbtn0.
// You dont need call it. Make sure it is as short and quick as possible.
void cirbtn0_cb(){
    bool myValue = cirbtn0.getStatus();
    Serial.print("New value for circlebutton is: ");Serial.println(myValue);
}
// This function is a callback of this element slider0.
// You dont need call it. Make sure it is as short and quick as possible.
void slider0_cb(){
    int myValue = slider0.getValue();
    Serial.print("New value for slider is: ");Serial.println(myValue);
}
// This function is a callback of this element numberbox0.
// You dont need call it. Make sure it is as short and quick as possible.
void numberbox0_cb(){
    float myValue = numberbox0.getValue();
    Serial.print("New value for numberbox is: ");Serial.println(myValue);
}
// This function is a callback of radio buttons of group 1.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo1(){
    uint16_t selectedOption = grupo1.getSelected();
    Serial.print("New value for radiogroup is: ");Serial.println(selectedOption);
}
// This function is a callback of radio buttons of group 2.
// You dont need call it. Make sure it is as short and quick as possible.
void callbackOfGroup_grupo2(){
    uint16_t selectedOption = grupo2.getSelected();
    Serial.print("New value for radiogroup is: ");Serial.println(selectedOption);
}
// This function is a callback of this element btnteste0.
// You dont need call it. Make sure it is as short and quick as possible.
void btnteste0_cb(){
    bool myValue = btnteste0.getStatus();
    Serial.print("New value for rectbutton is: ");Serial.println(myValue);
}
// This function is a callback of this element temperature0.
// You dont need call it. Make sure it is as short and quick as possible.
void temperature0_cb(){
    int myValue = temperature0.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element velocidade1.
// You dont need call it. Make sure it is as short and quick as possible.
void velocidade1_cb(){
    int myValue = velocidade1.getValue();
    Serial.print("New value for spinbox is: ");Serial.println(myValue);
}
// This function is a callback of this element textbox0.
// You dont need call it. Make sure it is as short and quick as possible.
void textbox0_cb(){
    const char* myValue = textbox0.getValue();
    Serial.print("New value for textbox is: ");Serial.println(myValue);
}
// This function is a callback of this element alerta0.
// You dont need call it. Make sure it is as short and quick as possible.
void alerta0_cb(){
    Serial.print("Clicked on: ");Serial.println("alerta0_cb");
}
// This function is a callback of this element wifi0.
// You dont need call it. Make sure it is as short and quick as possible.
void wifi0_cb(){
    bool myValue = wifi0.getStatus();
    Serial.print("New value for toggle is: ");Serial.println(myValue);
}
// This function is a callback of this element touchaarea0.
// You dont need call it. Make sure it is as short and quick as possible.
void touchaarea0_cb(){
    Serial.println("AreaTouch was clicked");
}
