#include <ESP32WifiCLI.hpp>
#include "storage_utils.h"
#include "telnet_cli.h"

Shellminator shell_( &Serial );

String temp_ssid = "";
String temp_pasw = "";

ESP32WifiCLI::ESP32WifiCLI() {
  this->shell = &shell_;
  shell->attachLogo(logo_wcli);
#ifndef DISABLE_CLI_TELNET
  this->client = &client_;
#endif
}

void ESP32WifiCLI::printHelp() {
  this->shell->printHelp();
}

String ESP32WifiCLI::getNetKeyName(uint8_t net) {
  if (net > 99) return "";
  char key[11];
  sprintf(key, "key_net%02d", net);
  return String(key);
}

void ESP32WifiCLI::loadSavedNetworks(bool addAP, Stream *response) {
  wcfg.begin(app_name.c_str(), RO_MODE);
  int net = 1;
  int default_net = wcfg.getInt("default_net", 1);
  if (!addAP) response->println("\nSaved networks:\n");
  while (wcfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid = wcfg.getString(String(key + "_ssid").c_str(), "");
    String pasw = wcfg.getString(String(key + "_pasw").c_str(), "");
    String dfl = (net == default_net) ? "*" : " ";
    if (!addAP) response->printf("(%s) %d: [%s]\r\n", dfl.c_str(), net, ssid.c_str());
    if (addAP) wifiMulti.addAP(ssid.c_str(), pasw.c_str());
    net++;
  }
  if(!addAP)response->println("");
  wcfg.end();
}

void ESP32WifiCLI::deleteNetwork(String ssid, Stream *response) {
  std::lock_guard<std::mutex> lck(cli_mtx);
  if (ssid.length() == 0) {
    response->println("\nSSID is empty, please set a valid SSID into quotes");
    return;
  }
  int net = 1;
  bool dropped = false;
  wcfg.begin(app_name.c_str(), RW_MODE);
  while (wcfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = wcfg.getString(String(key + "_ssid").c_str(), "");
    if (!dropped && ssid_.equals(ssid)) {
      response->printf("\nDeleting network [%s][%s]\r\n", key.c_str(), ssid.c_str());
      wcfg.remove(String(key + "_ssid").c_str());
      wcfg.remove(String(key + "_pasw").c_str());
      dropped = true;
      int net_count = wcfg.getInt("net_count", 0);
      wcfg.putInt("net_count", net_count - 1);
      int default_net = wcfg.getInt("default_net", 1);
      if (net - 1 == default_net && net_count > 1 ) wcfg.putInt("default_net", net_count - 1);
      continue;
    }
    if (dropped) {
      String ssid_drop = wcfg.getString(String(key + "_ssid").c_str(), "");
      String pasw_drop = wcfg.getString(String(key + "_pasw").c_str(), "");
      String key_drop = getNetKeyName(net - 2);
      // response->printf("ssid drop: [%s][%d][%s]\r\n",ssid_drop.c_str(), net - 2, key_drop.c_str());
      wcfg.putString(String(key_drop + "_ssid").c_str(), ssid_drop);
      wcfg.putString(String(key_drop + "_pasw").c_str(), pasw_drop);
      // response->printf("remove key: [%d][%s]\r\n",net - 1, key.c_str());
      int default_net = wcfg.getInt("default_net", 1);
      if (net - 1 == default_net) wcfg.putInt("default_net", net - 2);
      wcfg.remove(String(key + "_ssid").c_str());
      wcfg.remove(String(key + "_pasw").c_str());
    }
  }
  wcfg.end();
  loadSavedNetworks(false, response);
}

void ESP32WifiCLI::saveNetwork(String ssid, String pasw) {
  std::lock_guard<std::mutex> lck(cli_mtx);
  wcfg.begin(app_name.c_str(), RW_MODE);
  int net = wcfg.getInt("net_count", 0);
  String key = getNetKeyName(net + 1);
  if (!silent) Serial.printf("Saving network: [%s][%s]\r\n", ssid.c_str(), pasw.c_str());
  wcfg.putString(String(key + "_ssid").c_str(), ssid);
  wcfg.putString(String(key + "_pasw").c_str(), pasw);
  wcfg.putInt("net_count", net + 1);
  wcfg.putInt("default_net", net + 1);
  wcfg.end();
}

bool ESP32WifiCLI::isSSIDSaved(String ssid, int *net_mumber) {
  wcfg.begin(app_name.c_str(), RO_MODE);
  bool isSaved = false;
  int net = 1;
  while (wcfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = wcfg.getString(String(key + "_ssid").c_str(), "");
    if (ssid_.equals(ssid)) {
      isSaved = true;
      if (net_mumber!=nullptr) *net_mumber = net-1;
      break;
    }
  }
  wcfg.end();
  return isSaved;
}

