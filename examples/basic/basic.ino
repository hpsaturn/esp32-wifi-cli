
/*********************************************************************
 This sample file is part of the esp32-wifi-cli:
 Copyright (c) 2022, @hpsaturn, Antonio Vanegas
 https://hpsaturn.com, All rights reserved.
 https://github.com/hpsaturn/esp32-wifi-cli
 *********************************************************************/

/*********************************************************************
 * Basic example of ESP32WifiCLI.
 * 
 * After compiling and running this sketch, you can use the WiFi CLI
 * to connect to a WiFi network via the serial console or device monitor
 * 
 * Enter to the console and type ENTER, reset your device and you should 
 * see something like the following:

SerialTerm v1.1.1
(C) 2019, MikO
  available commands:
        help            show detail usage information
        setSSID         set the Wifi SSID
        setPASW         set the WiFi password
        connect         save and connect to WiFi network
        list            list saved WiFi networks
        select          select the default AP (default: last)
        mode            set the default operation single/multi AP (slow)
        scan            scan WiFi networks
        status          WiFi status information
        disconnect      WiFi disconnect
        delete          remove saved WiFi network by SSID

st>

* To save and setup your first WiFi network, type:

        setSSID "my-wifi-network"
        setPASW "my-wifi-password"
        connect

* For advanced usage, you can review the advanced examples.
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