
/*********************************************************************
 This sample file is part of the esp32-wifi-cli:
 Copyright (c) 2022, @hpsaturn, Antonio Vanegas
 https://hpsaturn.com, All rights reserved.
 https://github.com/hpsaturn/esp32-wifi-cli

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/

#include <Arduino.h>
#include <M5Atom.h>
#include <ESP32WifiCLI.hpp>

class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    M5.dis.setBrightness(5);     // set brightness to 50%
    if (isConnected)
      M5.dis.fillpix(0x00ff00);  // set LED to green
    else
      M5.dis.fillpix(0xffff00);  // set LED to yellow
  }
};

void setup() {
  M5.begin(true,false,true);  //Init Atom(Initialize serial port, LED)
  M5.dis.fillpix(0xffffff);   //Light LED with the specified RGB color. 
  Serial.flush();
  delay(1000);
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.begin();
}

void loop() {
  wcli.loop();
}