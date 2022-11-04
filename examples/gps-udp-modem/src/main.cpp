#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESP32WifiCLI.hpp>
#include "hal.h"

// The serial connection to the GPS device
HardwareSerial * gps;
// GPS speed configuration (sometimes is 9600)
static const uint32_t GPSBaud = 9600;

bool output_serial;

WiFiUDP Udp;
IPAddress udpip(192, 168, 178, 145); // Your PC IP address. TODO: config via CLI

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
      if(output_serial) Serial.write(gps->read()); // Send output via primary serial port (Direct connection)
      else udp_write(gps->read(), 9000);  // Send output to your open PC port via UDP (Virtual serial port)
                                          // If you want undestand more about virtual serial port:
                                          // https://hpsaturn.com/virtual-serial-port/
    }
  } while (millis() - start < ms);
}

void cli_output(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  String enable = operands.first();
  if (enable.equals("on")) output_serial = true;
  else if (enable.equals("off")) output_serial = false;
  else Serial.printf("\r\nbad argument. Please use on or off\r\n");
  Serial.printf("\rGPS output enable: %s\r", output_serial ? "on" : "off");
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.flush();
  delay(1000);

  pinMode(HW_EN, OUTPUT);     // optional hardware pin enable for example with a
  digitalWrite(HW_EN, HIGH);  // step-up for enable/disable all hardware.

  delay(100);

  // GPS initialization
  Serial2.begin(GPSBaud, SERIAL_8N1, GPS_RX, GPS_TX);
  gps = &Serial2;
  delay(100);

  // WiFi CLI init and custom commands
  wcli.begin();
  // adding output command. To choose the Serial2 redirection, to UDP or Serial.
  wcli.term->add("output", &cli_output, "\t[on|off] GPS serial output.");

  // Configuration loop:
  // 15 seconds for reconfiguration or first use case.
  // for reconfiguration type disconnect and switch the "output" mode
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED || (millis() - start < 15000)) wcli.loop();

  Serial.printf("\r\n== end setup ==\r\n");
}

void loop() {
  smartDelay(60);
  wcli.loop();
}
