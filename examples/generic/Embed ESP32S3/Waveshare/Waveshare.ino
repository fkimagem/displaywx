// #define DFK_SD
// Defines for font and files
#define FORMAT_SPIFFS_IF_FAILED false
#define DISPLAY_W 800
#define DISPLAY_H 480
#define TC_SCL	9
#define TC_SDA	8
#define TC_INT	-1
#define TC_RST	-1
#define TOUCH_ROTATION	ROTATION_NORMAL
#define TOUCH_MAP_X0	0
#define TOUCH_MAP_X1	800
#define TOUCH_MAP_Y0	0
#define TOUCH_MAP_Y1	480
#define TOUCH_SWAP_XY	false
#define TOUCH_INVERT_X	false
#define TOUCH_INVERT_Y	false



//#define TEST_I2C
#define TEST_WIDGETS
#define TEST_IO


#define EXAMPLE_LCD_NAME                        ST7262 // LCD model name
#define EXAMPLE_LCD_WIDTH                       (800) // LCD width in pixels
#define EXAMPLE_LCD_HEIGHT                      (480) // LCD height in pixels
#define EXAMPLE_LCD_COLOR_BITS                  (24)  // Color depth in bits
#define EXAMPLE_LCD_RGB_DATA_WIDTH              (16)  // Width of RGB data
#define EXAMPLE_LCD_RGB_COLOR_BITS          (16)    // |      24      |      16       |

#define EXAMPLE_LCD_RGB_TIMING_FREQ_HZ          (16 * 1000 * 1000) // RGB timing frequency
#define EXAMPLE_LCD_RGB_TIMING_HPW              (4)   // Horizontal pulse width
#define EXAMPLE_LCD_RGB_TIMING_HBP              (8)   // Horizontal back porch
#define EXAMPLE_LCD_RGB_TIMING_HFP              (8)   // Horizontal front porch
#define EXAMPLE_LCD_RGB_TIMING_VPW              (4)   // Vertical pulse width
#define EXAMPLE_LCD_RGB_TIMING_VBP              (8)   // Vertical back porch
#define EXAMPLE_LCD_RGB_TIMING_VFP              (8)   // Vertical front porch
#define EXAMPLE_LCD_RGB_BOUNCE_BUFFER_SIZE  (EXAMPLE_LCD_WIDTH * 10)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your board spec ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_RGB_IO_DISP            (-1)  // RGB display pin number
#define EXAMPLE_LCD_RGB_IO_VSYNC           (3)   // VSYNC pin number
#define EXAMPLE_LCD_RGB_IO_HSYNC           (46)  // HSYNC pin number
#define EXAMPLE_LCD_RGB_IO_DE              (5)   // Data enable pin number
#define EXAMPLE_LCD_RGB_IO_PCLK            (7)   // Pixel clock pin number
#define EXAMPLE_LCD_RGB_IO_DATA0           (14)  // RGB data pin 0
#define EXAMPLE_LCD_RGB_IO_DATA1           (38)  // RGB data pin 1
#define EXAMPLE_LCD_RGB_IO_DATA2           (18)  // RGB data pin 2
#define EXAMPLE_LCD_RGB_IO_DATA3           (17)  // RGB data pin 3
#define EXAMPLE_LCD_RGB_IO_DATA4           (10)  // RGB data pin 4
#define EXAMPLE_LCD_RGB_IO_DATA5           (39)  // RGB data pin 5
#define EXAMPLE_LCD_RGB_IO_DATA6           (0)   // RGB data pin 6
#define EXAMPLE_LCD_RGB_IO_DATA7           (45)  // RGB data pin 7
#if EXAMPLE_LCD_RGB_DATA_WIDTH > 8
#define EXAMPLE_LCD_RGB_IO_DATA8           (48)  // RGB data pin 8
#define EXAMPLE_LCD_RGB_IO_DATA9           (47)  // RGB data pin 9
#define EXAMPLE_LCD_RGB_IO_DATA10          (21)  // RGB data pin 10
#define EXAMPLE_LCD_RGB_IO_DATA11          (1)   // RGB data pin 11
#define EXAMPLE_LCD_RGB_IO_DATA12          (2)   // RGB data pin 12
#define EXAMPLE_LCD_RGB_IO_DATA13          (42)  // RGB data pin 13
#define EXAMPLE_LCD_RGB_IO_DATA14          (41)  // RGB data pin 14
#define EXAMPLE_LCD_RGB_IO_DATA15          (40)  // RGB data pin 15
#endif
#define EXAMPLE_LCD_RST_IO                 (-1)  // Reset pin number
#define EXAMPLE_LCD_BL_IO            (-1)  // Backlight pin number
#define EXAMPLE_LCD_BL_ON_LEVEL           (1)   // Backlight ON level
#define EXAMPLE_LCD_BL_OFF_LEVEL !EXAMPLE_LCD_BL_ON_LEVEL // Backlight OFF level

