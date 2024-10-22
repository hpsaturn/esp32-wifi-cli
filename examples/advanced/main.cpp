
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

int LED_PIN = 13; // change it via CLI using this example :D
                  // for instance if you are using a LilyGO T7 v1.5 board,
                  // set LED like this:
                  //   setled 19
                  // and the reboot with the command reboot.
                  // Also the LED could be ON when the WiFi is ready.

const char logo[] =
"▓█████ ▒██   ██▒ ▄▄▄       ███▄ ▄███▓ ██▓███   ██▓    ▓█████ \n"
"▓█   ▀ ▒▒ █ █ ▒░▒████▄    ▓██▒▀█▀ ██▒▓██░  ██▒▓██▒    ▓█   ▀ \n"
"▒███   ░░  █   ░▒██  ▀█▄  ▓██    ▓██░▓██░ ██▓▒▒██░    ▒███   \n"
"▒▓█  ▄  ░ █ █ ▒ ░██▄▄▄▄██ ▒██    ▒██ ▒██▄█▓▒ ▒▒██░    ▒▓█  ▄ \n"
"░▒████▒▒██▒ ▒██▒ ▓█   ▓██▒▒██▒   ░██▒▒██▒ ░  ░░██████▒░▒████▒\n"
"░░ ▒░ ░▒▒ ░ ░▓ ░ ▒▒   ▓▒█░░ ▒░   ░  ░▒▓▒░ ░  ░░ ▒░▓  ░░░ ▒░ ░\n"
" ░ ░  ░░░   ░▒ ░  ▒   ▒▒ ░░  ░      ░░▒ ░     ░ ░ ▒  ░ ░ ░  ░\n"
"   ░    ░    ░    ░   ▒   ░      ░   ░░         ░ ░      ░   \n"
"   ░  ░ ░    ░        ░  ░       ░                ░  ░   ░  ░\n"
"                                                             \n"
"\n"
"";

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
  }

  void onNewWifi(String ssid, String passw) {
  }
};

/*********************************************************************
 * User defined commands sectioh. Examples: suspend, blink, reboot, etc.
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

void blink(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int times = operands.first().toInt();
  int miliseconds = operands.second().toInt();
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
    wcli.setInt("KEY_LED_PIN", pin);
    Serial.println("\r\nLED GPIO set to " + String(pin));
    Serial.println("Please reboot to apply the change.");
  }
  else {
    Serial.println("setLED: invalid pin number");
  }
}

void echo(char *args, Stream *response) {
  String echo = wcli.parseArgument(args);
  Serial.println(echo);
}

void info(char *args, Stream *response) {
  wcli.status(response);
}

void reboot(char *args, Stream *response){
  wcli.shell->clear();
  ESP.restart();
}

void setup() {
  Serial.begin(115200);  // Optional, you can init it on begin()
  Serial.flush();        // Only for showing the message on serial
  delay(2000);           // Only for this demo
  wcli.setCallback(new mESP32WifiCLICallbacks());
  wcli.setSilentMode(true);  // less debug output
  
  // Enter your custom commands:
  wcli.add("sleep", &sleep,     "\t\t<mode> <time> ESP32 sleep mode (deep/light)\r\n");
  wcli.add("echo", &echo,       "\t\t\"message\" Echo the msg. Parameter into quotes");
  wcli.add("info", &info,       "\t\tsystem status info");
  wcli.add("setled", &setled,   "\t<PIN> config the LED GPIO for blink");
  wcli.add("blink", &blink,     "\t\t<times> <millis> LED blink x times each x millis");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  
  wcli.shell->attachLogo(logo);
  wcli.shell->clear();
  wcli.begin();  // Alternatively, you can init with begin(115200,appname)

  // Configure previously configured LED pins via CLI command
  LED_PIN = (int) wcli.getInt("KEY_LED_PIN", LED_PIN);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  wcli.loop();
}