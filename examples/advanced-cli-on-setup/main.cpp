
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

int LED_PIN = 13;
bool setup_mode = false;
int setup_time = 10000;
bool first_run = true;

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

void setLED(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
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

void echo(char *args, Stream *response) {
  String echo = wcli.parseArgument(args);
  Serial.println(echo);
}

void reboot(char *args, Stream *response){
  ESP.restart();
}

void wcli_exit(char *args, Stream *response) {
  setup_time = 0;
  setup_mode = false;
}

void wcli_setup(char *args, Stream *response) {
  setup_mode = true;
  Serial.println("\r\nSetup Mode Enable (fail-safe mode)\r\n");
}

void setup() {
  Serial.begin(115200);  // Optional, you can init it on begin()
  Serial.flush();        // Only for showing the message on serial
  delay(1000);           // Only for this demo
  wcli.setCallback(new mESP32WifiCLICallbacks());
  // Disable WiFi connect in boot, you able to connect after setup with connect cmd.
  wcli.disableConnectInBoot();
  wcli.setSilentMode(true);  // less debug output

  // Configure previously configured LED pins via CLI command
  int LED_PIN = wcli.getInt("LED_PIN", LED_PIN);
  pinMode(LED_PIN, OUTPUT);

  // Enter your custom commands:
  wcli.add("sleep", &sleep,     "\t<mode> <time> ESP32 will enter to sleep mode");
  wcli.add("echo", &echo,       "\t\"message\" Echo the msg. Parameter into quotes");
  wcli.add("setLED", &setLED,   "\t<PIN> config the LED GPIO for blink");
  wcli.add("blink", &blink,     "\t<times> <millis> LED blink x times each x millis");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  // setup mode commands:
  wcli.add("exit", &wcli_exit,  "\texit of the setup mode. AUTO EXIT in 10 seg! :)");
  wcli.add("setup", &wcli_setup,"\tTYPE THIS WORD to start to configure the device :D\n");
  
  wcli.begin();              // Alternatively, you can init with begin(115200)

  // Configuration loop in setup:
  // 10 seconds for reconfiguration (first use case or fail-safe mode for example)
  uint32_t start = millis();
  while (setup_mode || (millis() - start < setup_time)) wcli.loop();
  Serial.println();

  if (setup_time == 0)
    Serial.println("==>[INFO] Settings mode end. Booting..\r\n");
  else
    Serial.println("==>[INFO] Time for initial setup over. Booting..\r\n");
}

void loop() {
  wcli.loop();
}