#ifndef DISABLE_CLI_TELNET
#include <ESP32WifiCLI.hpp>

WiFiClient client_;
WiFiServer server( WCLI_SERVER_PORT );
Shellminator shellTelnet_( &server );
TaskHandle_t telnet_handle = NULL;

void initClientSession(const char * prompt) {
  Serial.println("\r\nTelnet session ready");
  shellTelnet_.begin(prompt);
  shellTelnet_.clear();
}

void telnetClientTask(void *parameter) {
  char * prompt_name = (char *) parameter;
  for (;;) {
      
    client_ = server.available();

    if (client_) {
      initClientSession(prompt_name);
      while (client_.connected()) {
        shellTelnet_.update();
        vTaskDelay(60 / portTICK_PERIOD_MS);
      }
    }
    vTaskDelay(120 / portTICK_PERIOD_MS);
  }
}

void startTelnet() {
  if (telnet_handle != NULL) return;
  wcli.shellTelnet = &shellTelnet_;
  wcli.shellTelnet->attachCommander(&wcli.commander);
  wcli.shellTelnet->attachLogo(logo_wcli);
  wcli.shellTelnet->beginServer();
  xTaskCreate(telnetClientTask, "Telnet Task", 10000, (void *)wcli.prompt.c_str(), 1, &telnet_handle);
}

void stopTelnet() {
  if (telnet_handle == NULL) return;
  vTaskDelete(telnet_handle);
  wcli.client->stop();
  wcli.shellTelnet->stopServer();
  telnet_handle = NULL;
}

void _nmcli_telnet(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String param = operands.first();
  param.toUpperCase();
  bool enable;
  if (param.equals("ON") || param.equals("START"))
    enable = true;
  else if (param.equals("OFF") || param.equals("STOP"))
    enable = false;
  else {
    response->println("invalid syntax: use on/off or star/stop");
    return;
  }
  if (enable) {
    response->println("starting telnet server..");
    startTelnet(); 
    delay(1000);
  }
  else {
    response->println("stopping telnet server..");
    delay(1000);
    stopTelnet(); 
  }
}

void ESP32WifiCLI::enableTelnet() { startTelnet(); }

void ESP32WifiCLI::disableTelnet() { stopTelnet(); }


#endif