bool ESP32WifiCLI::wifiValidation() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!silent) Serial.println("connected!");
    if (!silent) status();
    return true;
  } else {
    Serial.println(" connection failed!");
    return false;
  }
}

/**
 * @brief Main method for save and connect to a WiFi network
 * @param save if is true save the network, false only connect
 */
void ESP32WifiCLI::wifiAPConnect(bool save) {
  std::unique_lock<std::mutex> lck(cli_mtx);
  if (temp_ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes\n");
    return;
  }
  if (!silent) Serial.print("\nConnecting to " + temp_ssid + "...");
  if (save) {
    wifiMulti.addAP(temp_ssid.c_str(), temp_pasw.c_str());
  }
  int retry = 0;
  WiFi.begin(temp_ssid.c_str(), temp_pasw.c_str());

  #if CONFIG_IDF_TARGET_ESP32C3
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif

  while (WiFi.status() != WL_CONNECTED && retry++ < 20) {  // M5Atom will connect automatically
    delay(1000);
    if (!silent) Serial.print(".");
  }
  delay(100);
  if (wifiValidation() && save) {
    lck.unlock();
    saveNetwork(temp_ssid, temp_pasw);
    if(cb != nullptr) cb->onNewWifi(temp_ssid, temp_pasw);
  }
  else if (save) {
    loadAP(getDefaultAP());
    log_w("network not saved!");
  }
}

bool ESP32WifiCLI::isConfigured() {
  wcfg.begin("wifi_cli_prefs", RO_MODE);
  String key = getNetKeyName(1);
  bool isConfigured = wcfg.isKey(String(key + "_ssid").c_str());
  wcfg.end();
  return isConfigured;
}

bool ESP32WifiCLI::loadAP(int net) {
  wcfg.begin(app_name.c_str(), RO_MODE);
  String key = getNetKeyName(net);
  if (!wcfg.isKey(String(key + "_ssid").c_str())) {
    wcfg.end();
    return false;
  }
  temp_ssid = wcfg.getString(String(key + "_ssid").c_str(), "");
  temp_pasw = wcfg.getString(String(key + "_pasw").c_str(), "");
  log_v("Default AP: %i: [%s]", net, temp_ssid.c_str());
  wcfg.end();
  return true;
}

bool ESP32WifiCLI::loadAP(String ssid) {
  int net = 0; 
  if(isSSIDSaved(ssid,&net)) return loadAP(net);
  return false;
}

void ESP32WifiCLI::selectAP(int net, Stream *response) {
  std::unique_lock<std::mutex> lck(cli_mtx);
  if (!loadAP(net)) {
    response->println("\nNetwork not found");
    return;
  }
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putInt("default_net", net);
  wcfg.end();
  loadSavedNetworks(false, response);
  lck.unlock();
  if(cb != nullptr) cb->onNewWifi(temp_ssid, temp_pasw);
}

int ESP32WifiCLI::getDefaultAP() {
  wcfg.begin(app_name.c_str(), RO_MODE);
  int net = wcfg.getInt("default_net", 1);
  wcfg.end();
  return net;
}

String ESP32WifiCLI::getMode() {
  wcfg.begin(app_name.c_str(), RO_MODE);
  String mode = wcfg.getString("mode", "single");
  wcfg.end();
  return mode;
}

void ESP32WifiCLI::setMode(String mode, Stream *response) {
  std::lock_guard<std::mutex> lck(cli_mtx);
  wcfg.begin(app_name.c_str(), RW_MODE);
  if (mode.equals("single")) {
    wcfg.putString("mode", "single");
  } else if (mode.equals("multi")) {
    wcfg.putString("mode", "multi");
  } else if (mode.equals("")) {
    response->printf("\nCurrent mode: %s\r\n", wcfg.getString("mode", "single").c_str());
  } else {
    response->println("\nInvalid mode, please use single/multi parameter");
  }
  wcfg.end();
}

