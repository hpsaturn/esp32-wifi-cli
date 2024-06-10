#include <ESP32WifiCLI.hpp>
#include "storage_utils.h"

Shellminator shell_( &Serial );
WiFiClient client_;
WiFiServer server( WCLI_SERVER_PORT );
Shellminator shellTelnet_( &server );

String temp_ssid = "";
String temp_pasw = "";


const char logo[] =
"\r\n"
"=============================\r\n"
"                             \r\n"
"██     ██  ██████ ██      ██ \r\n"
"██     ██ ██      ██      ██ \r\n"
"██  █  ██ ██      ██      ██ \r\n"
"██ ███ ██ ██      ██      ██ \r\n"
" ███ ███   ██████ ███████ ██ \r\n"
"                             \r\n"
"=============================\r\n"
"\r\n"
;

void terminalClientTask(void *parameter);

ESP32WifiCLI::ESP32WifiCLI(){
  this->shell = &shell_;
  shell->attachLogo(logo);
  this->client = &client_;
}

void ESP32WifiCLI::printHelp() {
  this->shell->printHelp();
}

String ESP32WifiCLI::getNetKeyName(int net) {
  if (net > 99) return "";
  char key[11];
  sprintf(key, "key_net%02d", net);
  return String(key);
}

void ESP32WifiCLI::loadSavedNetworks(bool addAP, Stream *response) {
  cfg.begin(app_name.c_str(), RO_MODE);
  int net = 1;
  int default_net = cfg.getInt("default_net", 1);
  if (!addAP) response->println("\nSaved networks:\n");
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid = cfg.getString(String(key + "_ssid").c_str(), "");
    String pasw = cfg.getString(String(key + "_pasw").c_str(), "");
    String dfl = (net == default_net) ? "*" : " ";
    if (!addAP) response->printf("(%s) %d: [%s]\r\n", dfl.c_str(), net, ssid.c_str());
    if (addAP) wifiMulti.addAP(ssid.c_str(), pasw.c_str());
    net++;
  }
  if(!addAP)response->println("");
  cfg.end();
}

void ESP32WifiCLI::deleteNetwork(String ssid, Stream *response) {
  if (ssid.length() == 0) {
    response->println("\nSSID is empty, please set a valid SSID into quotes");
    return;
  }
  int net = 1;
  bool dropped = false;
  cfg.begin(app_name.c_str(), RW_MODE);
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (!dropped && ssid_.equals(ssid)) {
      response->printf("\nDeleting network [%s][%s]\r\n", key.c_str(), ssid.c_str());
      cfg.remove(String(key + "_ssid").c_str());
      cfg.remove(String(key + "_pasw").c_str());
      dropped = true;
      int net_count = cfg.getInt("net_count", 0);
      cfg.putInt("net_count", net_count - 1);
      int default_net = cfg.getInt("default_net", 1);
      if (net - 1 == default_net && net_count > 1 ) cfg.putInt("default_net", net_count - 1);
      continue;
    }
    if (dropped) {
      String ssid_drop = cfg.getString(String(key + "_ssid").c_str(), "");
      String pasw_drop = cfg.getString(String(key + "_pasw").c_str(), "");
      String key_drop = getNetKeyName(net - 2);
      // response->printf("ssid drop: [%s][%d][%s]\r\n",ssid_drop.c_str(), net - 2, key_drop.c_str());
      cfg.putString(String(key_drop + "_ssid").c_str(), ssid_drop);
      cfg.putString(String(key_drop + "_pasw").c_str(), pasw_drop);
      // response->printf("remove key: [%d][%s]\r\n",net - 1, key.c_str());
      int default_net = cfg.getInt("default_net", 1);
      if (net - 1 == default_net) cfg.putInt("default_net", net - 2);
      cfg.remove(String(key + "_ssid").c_str());
      cfg.remove(String(key + "_pasw").c_str());
    }
  }
  cfg.end();
  loadSavedNetworks(false, response);
}

void ESP32WifiCLI::saveNetwork(String ssid, String pasw) {
  cfg.begin(app_name.c_str(), RW_MODE);
  int net = cfg.getInt("net_count", 0);
  String key = getNetKeyName(net + 1);
  if (!silent) Serial.printf("Saving network: [%s][%s]\r\n", ssid.c_str(), pasw.c_str());
  cfg.putString(String(key + "_ssid").c_str(), ssid);
  cfg.putString(String(key + "_pasw").c_str(), pasw);
  cfg.putInt("net_count", net + 1);
  cfg.putInt("default_net", net + 1);
  cfg.end();
}

