
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
 * User defined commands. Example: echo, info, reboot, etc.
 ********************************************************************/

void echo(char *args, Stream *response) {
  // parse one argument only if is into double quotes:
  String echo = wcli.parseArgument(args);
  response->println(echo);
}

void info(char *args, Stream *response) {
  wcli.status(response);
}

void sum(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  int a = operands.first().toInt();
  int b = operands.second().toInt();
  response -> print( a );
  response -> print( " + " );
  response -> print( b );
  response -> print( " = " );
  response -> println( a + b );
}

void reboot(char *args, Stream *response){
  wcli.shell->clear();
  wcli.client->stop();
  ESP.restart();
}

void initRemoteShell(){
  if (wcli.isTelnetEnable()) wcli.shellTelnet->attachLogo(logo);
}

void initSerialShell(){
  // Enter your custom commands:
  wcli.add("sum", &sum,         "\t\t<int> <int> calculate the sum into two numbers\r\n");
  wcli.add("echo", &echo,       "\t\t\"message\" example of echo of a message into quotes");
  wcli.add("info", &info,       "\t\tsystem status info");
  wcli.add("reboot", &reboot,   "\tperform a ESP32 reboot");
  wcli.shell->attachLogo(logo);
  wcli.begin();  // Alternatively, you can init with begin(115200,appname)
}

void setup() {
  Serial.begin(115200);  // Optional, you can init it on begin()
  Serial.flush();        // Only for showing the message on serial
  delay(2000);           // Only for this demo
  
  wcli.setSilentMode(true);  // less debug output
 
  initSerialShell();
  initRemoteShell();  
}

void loop() {
  wcli.loop();
}

