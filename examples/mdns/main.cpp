
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

#include <ESP32WifiCLI.hpp>
#include <ESPmDNS.h>

/*********************************************************************
 * User defined commands
 ********************************************************************/

void setHostname(char *args, Stream *response) {
  String host = wcli.parseArgument(args);
  Serial.println(WiFi.localIP());
 
  int attemp = 0;
  while(!MDNS.begin(host.c_str()) && attemp++ < 3) {
     Serial.println("Starting mDNS...");
     delay(1000);
  }
  Serial.println("MDNS started"); 
}

void getIpAddress(char *args, Stream *response) {
  String host = wcli.parseArgument(args);
  Serial.printf("Resolving hostname: %s\r\n",host.c_str());

  while(mdns_init()!= ESP_OK){
    delay(1000);
    Serial.println("Starting MDNS...");
  }

  Serial.println("MDNS started");
 
  IPAddress serverIp;
  int attemp = 0;
 
  while (serverIp.toString() == "0.0.0.0" && attemp++ < 5) {
    Serial.println("Resolving host...");
    delay(250);
    serverIp = MDNS.queryHost(host.c_str(),3000);
  }
  if (serverIp.toString() != "0.0.0.0") {
    Serial.println("Host address resolved:");
    Serial.println(serverIp.toString());
  }
  else {
    Serial.println("Host was not found!");
  }
}

void reboot(char *args, Stream *response){
  ESP.restart();
}

void setup() {
  Serial.begin(115200); // Optional, you can init it on begin()
  Serial.flush();       // Only for showing the message on serial 
  delay(1000);
  // wcli.disableConnectInBoot();
  // wcli.setSilentMode(true);
  // wcli.clearSettings(); // Clear all networks and settings

  // Enter your custom commands:
  wcli.add("host", &setHostname, "\t<hostname> set hostname into quotes");
  wcli.add("getIp", &getIpAddress, "\t<hostname> get IP address of hostname into quotes");
  wcli.add("reboot", &reboot, "\tperform a ESP32 reboot");
  wcli.begin();         // Alternatively, you can init with begin(115200) 
}

void loop() {
  wcli.loop();
}