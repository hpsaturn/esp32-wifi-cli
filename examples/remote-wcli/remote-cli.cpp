
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

int LED_PIN = 17;

const char logo[] =
"                                                             \r\n"
"▓█████ ▒██   ██▒ ▄▄▄       ███▄ ▄███▓ ██▓███   ██▓    ▓█████ \r\n"
"▓█   ▀ ▒▒ █ █ ▒░▒████▄    ▓██▒▀█▀ ██▒▓██░  ██▒▓██▒    ▓█   ▀ \r\n"
"▒███   ░░  █   ░▒██  ▀█▄  ▓██    ▓██░▓██░ ██▓▒▒██░    ▒███   \r\n"
"▒▓█  ▄  ░ █ █ ▒ ░██▄▄▄▄██ ▒██    ▒██ ▒██▄█▓▒ ▒▒██░    ▒▓█  ▄ \r\n"
"░▒████▒▒██▒ ▒██▒ ▓█   ▓██▒▒██▒   ░██▒▒██▒ ░  ░░██████▒░▒████▒\r\n"
"░░ ▒░ ░▒▒ ░ ░▓ ░ ▒▒   ▓▒█░░ ▒░   ░  ░▒▓▒░ ░  ░░ ▒░▓  ░░░ ▒░ ░\r\n"
" ░ ░  ░░░   ░▒ ░  ▒   ▒▒ ░░  ░      ░░▒ ░     ░ ░ ▒  ░ ░ ░  ░\r\n"
"   ░    ░    ░    ░   ▒   ░      ░   ░░         ░ ░      ░   \r\n"
"   ░  ░ ░    ░        ░  ░       ░                ░  ░   ░  ░\r\n"
"                                                             \r\n"
"\n"
"";

/*********************************************************************
 * Optional callback.
 ********************************************************************/
class mESP32WifiCLICallbacks : public ESP32WifiCLICallbacks {
  void onWifiStatus(bool isConnected) {
    if (isConnected) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }

  void onHelpShow() {}

  void onNewWifi(String ssid, String passw) {}
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

void sleep(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int seconds = operands.second().toInt();
  if(operands.first().equals("deep")) {
    response->println("\ndeep suspending..");
    gotToSuspend(0, seconds);
  }
  else if(operands.first().equals("light")) {
    response->println("\nlight suspending..");
    gotToSuspend(1, seconds);
  }
  else {
    response->println("sleep: invalid option");
  }
}

void blink(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int times = operands.first().toInt();
  int miliseconds = operands.second().toInt();
  response->printf("blink %i times each %i ms.\r\n", times, miliseconds);
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(miliseconds);
    digitalWrite(LED_PIN, LOW);
    delay(miliseconds);
  }
}

void setled(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int pin = operands.first().toInt();
  if(pin >= 0 && pin <= 31) {
    wcli.setInt("LED_PIN", pin);
    response->println("\r\nLED GPIO set to " + String(pin));
    response->println("Please reboot to apply the change.");
  }
  else {
    response->println("setLED: invalid pin number");
  }
}

void echo(char *args, Stream *response) {
  String echo = wcli.parseArgument(args);
  response->println(echo);
}

void reboot(char *args, Stream *response){
  wcli.shell->clear();
  wcli.client->stop();
  ESP.restart();
}

void initRemoteShell(){
  wcli.enableTelnet();
  wcli.shellTelnet->attachLogo(logo);
}

void initSerialShell(){
  // Enter your custom commands:
  wcli.add("sleep", &sleep,     "\t\t<mode> <time> ESP32 sleep mode (deep/light)\r\n");
  wcli.add("echo", &echo,       "\t\t\"message\" Echo the msg. Parameter into quotes");
  wcli.add("setled", &setled,   "\t<PIN> config the LED GPIO for blink");
  wcli.add("blink", &blink,     "\t\t<times> <millis> LED blink x times each x millis");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  wcli.shell->attachLogo(logo);
  wcli.begin();  // Alternatively, you can init with begin(115200,appname)
}

void setup() {
  Serial.begin(115200);  // Optional, you can init it on begin()
  Serial.flush();        // Only for showing the message on serial
  delay(2000);           // Only for this demo
  // Configure previously configured LED pins via CLI command
  int LED_PIN = wcli.getInt("LED_PIN", LED_PIN);
  
  wcli.setCallback(new mESP32WifiCLICallbacks());
  // Disable WiFi connect in boot, you able to connect after setup with connect cmd.
  // wcli.disableConnectInBoot();
  wcli.setSilentMode(true);  // less debug output
 
  initSerialShell();
  initRemoteShell();  
}

void loop() {
  wcli.loop();
}

