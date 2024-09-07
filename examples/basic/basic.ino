
/*********************************************************************
 This sample file is part of the esp32-wifi-cli:
 Copyright (c) 2022, @hpsaturn, Antonio Vanegas
 https://hpsaturn.com, All rights reserved.
 https://github.com/hpsaturn/esp32-wifi-cli
 *********************************************************************/

/*********************************************************************
 * Basic example of ESP32WifiCLI.
 * 
 * After compiling and running this sketch, you can use the network manager
 * CLI to connect to your WiFi network via the serial console or device monitor.
 * For details please type "nmcli help".
 * 
 ********************************************************************/

#include <Arduino.h>
#include <ESP32WifiCLI.hpp>

void setup() {
  Serial.begin(115200);
  wcli.begin();
  delay(100);
}

void loop() {
  wcli.loop();
}