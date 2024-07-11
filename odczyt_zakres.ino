#include "Wire.h"

#define TCAADDR 0x70

#define AS5600_ADDR 0x36
#define ZPOS_HIGH 0x01
#define ZPOS_LOW 0x02
#define MPOS_HIGH 0x03
#define MPOS_LOW 0x04
#define MANG_HIGH 0x05
#define MANG_LOW 0x06

void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  delay(1000);

  Serial.println("\nConfiguring AS5600 encoders through TCA9548A...");

  for (uint8_t channel = 0; channel < 8; channel++) {
    tcaselect(channel);
    Serial.print("Configuring TCA Port #"); Serial.println(channel);

    // Set zero position
    writeAS5600Register(ZPOS_HIGH, 0x00);
    writeAS5600Register(ZPOS_LOW, 0x00);

    // Set maximum position (for full range)
    writeAS5600Register(MPOS_HIGH, 0x0F); // High byte of 4095
    writeAS5600Register(MPOS_LOW, 0xFF);  // Low byte of 4095

    // Set maximum angle (for 360 degrees)
    writeAS5600Register(MANG_HIGH, 0x0F); // High byte of 4095
    writeAS5600Register(MANG_LOW, 0xFF);  // Low byte of 4095

    Serial.println("Configured AS5600 for 360-degree range.");
  }
  Serial.println("\nConfiguration done.");
}

void loop() {
  for (uint8_t channel = 0; channel < 4; channel++) {
    tcaselect(channel);
    Serial.print("Reading TCA Port #"); Serial.println(channel);

    int angle = readAS5600Angle();
    if (angle != -1) {
      Serial.print("AS5600 Angle: ");
      Serial.println(angle);
    } else {
      Serial.println("Error reading AS5600 angle");
    }
    
    
  }
  delay(1000); // Adjust the delay as needed
}

void writeAS5600Register(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

int readAS5600Angle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0E); // High byte of the raw angle
  Wire.endTransmission();
  Wire.requestFrom(AS5600_ADDR, 2);

  int angle = -1;
  if (Wire.available() == 2) {
    angle = Wire.read() << 8;
    angle |= Wire.read();
  }

  return angle;
}