// Prototypes for each screen
void screen0();
void loadWidgets();
void textbutton_cb();
void scan_i2c();
void test_widgets();
void test_io();



// Prototypes for callback functions

// Include for plugins of chip 1
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
#define VSPI FSPI
#endif
// Include external libraries and files
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <displaywx.h>
#if defined(TEST_IO)
#include <esp_io_expander.hpp>
#endif

#if defined(TEST_IO)
#define EXAMPLE_I2C_ADDR    (ESP_IO_EXPANDER_I2C_CH422G_ADDRESS)
#define EXAMPLE_I2C_SDA_PIN 8         // I2C data line pins
#define EXAMPLE_I2C_SCL_PIN 9         // I2C clock line pin


#define DI0 0                     // Digital Input 0
#define DI0_mask 1ULL<<DI0       // Mask for Digital Input 0
#define DI1 5                     // Digital Input 1
#define DI1_mask 1ULL<<DI1       // Mask for Digital Input 1

#define DO0 8                     // Digital Output 0
#define DO0_mask 1ULL<<DO0       // Mask for Digital Output 0
#define DO1 9                     // Digital Output 1
#define DO1_mask 1ULL<<DO1       // Mask for Digital Output 1

#endif

// Create global objects. Constructor is: xPos, yPos and indexScreen
// Create global objects. Constructor is: xPos, yPos and indexScreen
#define DISP_LED 2
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *tft = nullptr;
uint8_t rotationScreen = 1;
DisplayWX myDisplay;
GaugeSuper widget(185, 420, 0);
const uint8_t qtdGauge = 1;
GaugeSuper *arrayGauge[qtdGauge] = {&widget};
const uint8_t qtdIntervalG0 = 4;
int range0[qtdIntervalG0] = {0,25,50,75};
uint16_t colors0[qtdIntervalG0] = {CFK_COLOR17,CFK_COLOR11,CFK_COLOR04,CFK_COLOR03};
Label term1(585, 390, 0);
const uint8_t qtdLabel = 1;
Label *arrayLabel[qtdLabel] = {&term1};
TextButton textbutton(325, 25, 0);
const uint8_t qtdTextButton = 1;
TextButton *arrayTextButton[qtdTextButton] = {&textbutton};
Thermometer thermometer(695, 145, 0);
const uint8_t qtdThermometer = 1;
Thermometer *arrayThermometer[qtdThermometer] = {&thermometer};
int count = 0;
#if defined(TEST_IO)
esp_expander::CH422G *expander = NULL;
#endif
//ESP_IOExpander *expander = NULL;
void setup(){
    Serial.begin(115200);
    rgbpanel = new Arduino_ESP32RGBPanel(
    EXAMPLE_LCD_RGB_IO_DE, EXAMPLE_LCD_RGB_IO_VSYNC, EXAMPLE_LCD_RGB_IO_HSYNC, EXAMPLE_LCD_RGB_IO_PCLK,
    EXAMPLE_LCD_RGB_IO_DATA0, EXAMPLE_LCD_RGB_IO_DATA1, EXAMPLE_LCD_RGB_IO_DATA2, EXAMPLE_LCD_RGB_IO_DATA3, EXAMPLE_LCD_RGB_IO_DATA4,
    EXAMPLE_LCD_RGB_IO_DATA5, EXAMPLE_LCD_RGB_IO_DATA6, EXAMPLE_LCD_RGB_IO_DATA7, EXAMPLE_LCD_RGB_IO_DATA8, EXAMPLE_LCD_RGB_IO_DATA9, EXAMPLE_LCD_RGB_IO_DATA10,
    EXAMPLE_LCD_RGB_IO_DATA11, EXAMPLE_LCD_RGB_IO_DATA12, EXAMPLE_LCD_RGB_IO_DATA13, EXAMPLE_LCD_RGB_IO_DATA14, EXAMPLE_LCD_RGB_IO_DATA15,
    0, EXAMPLE_LCD_RGB_TIMING_HFP, EXAMPLE_LCD_RGB_TIMING_HPW, EXAMPLE_LCD_RGB_TIMING_HBP,
    0, EXAMPLE_LCD_RGB_TIMING_VFP, EXAMPLE_LCD_RGB_TIMING_VPW, EXAMPLE_LCD_RGB_TIMING_VBP,
    1, EXAMPLE_LCD_RGB_TIMING_FREQ_HZ, false,
    0, 0, EXAMPLE_LCD_RGB_BOUNCE_BUFFER_SIZE);
    tft = new Arduino_RGB_Display(DISPLAY_W, DISPLAY_H, rgbpanel, 0 , true);
    tft->begin();
    #if defined(DISP_LED)
    pinMode(DISP_LED, OUTPUT);
    digitalWrite(DISP_LED, HIGH);
    #endif
    WidgetBase::objTFT = tft; // Reference to object to draw on screen
    // Setup touch
    myDisplay.setTouchCorners(TOUCH_MAP_X0, TOUCH_MAP_X1, TOUCH_MAP_Y0, TOUCH_MAP_Y1);
    myDisplay.setInvertAxis(TOUCH_INVERT_X, TOUCH_INVERT_Y);
    myDisplay.setSwapAxis(TOUCH_SWAP_XY);
    myDisplay.startTouchGT911(DISPLAY_W, DISPLAY_H, rotationScreen, TC_SDA, TC_SCL, TC_INT, TC_RST);
    myDisplay.enableTouchLog();
    myDisplay.setupAutoClick(2000, 400, 50); // Setup auto click
    myDisplay.startAutoClick(); // Start auto click
    loadWidgets(); // This function is used to setup with widget individualy
    WidgetBase::loadScreen = screen0; // Use this line to change between screens
    myDisplay.createTask(false, 3); // Initialize the task to read touch and draw
}

