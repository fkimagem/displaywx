# Display FK
This library is a HAL that allow you to use many widgtes based on display library ArduinoGFX version 1.5.9 from `moononournation`

[https://github.com/moononournation/Arduino_GFX]

## How to Use
1. Download all the files and unzip them.
2. Go to your `libraries` folder in the Arduino IDE (portable or Documents installation).
3. Create a new folder named `DisplayWX` and paste all the unzipped files into that folder.
4. Open your Arduino IDE (if it is already open, close and reopen it), go to **File > Examples > DisplayWX** and check that the examples are listed.

## Display backends

Select one backend in `user_setup.h`:

| Define | Drawing object | Enabled output widgets |
| --- | --- | --- |
| `DISP_DEFAULT` | `Arduino_GFX*` | All; MIPI camera additionally requires ESP32-P4, IDF >= 5.4 and a configured sensor |
| `DISP_PCD8544` | `Adafruit_PCD8544*` | Label, Led, VBar, VAnalog, GaugeSuper, LineChart, Thermometer, Image |
| `DISP_SSD1306` | `Adafruit_SSD1306*` | Same as PCD8544 |
| `DISP_ST7565` | `ST7565_LCD*` from ST7565_LCD | Same as PCD8544 |
| `DISP_U8G2` | `U8G2*` | Image |

Pass the initialized display to `DisplayWX::setDrawObject()`. Touch integration is
enabled only for `DISP_DEFAULT`. `DFK_DISABLE_IMAGE` and `DFK_DISABLE_SD` retain
their existing meanings. The UI task sends the Adafruit monochrome framebuffer
with `display()`; U8g2's buffer/page lifecycle remains managed by the sketch.

Unsupported drawing operations are excluded inside each source file, including
helpers and backend-specific types. For example, CircularBar's drawing methods
do nothing outside Arduino GFX, even when its implementation file is compiled.
This does not enable its automatic registration through `DFK_CIRCULARBAR`.

`python tests/display_backend_contract_test.py` checks conditional branches and
display method names/argument counts against the locally installed sibling
libraries. It does not compile firmware or replace a C++ type check.
