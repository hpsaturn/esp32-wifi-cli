#ifndef DISABLE_CLI_TELNET
#include <ESP32WifiCLI.hpp>

WiFiClient client_;
WiFiServer server( WCLI_SERVER_PORT );
Shellminator shellTelnet_( &server );
TaskHandle_t telnet_handle = NULL;

void initClientSession(const char * prompt, bool &init) {
  if(init) return;
  Serial.println("Telnet client connected");
  shellTelnet_.clear();
  shellTelnet_.begin(prompt);
  shellTelnet_.update();
  init = true;
}

void telnetClientTask(void *parameter) {
  char *prompt_name = (char *)parameter;
  bool init_session;
  for (;;) {
    initClientSession(prompt_name, init_session);
    shellTelnet_.update();
    vTaskDelay(30 / portTICK_PERIOD_MS); 
  }
}

void startTelnet() {
  if (telnet_handle != NULL) return;
  wcli.shellTelnet = &shellTelnet_;
  wcli.shellTelnet->attachCommander(&wcli.commander);
  wcli.shellTelnet->attachLogo(logo_wcli);
  wcli.shellTelnet->beginServer();
  server.begin();
  xTaskCreate(telnetClientTask, "Telnet Task", 10000, (void *)wcli.prompt.c_str(), 5, &telnet_handle);
}

void stopTelnet() {
  if (telnet_handle == NULL) return;
  vTaskDelete(telnet_handle);
  wcli.client->stop();
  wcli.shellTelnet->stopServer();
  telnet_handle = NULL;
}

String telnetStatus(){
  return wcli.isTelnetEnable() ? "\033[0;32menable\033[0;37m" : "\033[0;31mdisable\033[0;37m";
}

void _nmcli_telnet(char *args, Stream *response) {
  Pair<String, String> operands = wcli.parseCommand(args);
  String param = operands.first();
  param.toUpperCase();
  bool enable;
  if (param.equals("ENABLE"))
    enable = true;
  else if (param.equals("DISABLE"))
    enable = false;
  else {
    response->println("invalid syntax: use\033[0;33m nmcli telnet [enable|disable]\033[0m");
    response->printf("service status: %s\r\n", telnetStatus().c_str());
    return;
  }
  if (enable) {
    response->println("starting telnet server..");
    wcli.enableTelnet();
    delay(1000);
  }
  else {
    response->println("stopping telnet server..");
    wcli.disableTelnet();
    delay(1000);
  }
}

void ESP32WifiCLI::enableTelnet() {
  startTelnet();
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putBool("telnet_enable",true);
  wcfg.end();
}

void ESP32WifiCLI::disableTelnet() { 
  stopTelnet(); 
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putBool("telnet_enable",false);
  wcfg.end();
}

bool ESP32WifiCLI::isTelnetEnable() { 
  wcfg.begin(app_name.c_str(), RO_MODE);
  bool enable = wcfg.getBool("telnet_enable", false);
  wcfg.end();
  return enable;
}
#endif