void loop(){
    #if defined(TEST_WIDGETS)
    test_widgets();
    #endif

    #if defined(TEST_I2C)
    scan_i2c();
    #endif

    #if defined(TEST_IO)
    test_io();
    #endif
}

void screen0(){
    tft->fillScreen(CFK_WHITE);
    WidgetBase::backgroundColor = CFK_WHITE;
    tft->fillRect(155, 80, 49, 53, CFK_COLOR01);
    tft->drawRect(155, 80, 49, 53, CFK_COLOR01);
    tft->fillCircle(345, 210, 40, CFK_COLOR15);
    tft->drawCircle(345, 210, 40, CFK_COLOR01);
    myDisplay.printText("Custom text", 450, 307, TL_DATUM, CFK_COLOR01, CFK_WHITE, &RobotoRegular10pt7b);
    myDisplay.drawWidgetsOnScreen(0);
}

// Configure each widgtes to be used
void loadWidgets(){
  GaugeConfig configGauge0 = {
            .title = "Title",
            .intervals = range0,
            .colors = colors0,
            .fontFamily = &RobotoBold10pt7b,
            .minValue = 0,
            .maxValue = 100,
            .width = 261,
            .borderColor = CFK_COLOR23,
            .textColor = CFK_BLACK,
            .backgroundColor = CFK_WHITE,
            .titleColor = CFK_BLACK,
            .needleColor = CFK_RED,
            .markersColor = CFK_BLACK,
            .amountIntervals = qtdIntervalG0,
            .showLabels = true
        };
    widget.setup(configGauge0);
    myDisplay.setGauge(arrayGauge,qtdGauge);

    LabelConfig configLabel0 = {
            .text = "99",
            .prefix = "",
            .suffix = "C",
            .fontFamily = &G7_Segment7_S520pt7b,
            .datum = TL_DATUM,
            .fontColor = CFK_COLOR01,
            .backgroundColor = CFK_WHITE
        };
    term1.setup(configLabel0);
    term1.setDecimalPlaces(0);
    myDisplay.setLabel(arrayLabel,qtdLabel);

    TextButtonConfig configTextButton0 = {
            .text = "Button",
            .callback = textbutton_cb,
            .width = 151,
            .height = 50,
            .radius = 10,
            .backgroundColor = CFK_COLOR01,
            .textColor = CFK_WHITE
        };

    textbutton.setup(configTextButton0);
    myDisplay.setTextButton(arrayTextButton,qtdTextButton);

    ThermometerConfig configThermometer0 = {
            .subtitle = &term1,
            .unit = "C",
            .minValue = 0,
            .maxValue = 100,
            .width = 27,
            .height = 227,
            .filledColor = CFK_COLOR01,
            .backgroundColor = CFK_WHITE,
            .markColor = CFK_BLACK,
            .decimalPlaces = 1
        };
    thermometer.setup(configThermometer0);
    myDisplay.setThermometer(arrayThermometer,qtdThermometer);
}
void textbutton_cb(){
    Serial.print("Clicked on: ");Serial.println("textbutton_cb");
}

