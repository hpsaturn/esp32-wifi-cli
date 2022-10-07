#include <Arduino.h>
#include "hal.h"


static const uint32_t GPSBaud = 38400;
// The serial connection to the GPS device
HardwareSerial * gps;

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    String out = "";
    while (gps->available()) {
      Serial.write(gps->read());
    }
  } while (millis() - start < ms);
}


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(38400);

  pinMode(HW_EN, OUTPUT);
  digitalWrite(HW_EN, HIGH);  // step-up on

  delay(100);

  Serial2.begin(GPSBaud, SERIAL_8N1, GPS_RX, GPS_TX);
  gps = &Serial2;

  delay(100);
}


void loop() {
  smartDelay(1000);
}