void ESP32WifiCLI::list() {
  loadSavedNetworks(false);
}

bool ESP32WifiCLI::isSSIDSaved(String ssid) {
  cfg.begin(app_name.c_str(), RO_MODE);
  bool isSaved = false;
  int net = 1;
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (ssid_.equals(ssid)) {
      isSaved = true;
      break;
    }
  }
  cfg.end();
  return isSaved;
}

bool ESP32WifiCLI::wifiValidation() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("connected!");
    if(!silent) status();
    return true;
  } else {
    Serial.println("connection failed!");
    return false;
  }
}

void ESP32WifiCLI::wifiAPConnect(bool save) {
  if (temp_ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes\n");
    return;
  }
  Serial.print("\nConnecting to " + temp_ssid + "...");
  if (save) {
    wifiMulti.addAP(temp_ssid.c_str(), temp_pasw.c_str());
  }
  int retry = 0;
  WiFi.begin(temp_ssid.c_str(), temp_pasw.c_str());

  #ifdef FAMILY
  if (FAMILY == "ESP32-C3") WiFi.setTxPower(WIFI_POWER_8_5dBm);  // TODO: uggly workaround for some C3 devices
  #endif

  while (WiFi.status() != WL_CONNECTED && retry++ < 20) {  // M5Atom will connect automatically
    delay(1000);
    Serial.print(".");
  }
  delay(100);
  if (wifiValidation() && save) {
    saveNetwork(temp_ssid, temp_pasw);
    if(cb != nullptr) cb->onNewWifi(temp_ssid, temp_pasw);
  }
}

bool ESP32WifiCLI::isConfigured() {
  cfg.begin("wifi_cli_prefs", RO_MODE);
  String key = getNetKeyName(1);
  bool isConfigured = cfg.isKey(String(key + "_ssid").c_str());
  cfg.end();
  return isConfigured;
}

bool ESP32WifiCLI::loadAP(int net) {
  cfg.begin(app_name.c_str(), RO_MODE);
  String key = getNetKeyName(net);
  if (!cfg.isKey(String(key + "_ssid").c_str())) {
    cfg.end();
    return false;
  }
  // Serial.printf("\nDefault AP: %i: [%s]\r\n", net, cfg.getString(String(key + "_ssid").c_str(), "").c_str());
  temp_ssid = cfg.getString(String(key + "_ssid").c_str(), "");
  temp_pasw = cfg.getString(String(key + "_pasw").c_str(), "");
  cfg.end();
  return true;
}

void ESP32WifiCLI::selectAP(int net, Stream *response) {
  if (!loadAP(net)) {
    response->println("\nNetwork not found");
    return;
  }
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putInt("default_net", net);
  cfg.end();
  loadSavedNetworks(false, response);
}

int ESP32WifiCLI::getDefaultAP() {
  cfg.begin(app_name.c_str(), RO_MODE);
  int net = cfg.getInt("default_net", 1);
  cfg.end();
  return net;
}

String ESP32WifiCLI::getMode() {
  cfg.begin(app_name.c_str(), RO_MODE);
  String mode = cfg.getString("mode", "single");
  cfg.end();
  return mode;
}

void ESP32WifiCLI::setMode(String mode, Stream *response) {
  cfg.begin(app_name.c_str(), RW_MODE);
  if (mode.equals("single")) {
    cfg.putString("mode", "single");
  } else if (mode.equals("multi")) {
    cfg.putString("mode", "multi");
  } else if (mode.equals("")) {
    response->printf("\nCurrent mode: %s\r\n", cfg.getString("mode", "single").c_str());
  } else {
    response->println("\nInvalid mode, please use single/multi parameter");
  }
  cfg.end();
}

void ESP32WifiCLI::multiWiFiConnect() {
  int retry = 0;
  Serial.print("\nConnecting in MultiAP mode..");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED && retry++ < 10) {
    delay(500);
    Serial.print(".");
  }
  wifiValidation();
}

void ESP32WifiCLI::reconnect() {
  if (WiFi.status() != WL_CONNECTED && getMode().equals("single")) {
    wifiAPConnect(false);
  } else {
    multiWiFiConnect();
  }
}



void _scanNetworks(char *args, Stream *response) {
  int n = WiFi.scanNetworks();
  response->print("\nscan done: ");
  if (n == 0) {
    response->println("no networks found");
  } else {
    response->print(n);
    response->println(" networks found\n");
    for (int i = 0; i < n; ++i) {
      String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[O]" : "[*]";
      response->printf("%02d %s[%i][%s]\r\n", i + 1, enc.c_str(), WiFi.RSSI(i), WiFi.SSID(i).c_str());
      delay(10);
    }
  }
}

