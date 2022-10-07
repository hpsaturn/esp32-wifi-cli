#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32WifiCLI.hpp>

#include "hal.h"

static const uint32_t GPSBaud = 38400;
// The serial connection to the GPS device
HardwareSerial * gps;

bool output_serial;

WiFiUDP Udp;
IPAddress udpip(192, 168, 178, 145);

bool udp_write(uint8_t text, int port) {
  //UDP Write

  if (WiFi.status() == WL_CONNECTED) {
    Udp.stop();
    Udp.begin(WiFi.localIP(), 3332);
    delay(10); //Tweak this
    if (Udp.beginPacket(udpip, port)) {
      Udp.write(text);
      Udp.endPacket();
      return (true);
    } else {
      Udp.stop();
      return (false);
    }
  } else {
    return (false);
  }
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    String out = "";
    while (gps->available()) {
      if(output_serial) Serial.write(gps->read());
      else udp_write(gps->read(), 9000);
    }
  } while (millis() - start < ms);
}

void cli_output(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String enable = operands.first();
  if (enable.equals("on")) output_serial = true;
  else if (enable.equals("off")) output_serial = false;
  else Serial.printf("\r\nbad argument. Please use on or off\r\n");
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.flush();
  delay(1000);

  pinMode(HW_EN, OUTPUT);
  digitalWrite(HW_EN, HIGH);  // step-up on

  delay(100);

  Serial2.begin(GPSBaud, SERIAL_8N1, GPS_RX, GPS_TX);
  gps = &Serial2;
  delay(100);

  wcli.begin();
  wcli.term->add("output", &cli_output, "\t[on|off] GPS serial output.");

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED || (millis() - start < 5000)) wcli.loop();

  Serial.println("== end setup ==");
}

void loop() {
  smartDelay(60);
  wcli.loop();
}
