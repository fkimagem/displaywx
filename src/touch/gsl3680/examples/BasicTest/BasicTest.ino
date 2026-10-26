#include <Wire.h>
#include <displaywx.h>

// Ajuste conforme seus pinos
#define TP_I2C_SDA 7
#define TP_I2C_SCL 8
#define TP_RST 22
#define TP_INT 21


gsl3680_touch touch(TP_I2C_SDA, TP_I2C_SCL, TP_RST, TP_INT);

void setup() {
  Serial.begin(115200);
  while(!Serial) {}
  Serial.println("GSL3680 BasicTest");

  if (!touch.begin(400000)) {
    Serial.println("Touch init failed");
  } else {
    Serial.println("Touch init ok");
  }
}

void loop() {
  uint16_t x, y;
  if (touch.getPoint(x, y)) {
    Serial.print("Touch: ");
    Serial.print(x);
    Serial.print(", ");
    Serial.println(y);
  }
  delay(10);
}