void _printHelp(char* args, Stream *response) {
  wcli.printNetworkHelp();
}

void _setSSID(char *args, Stream *response) {
  String ssid = ParseArgument(args);
  temp_ssid = ssid;
  if (temp_ssid.length() == 0) {
    response->println("\nSSID is empty, please set a valid SSID into quotes");
  } else {
    response->println("set ssid to: " + temp_ssid);
  }
}

void _setPASW(char *args, Stream *response) {
  String pasw = ParseArgument(args);
  temp_pasw = pasw;
  response->println("set password to: " + temp_pasw);
}

void _disconnect(char *args, Stream *response) {
  response->println("\nDisconnecting...");
  WiFi.disconnect();
}

void _connect(char *args, Stream *response) {
  if (WiFi.status() == WL_CONNECTED && temp_ssid == WiFi.SSID()) {
    response->println("\nWiFi is already connected");
    
    return;
  } else if (WiFi.status() == WL_CONNECTED) {
    _disconnect(args, response);
    delay(1000);
  }
  if (wcli.getMode().equals("single")) {
    if (temp_ssid.length() == 0) {
      response->println("\nSSID is empty, please set a valid SSID into quotes\n");
      return;
    }
    if (wcli.isSSIDSaved(temp_ssid)) {
      wcli.wifiAPConnect(false);
      return;
    } else {
      wcli.wifiAPConnect(true);
    }
  } else {
    wcli.multiWiFiConnect();
  }
}

void _nmclicon(char *args, Stream *response) {
  // response->println(args);
  char *ssid = NULL;
  char *password = NULL;

  if (!extract_connect_parames(args, &ssid, &password, response)) return;

  if (ssid != NULL && password != NULL) {
    size_t len = strlen(password);
    if (password[0] == '"' && password[len - 1] == '"') {
      memmove(password, password + 1, len - 2);
      password[len - 2] = '\0';
    }
    // wcli.setSSID(ssid);
    // wcli.setPASW(password);
    temp_ssid = String(ssid);
    temp_pasw = String(password);
    free(ssid);
    free(password);
  }
  else{
    response->printf("Invalid command syntax\r\n");
    return;
  }
  _connect(args,response);
}

void _listNetworks(char *args, Stream *response) {
  wcli.loadSavedNetworks(false, response);
}

void _wifiStatus(char *args, Stream *response) {
  response->print("\nWiFi SSID \t: [");
  response->println(WiFi.SSID() + "]");  // Output Network name.
  response->print("IP address  \t: ");
  response->println(WiFi.localIP());     // Output IP Address.
  response->print("RSSI signal \t: ");
  response->println(WiFi.RSSI());        // Output signal strength.
  response->print("MAC Address\t: ");
  response->println(WiFi.macAddress());  // Output MAC address.
  response->print("Hostname \t: ");
  response->println(WiFi.getHostname()); // Output hostname.
  response->print("Memory free\t: ");
  response->println(String(ESP.getFreeHeap() / 1024) + "Kb");
}

void _deleteNetwork(char *args, Stream *response) {
  String ssid = ParseArgument(args);
  wcli.deleteNetwork(ssid, response);
}

void _selectAP(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  int net = operands.first().toInt();
  wcli.selectAP(net, response);
}

void _setMode(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  wcli.setMode(operands.first(), response);
}

void _nmcliProcessor(char *args, Stream *response) {
  if (strlen(args) == 0) {
    _printHelp(args, response);
  } else
    wcli.internal.execute(args, response);
}

void ESP32WifiCLI::printNetworkHelp() {
  this->shell->attachCommander(&internal);
  this->shell->printHelp();
  this->shell->attachCommander(&commander);
  if (shellTelnet == nullptr) return;
  this->shellTelnet->attachCommander(&internal);
  this->shellTelnet->printHelp();
  this->shellTelnet->attachCommander(&commander);
}

void ESP32WifiCLI::connect() { _connect(NULL, &Serial); }

void ESP32WifiCLI::setCallback(ESP32WifiCLICallbacks *pcb) { this->cb = pcb; }

void ESP32WifiCLI::setSilentMode(bool silent) { this->silent = silent; }

void ESP32WifiCLI::disableConnectInBoot() { this->connectInBoot = false; }

void ESP32WifiCLI::setSSID(String ssid) { _setSSID(NULL, &Serial); }

