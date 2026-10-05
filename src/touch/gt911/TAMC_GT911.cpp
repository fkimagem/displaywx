#include "Arduino.h"
#include "TAMC_GT911.h"
#include <Wire.h>
#include "../share_i2c.h"
#if defined(HAS_CAMERA_MIPI)
#define Wire TouchI2c
#endif

//#define LOG_GT911 1
//#define OLD_GT911 1

TAMC_GT911::TAMC_GT911(uint8_t _sda, uint8_t _scl, uint8_t _int, uint8_t _rst, uint16_t _width, uint16_t _height) :
  pinSda(_sda), pinScl(_scl), pinInt(_int), pinRst(_rst), width(_width), height(_height) {

}

void TAMC_GT911::begin(uint8_t _addr) {
  addr = _addr;
#if defined(HAS_CAMERA_MIPI)
  if (!Wire.begin(pinSda, pinScl)) {
    return;
  }
#else
  Wire.begin(pinSda, pinScl);
#endif
  reset();
}
void TAMC_GT911::reset() {
  if(pinInt > 0){pinMode(pinInt, OUTPUT);}
  if(pinRst > 0){pinMode(pinRst, OUTPUT);}
  if(pinInt > 0){digitalWrite(pinInt, 0);}
  if(pinRst > 0){digitalWrite(pinRst, 0);}
  delay(10);
  if(pinInt > 0){digitalWrite(pinInt, addr==GT911_ADDR2);}
  delay(1);
  if(pinRst > 0){digitalWrite(pinRst, 1);}
  delay(5);
  if(pinInt > 0){digitalWrite(pinInt, 0);}
  delay(50);
  if(pinInt > 0){pinMode(pinInt, INPUT);}
  // attachInterrupt(pinInt, TAMC_GT911::onInterrupt, RISING);
  delay(50);
  if (!readBlockData(configBuf, GT911_CONFIG_START, GT911_CONFIG_SIZE)) {
    Serial.println("GT911 config read failed");
    return;
  }
  m_savedX = configBuf[GT911_X_OUTPUT_MAX_LOW - GT911_CONFIG_START] |
                 (configBuf[GT911_X_OUTPUT_MAX_HIGH - GT911_CONFIG_START] << 8);
  m_savedY = configBuf[GT911_Y_OUTPUT_MAX_LOW - GT911_CONFIG_START] |
                 (configBuf[GT911_Y_OUTPUT_MAX_HIGH - GT911_CONFIG_START] << 8);

  Serial.print("GT911 config resolution: ");
  Serial.print(m_savedX);
  Serial.print("x");
  Serial.println(m_savedY);


  setResolution(width, height);
}
void TAMC_GT911::calculateChecksum() {
  uint8_t checksum;
  for (uint8_t i=0; i<GT911_CONFIG_SIZE; i++) {
    checksum += configBuf[i];
  }
  checksum = (~checksum) + 1;
  configBuf[GT911_CONFIG_CHKSUM - GT911_CONFIG_START] = checksum;
}
// void ARDUINO_ISR_ATTR TAMC_GT911::onInterrupt() {
//   read();
//   TAMC_GT911::onRead();
// }
void TAMC_GT911::reflashConfig() {
  calculateChecksum();
  writeByteData(GT911_CONFIG_CHKSUM, configBuf[GT911_CONFIG_CHKSUM-GT911_CONFIG_START]);
  writeByteData(GT911_CONFIG_FRESH, 1);
}
void TAMC_GT911::setRotation(uint8_t rot) {
  rotation = rot;
}
void TAMC_GT911::setResolution(uint16_t _width, uint16_t _height) {
  Serial.printf("Update resolution: %i x %i\n", _width, _height);
  configBuf[GT911_X_OUTPUT_MAX_LOW - GT911_CONFIG_START] = lowByte(_width);
  configBuf[GT911_X_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = highByte(_width);
  configBuf[GT911_Y_OUTPUT_MAX_LOW - GT911_CONFIG_START] = lowByte(_height);
  configBuf[GT911_Y_OUTPUT_MAX_HIGH - GT911_CONFIG_START] = highByte(_height);
  reflashConfig();
}
// void TAMC_GT911::setOnRead(void (*isr)()) {
//   onRead = isr;
// }
void TAMC_GT911::read(void) {
  #if defined(LOG_GT911)
    //Serial.println("TAMC_GT911::read");
  #endif
  uint8_t data[7];
  uint8_t id;
  uint16_t x, y, size;

  int pointInfoRaw = readByteData(GT911_POINT_INFO);
  if (pointInfoRaw < 0) {
    isTouched = false;
    touches = 0;
    return;
  }
  uint8_t pointInfo = (uint8_t)pointInfoRaw;
  uint8_t bufferStatus = pointInfo >> 7 & 1;
  uint8_t proximityValid = pointInfo >> 5 & 1;
  uint8_t haveKey = pointInfo >> 4 & 1;
  isLargeDetect = pointInfo >> 6 & 1;
  touches = pointInfo & 0xF;
  #if defined(LOG_GT911)
  //Serial.print("bufferStatus: ");Serial.println(bufferStatus);
  //Serial.print("largeDetect: ");Serial.println(isLargeDetect);
  //Serial.print("proximityValid: ");Serial.println(proximityValid);
  //Serial.print("haveKey: ");Serial.println(haveKey);
  //Serial.print("touches: ");Serial.println(touches);
  #endif
  isTouched = touches > 0;
  if (bufferStatus == 1 && isTouched) {
    for (uint8_t i=0; i<touches; i++) {
      if (!readBlockData(data, GT911_POINT_1 + i * 8, 7)) {
        isTouched = false;
        touches = 0;
        return;
      }
      points[i] = readPoint(data);
    }
  }
  writeByteData(GT911_POINT_INFO, 0);
}
TP_Point TAMC_GT911::readPoint(uint8_t *data) {
  uint16_t temp;
  uint8_t id = data[0];
  uint16_t x = data[1] + (data[2] << 8);
  uint16_t y = data[3] + (data[4] << 8);
  uint16_t size = data[5] + (data[6] << 8);

  // Print x,y
  #if defined(LOG_GT911)
  Serial.print(" x: ");Serial.print(x);
  Serial.print(" y: ");Serial.print(y);

  // print savedX, savedY
  Serial.print(" savedX: ");Serial.print(m_savedX);
  Serial.print(" savedY: ");Serial.print(m_savedY);

  // print width, height
  Serial.print(" width: ");Serial.print(width);
  Serial.print(" height: ");Serial.println(height);
  Serial.println(' ');
  #endif

  // Corrigir resolução antes da rotação
  x = map(x, 0, m_savedX, 0, width);   // Ajustar para 800
  y = map(y, 0, m_savedY, 0, height);  // Ajustar para 480

  // print x,y
  #if defined(LOG_GT911)
  Serial.print(" x: ");Serial.print(x);
  Serial.print(" y: ");Serial.println(y);
  #endif


  /*switch (rotation){
    case ROTATION_NORMAL:
      x = width - x;
      y = height - y;
      break;
    case ROTATION_LEFT:
      temp = x;
      x = width - y;
      y = temp;
      break;
    case ROTATION_INVERTED:
      x = x;
      y = y;
      break;
    case ROTATION_RIGHT:
      temp = x;
      x = y;
      y = height - temp;
      break;
    default:
      break;
  }*/
  return TP_Point(id, x, y, size);
}
void TAMC_GT911::writeByteData(uint16_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(highByte(reg));
  Wire.write(lowByte(reg));
  Wire.write(val);
  Wire.endTransmission();
}
int TAMC_GT911::readByteData(uint16_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(highByte(reg));
  Wire.write(lowByte(reg));
  if (Wire.endTransmission() != 0) {
    return -1;
  }
  if (Wire.requestFrom(addr, (uint8_t)1) != 1) {
    return -1;
  }
  int value = Wire.read();
  if (value < 0) {
    return -1;
  }
  return value;
}
void TAMC_GT911::writeBlockData(uint16_t reg, uint8_t *val, uint8_t size) {
  Wire.beginTransmission(addr);
  Wire.write(highByte(reg));
  Wire.write(lowByte(reg));
  // Wire.write(val, size);
  for (uint8_t i=0; i<size; i++) {
    Wire.write(val[i]);
  }
  Wire.endTransmission();
}
bool TAMC_GT911::readBlockData(uint8_t *buf, uint16_t reg, uint8_t size) {
  #if defined(OLD_GT911)
  Wire.beginTransmission(addr);
  Wire.write(highByte(reg));
  Wire.write(lowByte(reg));
  if (Wire.endTransmission() != 0) {
    return false;
  }
  if (Wire.requestFrom(addr, size) != size) {
    return false;
  }
  for (uint8_t i=0; i<size; i++) {
    int value = Wire.read();
    if (value < 0) {
      return false;
    }
    buf[i] = (uint8_t)value;
  }
  return true;
 #else
 const uint8_t CHUNK_SIZE = 64;  // máximo seguro para ESP32-P4 I²C FIFO
  uint8_t remaining = size;
  uint16_t currentReg = reg;
  uint8_t index = 0;

  while (remaining > 0) {
    uint8_t chunk = remaining > CHUNK_SIZE ? CHUNK_SIZE : remaining;

    Wire.beginTransmission(addr);
    Wire.write(highByte(currentReg));
    Wire.write(lowByte(currentReg));
    if (Wire.endTransmission(false) != 0) {
      return false;
    }

    if (Wire.requestFrom((int)addr, (int)chunk) != chunk) {
      return false;
    }
    for (uint8_t i = 0; i < chunk; i++) {
      int value = Wire.read();
      if (value < 0) {
        return false;
      }
      buf[index++] = (uint8_t)value;
    }

    currentReg += chunk;
    remaining -= chunk;
  }
  return true;
  #endif
}
TP_Point::TP_Point(void) {
  id = x = y = size = 0;
}
TP_Point::TP_Point(uint8_t _id, uint16_t _x, uint16_t _y, uint16_t _size) {
  id = _id;
  x = _x;
  y = _y;
  size = _size;
}
bool TP_Point::operator==(TP_Point point) {
  return ((point.x == x) && (point.y == y) && (point.size == size));
}
bool TP_Point::operator!=(TP_Point point) {
  return ((point.x != x) || (point.y != y) || (point.size != size));
}
