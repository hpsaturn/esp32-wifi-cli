
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

bool set_color = false;

/*********************************************************************
 * Optional callback.
 ********************************************************************/
class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    M5.dis.setBrightness(5);  // set brightness to 50%
    if (!set_color) {         // when you set a color stop wifi notification
      if (isConnected)
        M5.dis.fillpix(0x00ff00);  // set LED to green
      else
        M5.dis.fillpix(0xffff00);  // set LED to yellow
    }
  }
  void onHelpShow() {}
  void onNewWifi(String ssid, String passw) {}
};

/*********************************************************************
 * User defined commands. Example: suspend, blink, reboot, etc.
 ********************************************************************/
void blink(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int times = operands.first().toInt();
  int miliseconds = operands.second().toInt();
  for (int i = 0; i < times; i++) {
    M5.dis.fillpix(0xaaff00);  // set LED to green
    delay(miliseconds);
    M5.dis.fillpix(0xff0000);  // set LED to green
    delay(miliseconds);
  }
}

void color(char *args, Stream *response){
  String hexcolor = wcli.parseArgument(args);
  set_color = true;
  int red;
  int green;
  int blue;
  sscanf(hexcolor.c_str(), "%i %i %i", &red, &green, &blue);
  M5.dis.setBrightness(20);  
  M5.dis.fillpix(CRGB().setRGB(red, green, blue));
}

void echo(char *args, Stream *response) {
  String echo = wcli.parseArgument(args);
  Serial.println("\r\nmsg: "+echo);
}

void reboot(char *args, Stream *response){
  ESP.restart();
}

void setup() {
  M5.begin(true,false,true);  //Init Atom(Initialize serial port, LED)
  M5.dis.fillpix(0xffffff);   //Light LED with the specified RGB color. 
  Serial.flush();
  delay(1000);
  wcli.setCallback(new mESP32WifiCLICallbacks());
  // User custom commands:
  wcli.add("blink", &blink, "\t\tLED blink x times each x millis");
  wcli.add("echo", &echo, "\t\tEcho message into quotes");
  wcli.add("color", &color, "\t\tset color into quotes: i.e \"255 128 0\"");
  wcli.add("reboot", &reboot, "\tperform a ESP32 reboot");
  wcli.begin();
}

void loop() {
  wcli.loop();
}