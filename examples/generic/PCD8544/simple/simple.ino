/*********************************************************************
This is an example sketch for our Monochrome Nokia 5110 LCD Displays

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/338

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <displaywx.h>

#define PIN_SCL 11
#define PIN_DIN 12
#define PIN_DC 14
#define PIN_CS 8
#define PIN_RST 18

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
// Adafruit_PCD8544 display = Adafruit_PCD8544(PIN_SCL, PIN_DIN, PIN_DC, PIN_CS, PIN_RST);
Adafruit_PCD8544 *display = nullptr;
DisplayWX myDisplay;

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16

void loadWidgets()
{
}

void screen0()
{
  display->clearDisplay();
  WidgetBase::backgroundColor = CFK_GREY6;
  // display->fillRect(0, 0, 20, 20, CFK_BLACK);
  // display->drawRect(10, 10, 20, 20, CFK_BLACK);
  int posX = 20, posY = 20;

  display->drawFastVLine(posX, posY, 80, CFK_BLACK);
  display->drawFastHLine(posX, posY, 80, CFK_BLACK);

  /*display->setCursor(posX,posY);
  display->setFont(&RobotoRegular10pt7b);
  display->println("Teste");

  display->setCursor(posX,posY);
  display->setFont((GFXfont *)0);
  display->println("Teste2");*/

  myDisplay.printText("Teste", posX, posY, TL_DATUM, CFK_BLACK, CFK_WHITE, &RobotoRegular5pt7b);
  myDisplay.printText("Teste2", posX, posY, BR_DATUM, CFK_BLACK, CFK_WHITE, nullptr);

  display->display();
  myDisplay.drawWidgetsOnScreen(0);
}

void setup()
{
  Serial.begin(115200);
  display = new Adafruit_PCD8544(PIN_SCL, PIN_DIN, PIN_DC, PIN_CS, PIN_RST);
  display->begin();
  display->setContrast(75);
  display->clearDisplay();
  WidgetBase::objTFT = display;     // Reference to object to draw on screen
  loadWidgets();                    // This function is used to setup with widget individualy
  WidgetBase::loadScreen = screen0; // Use this line to change between screens
  myDisplay.createTask(true, 5);    // Initialize the task to read touch and draw
}

void loop()
{
}