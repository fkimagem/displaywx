// Include for plugins of chip 0
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>


    /* Project setup:
    * MCU: ESP32S3
    * Display: ST7796
    * Touch: FT6336U
    */
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
const int DISPLAY_W = 480;
const int DISPLAY_H = 320;
const int DISP_FREQUENCY = 27000000;
const int TOUCH_MAP_X0 = 0;
const int TOUCH_MAP_Y0 = 0;
const int TOUCH_MAP_X1 = 320;
const int TOUCH_MAP_Y1 = 480;
const bool TOUCH_SWAP_XY = true;
const bool TOUCH_INVERT_X = false;
const bool TOUCH_INVERT_Y = true;
const int DISP_MOSI = 11;
const int DISP_MISO = -1;
const int DISP_SCLK = 12;
const int DISP_CS = 10;
const int DISP_DC = 8;
const int DISP_RST = 9;
const int TOUCH_SCL = 13;
const int TOUCH_SDA = 14;
const int TOUCH_INT = 18;
const int TOUCH_RST = 17;
const uint8_t rotationScreen = 1; // This value can be changed depending of orientation of your screen
const bool isIPS = true; // Come display can use this as bigEndian flag

// Prototypes for each screen
void screen0();
void loadWidgets();

// Prototypes for callback functions
void numberbox1_cb();
void numberbox1copy_cb();
void btnsum_cb();
void btnminus_cb();

// Create global SPI object
#if defined(CONFIG_IDF_TARGET_ESP32S3)
SPIClass spi_shared(FSPI);
#else
SPIClass spi_shared(HSPI);
#endif
Arduino_DataBus *bus = nullptr;
Arduino_GFX *tft = nullptr;
DisplayWX myDisplay;
// Create global objects. Constructor is: xPos, yPos and indexScreen
Label txtresultado(240, 240, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&txtresultado};
NumberBox numberbox1(215, 75, 0);
NumberBox numberbox1copy(215, 115, 0);
const uint8_t qtdNumberbox = 2;
NumberBox *arrayNumberbox[qtdNumberbox] = {&numberbox1, &numberbox1copy};
float nb0_val = 0; // Global variable that stores the value of the widget numberbox1
float nb1_val = 0; // Global variable that stores the value of the widget numberbox1copy
TextButton btnsum(150, 175, 0);
TextButton btnminus(250, 175, 0);
const uint8_t qtdTextButton = 2;
TextButton *arrayTextButton[qtdTextButton] = {&btnsum, &btnminus};

void setup(){

    Serial.begin(115200);
    // Start SPI object for display
    spi_shared.begin(DISP_SCLK, DISP_MISO, DISP_MOSI);
    bus = new Arduino_HWSPI(DISP_DC, DISP_CS, DISP_SCLK, DISP_MOSI, DISP_MISO, &spi_shared);
    tft = new Arduino_ST7796(bus, DISP_RST, rotationScreen, isIPS, 320, 480);
    tft->begin(DISP_FREQUENCY);
    myDisplay.setDrawObject(tft); // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchFT6336(DISPLAY_W, DISPLAY_H, rotationScreen, TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST);
    //myDisplay.enableTouchLog();
    loadWidgets(); // This function is used to setup with widget individualy
    myDisplay.loadScreen(screen0); // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){
    delay(2000);
}

void screen0(){

    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    //This screen has a/an caixaNumero
    myDisplay.printText("Valor 1", 130, 77, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.printText("Valor 2", 130, 117, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular10pt7b);
    //This screen has a/an caixaNumero
    //This screen has a/an label
    //This screen has a/an textButton
    //This screen has a/an textButton
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){

    LabelConfig configLabel0 = {
            .text = "------",
            .prefix = "",
            .suffix = "",
            .fontFamily = &RobotoRegular10pt7b,
            .datum = TC_DATUM,
            .fontColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE
        };
    txtresultado.setup(configLabel0);
    myDisplay.setLabel(arrayLabel,qtdLabel);


    Numpad::m_backgroundColor = CFK_GREY3;
    Numpad::m_letterColor = CFK_BLACK;
    Numpad::m_keyColor = CFK_GREY13;


    NumberBoxConfig configNumberBox0 = {
            .funcPtr = screen0,
            .callback = numberbox1_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = 10,
            .width = 101,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 0
        };
    numberbox1.setup(configNumberBox0);
    NumberBoxConfig configNumberBox1 = {
            .funcPtr = screen0,
            .callback = numberbox1copy_cb,
            .font = &RobotoRegular10pt7b,
            .startValue = 10,
            .width = 103,
            .height = 25,
            .letterColor = CFK_GREY1,
            .backgroundColor = CFK_WHITE,
            .decimalPlaces = 0
        };
    numberbox1copy.setup(configNumberBox1);
    myDisplay.setNumberbox(arrayNumberbox,qtdNumberbox);


    TextButtonConfig configTextButton0 = {
            .text = "Sum",
            .callback = btnsum_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 79,
            .height = 37,
            .radius = 10,
            .backgroundColor = CFK_GREY11,
            .textColor = CFK_BLACK
        };

    btnsum.setup(configTextButton0);
    TextButtonConfig configTextButton1 = {
            .text = "Sub",
            .callback = btnminus_cb,
            .fontFamily = &RobotoBold10pt7b,
            .width = 79,
            .height = 37,
            .radius = 10,
            .backgroundColor = CFK_GREY11,
            .textColor = CFK_BLACK
        };

    btnminus.setup(configTextButton1);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);


}

// This function is a callback of this element numberbox1.
// You dont need call it. Make sure it is as short and quick as possible.
void numberbox1_cb(){

    nb0_val = numberbox1.getValue();
}
// This function is a callback of this element numberbox1copy.
// You dont need call it. Make sure it is as short and quick as possible.
void numberbox1copy_cb(){

    nb1_val = numberbox1copy.getValue();
}
// This function is a callback of this element btnsum.
// You dont need call it. Make sure it is as short and quick as possible.
void btnsum_cb(){
float result = nb0_val + nb1_val;
txtresultado.setPrefix("Sum:");
txtresultado.setTextFloat(result, 2);
    
}
// This function is a callback of this element btnminus.
// You dont need call it. Make sure it is as short and quick as possible.
void btnminus_cb(){
    float result = nb0_val - nb1_val;
    txtresultado.setPrefix("Sub:");
    txtresultado.setTextFloat(result, 2);
}
