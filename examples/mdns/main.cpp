
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

int LED_PIN = 13;

/*********************************************************************
 * Optional callback.
 ********************************************************************/
class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    if(isConnected) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }

  void onHelpShow() {
    // Enter your custom help here:
    Serial.println("\r\nCustom commands:\r\n");
    Serial.println("sleep <mode> <time> \tESP32 sleep mode (deep or light)");
    Serial.println("echo \"message\" \t\tEcho the msg. Parameter into quotes");
    Serial.println("setLED <PIN> \t\tconfig the LED GPIO for blink");
    Serial.println("blink <times> <millis> \tLED blink x times each x millis");
    Serial.println("hostname <hostname> \tset hostname into quotes");
    Serial.println("getIp <hostname> \tget IP address of hostname into quotes");
    Serial.println("reboot\t\t\tperform a soft ESP32 reboot");
  }

  void onNewWifi(String ssid, String passw) {
  }
};

/*********************************************************************
 * User defined commands. Example: suspend, blink, reboot, etc.
 ********************************************************************/
void gotToSuspend(int type, int seconds) {
    delay(8);  // waiting for writing msg on serial
    //esp_deep_sleep(1000000LL * DEEP_SLEEP_DURATION);
    esp_sleep_enable_timer_wakeup(1000000LL * seconds);
    if (type == 0) esp_deep_sleep_start();
    else esp_light_sleep_start(); 
}

void sleep(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int seconds = operands.second().toInt();
  if(operands.first().equals("deep")) {
    Serial.println("\ndeep suspending..");
    gotToSuspend(0, seconds);
  }
  else if(operands.first().equals("light")) {
    Serial.println("\nlight suspending..");
    gotToSuspend(1, seconds);
  }
  else {
    Serial.println("sleep: invalid option");
  }
}

void blink(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int times = operands.first().toInt();
  int miliseconds = operands.second().toInt();
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(miliseconds);
    digitalWrite(LED_PIN, LOW);
    delay(miliseconds);
  }
}

void setLED(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int pin = operands.first().toInt();
  if(pin >= 0 && pin <= 31) {
    wcli.setInt("LED_PIN", pin);
    Serial.println("\r\nLED GPIO set to " + String(pin));
    Serial.println("Please reboot to apply the change.");
  }
  else {
    Serial.println("setLED: invalid pin number");
  }
}

void echo(String opts) {
  String echo = maschinendeck::SerialTerminal::ParseArgument(opts);
  Serial.println(echo);
}

void reboot(String opts){
  ESP.restart();
}

void setHostname(String opts) {
  String host = maschinendeck::SerialTerminal::ParseArgument(opts);
  Serial.println(WiFi.localIP());
 
  while(!MDNS.begin(host.c_str())) {
     Serial.println("Starting mDNS...");
     delay(1000);
  }
 
  Serial.println("MDNS started"); 
}

void getIpAddress(String opts) {
  String host = maschinendeck::SerialTerminal::ParseArgument(opts);
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

  attemp = 0;

  if (serverIp.toString() != "0.0.0.0") {
    Serial.println("Host address resolved:");
    Serial.println(serverIp.toString());
  }
  else {
    Serial.println("Host was not found!");
  }
  
}

void setup() {
  Serial.begin(115200); // Optional, you can init it on begin()
  Serial.flush();       // Only for showing the message on serial 
  delay(1000);
  wcli.setCallback(new mESP32WifiCLICallbacks());
  // wcli.disableConnectInBoot();
  // wcli.setSilentMode(true);
  // wcli.clearSettings(); // Clear all networks and settings
  wcli.begin();         // Alternatively, you can init with begin(115200) 

  // Configure previously configured LED pins via CLI command
  int LED_PIN = wcli.getInt("LED_PIN", LED_PIN);
  pinMode(LED_PIN, OUTPUT);

  // Enter your custom commands:
  wcli.term->add("sleep", &sleep, "\t<mode> <time> ESP32 will enter to sleep mode");
  wcli.term->add("echo", &echo, "\t\"message\" Echo the msg. Parameter into quotes");
  wcli.term->add("setLED", &setLED, "\t<PIN> config the LED GPIO for blink");
  wcli.term->add("blink", &blink, "\t<times> <millis> LED blink x times each x millis");
  wcli.term->add("host", &setHostname, "\t<hostname> set hostname into quotes");
  wcli.term->add("getIp", &getIpAddress, "\t<hostname> get IP address of hostname into quotes");
  wcli.term->add("reboot", &reboot, "\tperform a ESP32 reboot");
}

void loop() {
  wcli.loop();
}