void ESP32WifiCLI::setPASW(String pasw) { _setPASW(NULL, &Serial); }

void ESP32WifiCLI::disconnect() { _disconnect(NULL, &Serial); }

void ESP32WifiCLI::scan() { _scanNetworks(NULL, &Serial); }

void ESP32WifiCLI::status() { _wifiStatus(NULL, &Serial); }

void ESP32WifiCLI::add(const char* command, void (*callback)(char *args, Stream *response), const char* description) {
  if (this->size_ >= WCLI_MAX_CMDS) return;
  API_tree[this->size_] = Commander::API_t{0,
                                   NULL,
                                   NULL,
                                   command,
                                   description,
                                   callback};
  this->size_++;
}

void ESP32WifiCLI::addNetworkCommand(const char* command, void (*callback)(char *args, Stream *response), const char* description) {
  if (this->isize_ >= WCLI_MAX_ICMDS) return;
  API_internal_tree[this->isize_] = Commander::API_t{0,
                                   NULL,
                                   NULL,
                                   command,
                                   description,
                                   callback};
  this->isize_++;
}
Pair <String,String> ESP32WifiCLI::parseCommand(String args){
  return ParseCommand(args);
}

String ESP32WifiCLI::parseArgument(String args){
  return ParseArgument(args);
}

void ESP32WifiCLI::enableTelnet(){
  this->shellTelnet = &shellTelnet_;
  shellTelnet->attachCommander( &wcli.commander );
  shellTelnet->attachLogo( logo );
  shellTelnet->beginServer();
  server.begin(); 
  xTaskCreate( terminalClientTask, "Terminal Client Task", 10000, NULL, 1, NULL );
}

void ESP32WifiCLI::begin(long baudrate, String app) {
  app_name = app.length() == 0 ? "wifi_cli_prefs" : app;
  WiFi.mode(WIFI_STA);
  Serial.flush();
  delay(10);
  Serial.println("");
  loadSavedNetworks();
  loadAP(getDefaultAP());
  if (connectInBoot) {
    reconnect();
    delay(10);
  }

  // main command for the base commander:
  wcli.add("nmcli", &_nmcliProcessor, "\t\tnetwork manager CLI. Type nmcli help for more info");
  // fill unsed slots. Please increase it if we have more commands
  for (int i = this->size_; i < WCLI_MAX_CMDS; i++) {
    API_tree[i] = Commander::API_t{0, NULL, NULL, "", "", NULL};
  }

  if(!silent) commander.attachDebugChannel( &Serial );
  commander.attachTreeFunction(API_tree,sizeof(API_tree)/sizeof(API_tree[0]));
  commander.init();
  
  wcli.addNetworkCommand("connect", &_nmclicon, "\tadd new WiFi: connect your_ssid password \"your_passw\"");
  wcli.addNetworkCommand("scan", &_scanNetworks, "\t\tscan WiFi networks");
  wcli.addNetworkCommand("status", &_wifiStatus, "\tWiFi status information");
  wcli.addNetworkCommand("list", &_listNetworks, "\t\tlist saved WiFi networks and its IDs");
  wcli.addNetworkCommand("mode", &_setMode, "\t\tset the default operation. modes: single|multi");
  wcli.addNetworkCommand("select", &_selectAP, "\tselect the default AP by ID");
  wcli.addNetworkCommand("up", &_connect, "\t\tenable default WiFi");
  wcli.addNetworkCommand("down", &_disconnect, "\t\tdisconnect current WiFi");
  wcli.addNetworkCommand("delete", &_deleteNetwork, "\tremove saved WiFi network by SSID");
  wcli.addNetworkCommand("help", &_printHelp, "\t\tshow nmcli commands help");

  if(!silent) internal.attachDebugChannel( &Serial );
  internal.attachTree(API_internal_tree);
  internal.init();

  shell->attachCommander( &commander );
  shell->begin( "wcli" );
}

void ESP32WifiCLI::loop() {
  shell->update();
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 1000) {
    wifiTimeStamp = millis();
    if(cb != nullptr) cb->onWifiStatus(WiFi.status() == WL_CONNECTED); 
  }
  delay(5);
}

void terminalClientTask(void *parameter) {
  for (;;) {
    client_ = server.available();

    if (client_) {
      shellTelnet_.clear();
      shellTelnet_.begin("WiFi");

      while (client_.connected()) {
        shellTelnet_.update();
        vTaskDelay(10);
      }
    }

    vTaskDelay(10);
  }
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
ESP32WifiCLI wcli;
#endif
