
/*********************************************************************
 This sample file is part of the Serial Terminal Library:
 Copyright (c) 2022, @hpsaturn, Antonio Vanegas
 https://hpsaturn.com, All rights reserved.

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

#include <M5Atom.h>
#include <SerialTerminal.hpp>
#include "WiFi.h"

maschinendeck::SerialTerminal* term;

String ssid;
String pasw;

void printWifiSettings() {
  Serial.print("\nWiFi Connect To : ");
  Serial.println(WiFi.SSID());       //Output Network name.
  Serial.print("IP address   \t: ");
  Serial.println(WiFi.localIP());    //Output IP Address.
  Serial.print("RSSI\t\t: ");
  Serial.println(WiFi.RSSI());       //Output signal strength.
}

void printHelp(String opts) {
  Serial.println("\nUsage:\n");
  Serial.println("setSSID \"YOUR SSID\"");
  Serial.println("setPassword \"YOUR PASSWORD\"");
  Serial.println("connect");
}

void setSSID(String opts) {
  ssid = maschinendeck::SerialTerminal::ParseArgument(opts);
  Serial.println("\n\tsaved ssid to   \t: " + ssid);
}

void setPassword(String opts) {
  pasw = maschinendeck::SerialTerminal::ParseArgument(opts);
  Serial.println("\n\tsaved password to \t: " + pasw);
}

void connect(String opts) {
  WiFi.begin(ssid.c_str(), pasw.c_str());
  while (WiFi.status() != WL_CONNECTED) { // M5Atom will connect automatically
    delay(500);
    Serial.print(".");
  }
  M5.dis.fillpix(0x00ff00);   // set LED to green
  printWifiSettings();
}

void setup() {
  M5.begin(true,false,true);  //Init Atom(Initialize serial port, LED)
  M5.dis.fillpix(0xfff000);   //Light LED with the specified RGB color.
  delay(1000);
  Serial.flush();
  Serial.println("\n\n");
  term = new maschinendeck::SerialTerminal(115200);
  term->add("help", &printHelp, "\tshow help and usage information");
  term->add("setSSID", &setSSID, "\tset the Wifi SSID");
  term->add("setPassword", &setPassword, "set the Wifi password"); 
  term->add("connect", &connect, "\tconnect to the wifi station");
}

void loop() {
  term->loop();
}