void test_widgets(){
    float anglo = count * 0.0174533;
    int valor = sin(anglo) * 50.0 + 50.0;
    widget.setValue(100-valor); //Use this command to change widget value.
    thermometer.setValue(valor); //Use this command to change widget value.
    delay(100);
    count++;
}

void scan_i2c(){
  #if defined(TEST_I2C)
    byte error, address; // Store error content, address value
  int nDevices = 0; // Number of recording devices

  printf("Scanning for I2C devices ...\r\n");
  for (address = 0x01; address < 0x7f; address++) { // Start scanning all devices from 0x01 to 0x7f
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) { // Determine whether the device exists, and if so, output the address
      printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) { // Determine the number of devices. If it is 0, there is no device.
    printf("No I2C devices found.\r\n");
  }
  #endif
}

void test_io(){
  #if defined(TEST_IO)
  Serial.println("Initialize IO expander"); // Print initialization message
    /* Initialize IO expander */
    //expander = new esp_expander::CH422G(EXAMPLE_I2C_SCL_PIN, EXAMPLE_I2C_SDA_PIN, EXAMPLE_I2C_ADDR);
    expander = new esp_expander::CH422G(EXAMPLE_I2C_SCL_PIN, EXAMPLE_I2C_SDA_PIN, EXAMPLE_I2C_ADDR);
    // Create an instance of the IO expander
    expander->init(); // Initialize the expander
    expander->begin(); // Begin expander operation

    Serial.println("Set the OC pin to push-pull output mode.");
    expander->enableOC_PushPull();

    Serial.println("Set the IO0-7 pin to input mode.");
    expander->enableAllIO_Input();

    // Set output pins to default high
    //expander->digitalWrite(DO0, 1); // Set DO0 high
    //->digitalWrite(DO1, 1); // Set DO1 high

    /*Serial.println("IO test example start"); // Print start message
    while(1)
    {
        bool leituraDI0 = expander->digitalRead(DI0);
        bool leituraDI1 = expander->digitalRead(DI1);

        expander->digitalWrite(DO0, leituraDI0); // Set DO0 high
        expander->digitalWrite(DO1, leituraDI1); // Set DO1 low
        delay(100);
    }*/
    #endif
}