void ESP32WifiCLI::multiWiFiConnect() {
  int retry = 0;
  if (!silent) Serial.print("\nConnecting in MultiAP mode..");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED && retry++ < 10) {
    delay(500);
    if (!silent) Serial.print(".");
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

void ESP32WifiCLI::enableAutoConnect() {
  std::lock_guard<std::mutex> lck(cli_mtx);
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putBool("auto_connect",true);
  wcfg.end();
}

void ESP32WifiCLI::disableAutoConnect() { 
  std::lock_guard<std::mutex> lck(cli_mtx);
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putBool("auto_connect",false);
  wcfg.end();
}

bool ESP32WifiCLI::isAutoConnectEnable() { 
  wcfg.begin(app_name.c_str(), RO_MODE);
  bool enable = wcfg.getBool("auto_connect", true);
  wcfg.end();
  return enable;
}

String ESP32WifiCLI::getCurrentSSID() {
  return temp_ssid;
}

String ESP32WifiCLI::getCurrentPASW() {
  return temp_pasw;
}

String autoConnectStatus(){
  return wcli.isAutoConnectEnable() ? "\033[0;32menabled\033[0;37m" : "\033[0;31mdisabled\033[0;37m";
}

void ESP32WifiCLI::printNetworkHelp() {
  this->shell->attachCommander(&internal);
  this->shell->printHelp();
  this->shell->attachCommander(&commander);

#ifndef DISABLE_CLI_TELNET
  if (shellTelnet == nullptr) return;
  this->shellTelnet->attachCommander(&internal);
  this->shellTelnet->printHelp();
  this->shellTelnet->attachCommander(&commander);
#endif
}

void _nmcli_auto(char *args, Stream *response) {
  int enable = wcli.parseEnableDisable(args);
  if (enable == 1) {
    response->println("enable auto connect in boot");
    wcli.enableAutoConnect();
  }
  else if (enable == 0) {
    response->println("disable auto connect in boot");
    wcli.disableAutoConnect();
  }
  else {
    response->println("invalid syntax: use\033[0;33m nmcli auto [enable|disable]\033[0m");
    response->printf("auto connect is: %s\r\n", autoConnectStatus().c_str());
  }
}

void _nmcli_scan(char *args, Stream *response) {
  wcli.radioOn();
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

void _nmcli_help(char* args, Stream *response) {
  wcli.printNetworkHelp();
}

void ESP32WifiCLI::setSSID(String ssid) {  
  if (ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes");
  } else {
    temp_ssid = ssid;
    if (!wcli.silent) Serial.println("set ssid to: " + temp_ssid);
  }
}

void ESP32WifiCLI::setPASW(String pasw) {  
  temp_pasw = pasw;
  if (!wcli.silent) Serial.println("set password to: " + temp_pasw);
}

void _nmcli_down(char *args, Stream *response) {
  response->println("\nDisconnecting...");
  WiFi.disconnect();
}

void _nmcli_up(char *args, Stream *response) {
  if (WiFi.status() == WL_CONNECTED && temp_ssid == WiFi.SSID()) {
    response->println("\nWiFi is already connected");
    return;
  } else if (WiFi.status() == WL_CONNECTED) {
    _nmcli_down(args, response);
    delay(1000);
  }
  if (wcli.getMode().equals("single")) {
    if (temp_ssid.length() == 0) {
      response->println("\nSSID is empty, please set a valid SSID into quotes\n");
      return;
    }
    if (wcli.isSSIDSaved(temp_ssid)) {
      wcli.wifiAPConnect(false);
      if (wcli.isTelnetEnable() && !wcli.isTelnetRunning()) wcli.enableTelnet();
      return;
    } else {
      wcli.wifiAPConnect(true);
    }
  } else {
    wcli.multiWiFiConnect();
  }
  if (wcli.isTelnetEnable() && !wcli.isTelnetRunning()) wcli.enableTelnet();
}

void _nmcli_connect(char *args, Stream *response) {
  char *ssid = NULL;
  char *password = NULL;

  if (!extract_connect_parames(args, &ssid, &password, response)) return;

  if (ssid != NULL && password != NULL) {
    size_t len = strlen(password);
    if (password[0] == '"' && password[len - 1] == '"') {
      memmove(password, password + 1, len - 2);
      password[len - 2] = '\0';
    }
    temp_ssid = String(ssid);
    temp_pasw = String(password);
    free(ssid);
    free(password);
  }
  else{
    response->println("Invalid command syntax!");
    return;
  }
  _nmcli_up(args,response);
}

void _nmcli_list(char *args, Stream *response) {
  wcli.loadSavedNetworks(false, response);
}

void _nmcli_status(char *args, Stream *response) {
  response->println("\nWiFi SSID \t: ["+WiFi.SSID() + "]");
  response->print("IP address  \t: ");
  response->println(WiFi.localIP());
  response->printf("RSSI signal \t: %03i\r\n",WiFi.RSSI());
  response->println("MAC Address\t: "+WiFi.macAddress());
  response->printf("Hostname \t: %s\r\n",WiFi.getHostname());
  response->printf("Auto connect \t: %s\r\n",autoConnectStatus().c_str());
  #ifndef DISABLE_CLI_TELNET
  response->printf("Telnet status\t: %s\r\n",telnetStatus().c_str());
  #endif
  response->print("Memory free\t: ");
  response->println(String(ESP.getFreeHeap() / 1024) + "Kb");
}

void _nmcli_delete(char *args, Stream *response) {
  String ssid = ParseArgument(args);
  wcli.deleteNetwork(ssid, response);
}

void _nmcli_select(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  int net = operands.first().toInt();
  wcli.selectAP(net, response);
}

void _nmcli_mode(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  wcli.setMode(operands.first(), response);
}

void _nmcli(char *args, Stream *response) {
  if (strlen(args) == 0) {
    _nmcli_help(args, response);
  } else
    wcli.internal.execute(args, response);
}

void ESP32WifiCLI::connect() { _nmcli_up(NULL, &Serial); }

void ESP32WifiCLI::setCallback(ESP32WifiCLICallbacks *pcb) { this->cb = pcb; }

void ESP32WifiCLI::setSilentMode(bool silent) { this->silent = silent; }

void ESP32WifiCLI::disconnect() { _nmcli_down(NULL, &Serial); }

void ESP32WifiCLI::scan() { _nmcli_scan(NULL, &Serial); }

void ESP32WifiCLI::status(Stream *response) { _nmcli_status(NULL, response); }

void ESP32WifiCLI::list() { loadSavedNetworks(false); }

Pair<String, String> ESP32WifiCLI::parseCommand(String args) { return ParseCommand(args); }

String ESP32WifiCLI::parseArgument(String args) { return ParseArgument(args); }

int ESP32WifiCLI::parseEnableDisable(String args) { return ParseEnableDisable(args); }

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

void ESP32WifiCLI::radioOff(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void ESP32WifiCLI::radioOn(){
  WiFi.mode(WIFI_STA);
}

void ESP32WifiCLI::begin(String prompt_name, String app) {
  app_name = app.length() == 0 ? app_name : app;
  prompt = prompt_name;
  Serial.flush();
  delay(10);
  Serial.println("");
  loadSavedNetworks();
  loadAP(getDefaultAP());
  if (isAutoConnectEnable()) {
    log_i("reconnecting..");
    radioOn();
    reconnect();
    delay(10);
  } else {
    log_i("turning radio off..");
    radioOff();
  }

  // main command for the base commander:
  wcli.add("nmcli", &_nmcli, "\t\tnetwork manager CLI. Type nmcli help for more info");
  // fill unsed slots. Please increase it if we have more commands
  for (int i = this->size_; i < WCLI_MAX_CMDS; i++) {
    API_tree[i] = Commander::API_t{0, NULL, NULL, "", "", NULL};
  }

  if(!silent) commander.attachDebugChannel( &Serial );
  commander.attachTreeFunction(API_tree,sizeof(API_tree)/sizeof(API_tree[0]));
  commander.init();
  
  wcli.addNetworkCommand("connect", &_nmcli_connect, "\tadd new WiFi: \033[0;33mconnect your_ssid password \"your_passw\"\033[0m");
  wcli.addNetworkCommand("scan", &_nmcli_scan, "\t\tscan WiFi networks");
  wcli.addNetworkCommand("status", &_nmcli_status, "\tWiFi status information");
  wcli.addNetworkCommand("list", &_nmcli_list, "\t\tlist saved WiFi networks and its IDs");
  wcli.addNetworkCommand("mode", &_nmcli_mode, "\t\tset the default operation. modes: single|multi");
  wcli.addNetworkCommand("auto", &_nmcli_auto, "\t\tenable/disable auto connect in boot");
  wcli.addNetworkCommand("select", &_nmcli_select, "\tselect the default AP by ID");
  wcli.addNetworkCommand("up", &_nmcli_up, "\t\tenable default WiFi");
  wcli.addNetworkCommand("down", &_nmcli_down, "\t\tdisconnect current WiFi");
  wcli.addNetworkCommand("delete", &_nmcli_delete, "\tremove saved WiFi network by SSID");
  wcli.addNetworkCommand("help", &_nmcli_help, "\t\tshow nmcli commands help");
  
  #ifndef DISABLE_CLI_TELNET 
  wcli.addNetworkCommand("telnet", &_nmcli_telnet, "\tenable/disable Telnet service");
  if (isTelnetEnable()) wcli.enableTelnet();
  #endif

  if(!silent) internal.attachDebugChannel( &Serial );
  internal.attachTree(API_internal_tree);
  internal.init();

  shell->attachCommander( &commander );
  shell->begin(prompt.c_str() );
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

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
ESP32WifiCLI wcli;
#endif
