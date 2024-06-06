#include <ESP32WifiCLI.hpp>

Shellminator shell_( &Serial );

void ESP32WifiCLI::printWifiStatus() {
  Serial.print("\nWiFi SSID \t: [");
  Serial.println(WiFi.SSID() + "]");  // Output Network name.
  Serial.print("IP address  \t: ");
  Serial.println(WiFi.localIP());     // Output IP Address.
  Serial.print("RSSI signal \t: ");
  Serial.println(WiFi.RSSI());        // Output signal strength.
  Serial.print("MAC Address\t: ");
  Serial.println(WiFi.macAddress());  // Output MAC address.
  Serial.print("Hostname \t: ");
  Serial.println(WiFi.getHostname()); // Output hostname.
  Serial.print("Memory free\t: ");
  Serial.println(String(ESP.getFreeHeap() / 1024) + "Kb");
}

void ESP32WifiCLI::printHelp() {
  Serial.println("\nESP32WifiCLI Usage:\n");
  Serial.println("setSSID \"YOUR SSID\"\tset the SSID into quotes");
  Serial.println("setPASW \"YOUR PASW\"\tset the password into quotes");
  Serial.println("connect  \t\tsave and connect to the network");
  Serial.println("list \t\t\tlist all saved networks");
  Serial.println("select <number>   \tselect the default AP (default: last saved)");
  Serial.println("mode <single/multi>\tconnection mode. Multi AP is a little slow");
  Serial.println("scan \t\t\tscan for available networks");
  Serial.println("status \t\t\tprint the current WiFi status");
  Serial.println("disconnect \t\tdisconnect from the network");
  Serial.println("delete \"SSID\"\t\tremove saved network");
  Serial.println("help \t\t\tprint this help");
  if (cb != nullptr) cb->onHelpShow();
}

void ESP32WifiCLI::scan() {
  int n = WiFi.scanNetworks();
  Serial.print("\nscan done: ");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found\n");
    for (int i = 0; i < n; ++i) {
      String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[O]" : "[*]";
      Serial.printf("%02d %s[%i][%s]\r\n", i + 1, enc.c_str(), WiFi.RSSI(i), WiFi.SSID(i).c_str());
      delay(10);
    }
  }
}

void ESP32WifiCLI::status() { printWifiStatus(); }

String ESP32WifiCLI::getNetKeyName(int net) {
  if (net > 99) return "";
  char key[11];
  sprintf(key, "key_net%02d", net);
  return String(key);
}

void ESP32WifiCLI::loadSavedNetworks(bool addAP) {
  cfg.begin(app_name.c_str(), RO_MODE);
  int net = 1;
  int default_net = cfg.getInt("default_net", 1);
  if (!addAP) Serial.println("\nSaved networks:\n");
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid = cfg.getString(String(key + "_ssid").c_str(), "");
    String pasw = cfg.getString(String(key + "_pasw").c_str(), "");
    String dfl = (net == default_net) ? "*" : " ";
    if (!addAP) Serial.printf("(%s) %d: [%s]\r\n", dfl.c_str(), net, ssid.c_str());
    if (addAP) wifiMulti.addAP(ssid.c_str(), pasw.c_str());
    net++;
  }
  if(!addAP)Serial.println("");
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

void ESP32WifiCLI::deleteNetwork(String ssid) {
  if (ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes");
    return;
  }
  int net = 1;
  bool dropped = false;
  cfg.begin(app_name.c_str(), RW_MODE);
  while (cfg.isKey(String(getNetKeyName(net) + "_ssid").c_str())) {
    String key = getNetKeyName(net++);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (!dropped && ssid_.equals(ssid)) {
      Serial.printf("\nDeleting network [%s][%s]\r\n", key.c_str(), ssid.c_str());
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
      // Serial.printf("ssid drop: [%s][%d][%s]\r\n",ssid_drop.c_str(), net - 2, key_drop.c_str());
      cfg.putString(String(key_drop + "_ssid").c_str(), ssid_drop);
      cfg.putString(String(key_drop + "_pasw").c_str(), pasw_drop);
      // Serial.printf("remove key: [%d][%s]\r\n",net - 1, key.c_str());
      int default_net = cfg.getInt("default_net", 1);
      if (net - 1 == default_net) cfg.putInt("default_net", net - 2);
      cfg.remove(String(key + "_ssid").c_str());
      cfg.remove(String(key + "_pasw").c_str());
    }
  }
  cfg.end();
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

void ESP32WifiCLI::setSSID(String ssid) {
  temp_ssid = ssid;
  if (temp_ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes");
  } else {
    Serial.println("\nset ssid to: " + temp_ssid);
  }
}

void ESP32WifiCLI::setPASW(String pasw) {
  temp_pasw = pasw;
  Serial.println("\nset password to: " + temp_pasw);
}

void ESP32WifiCLI::disconnect() {
  Serial.println("\nDisconnecting...");
  WiFi.disconnect();
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

void ESP32WifiCLI::selectAP(int net) {
  if (!loadAP(net)) {
    Serial.println("\nNetwork not found");
    return;
  }
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putInt("default_net", net);
  cfg.end();
  list();
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

void ESP32WifiCLI::setMode(String mode) {
  cfg.begin(app_name.c_str(), RW_MODE);
  if (mode.equals("single")) {
    cfg.putString("mode", "single");
  } else if (mode.equals("multi")) {
    cfg.putString("mode", "multi");
  } else if (mode.equals("")) {
    Serial.printf("\nCurrent mode: %s\r\n", cfg.getString("mode", "single").c_str());
  } else {
    Serial.println("\nInvalid mode, please use single/multi parameter");
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

void ESP32WifiCLI::connect() {
  if (WiFi.status() == WL_CONNECTED && temp_ssid == WiFi.SSID()) {
    Serial.println("\nWiFi is already connected");
    return;
  } else if (WiFi.status() == WL_CONNECTED) {
    disconnect();
    delay(1000);
  }
  if (getMode().equals("single")) {
    if (temp_ssid.length() == 0) {
      Serial.println("\nSSID is empty, please set a valid SSID into quotes\n");
      return;
    }
    if (isSSIDSaved(temp_ssid)) {
      wifiAPConnect(false);
      return;
    } else {
      wifiAPConnect(true);
    }
  } else {
    multiWiFiConnect();
  }
}

void ESP32WifiCLI::setCallback(ESP32WifiCLICallbacks* pcb) {
  this->cb = pcb;
}

void ESP32WifiCLI::setInt(String key, int value) {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putInt(key.c_str(), value);
  cfg.end();
}

int32_t ESP32WifiCLI::getInt(String key, int defaultValue) {
  cfg.begin(app_name.c_str(), RO_MODE);
  int32_t out = cfg.getInt(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void ESP32WifiCLI::setString(String key, String value) {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.putString(key.c_str(), value);
  cfg.end();
}

String ESP32WifiCLI::getString(String key, String defaultValue) {
  cfg.begin(app_name.c_str(), RO_MODE);
  String out = cfg.getString(key.c_str(), defaultValue);
  cfg.end();
  return out;
}

void ESP32WifiCLI::clearSettings() {
  cfg.begin(app_name.c_str(), RW_MODE);
  cfg.clear();
  cfg.end();
  if (!silent) Serial.println("Settings cleared!");
}

void ESP32WifiCLI::setSilentMode(bool silent) {
  this->silent = silent;
}

void ESP32WifiCLI::disableConnectInBoot(){
  this->connectInBoot = false;
}

void _scanNetworks(char *args, Stream *response) {
  wcli.scan();
}

void _printHelp(char* args, Stream *response) {
  wcli.printHelp();
}

void _setSSID(char *args, Stream *response) {
  String ssid = ParseArgument(args);
  wcli.setSSID(ssid);
}

void _setPASW(char *args, Stream *response) {
  String pasw = ParseArgument(args);
  wcli.setPASW(pasw);
}

void _connect(char *args, Stream *response) {
  wcli.connect();
}

void _disconnect(char *args, Stream *response) {
  wcli.disconnect();
}

void _listNetworks(char *args, Stream *response) {
  wcli.loadSavedNetworks(false);
}

void _wifiStatus(char *args, Stream *response) {
  wcli.status();
}

void _deleteNetwork(char *args, Stream *response) {
  String ssid = ParseArgument(args);
  wcli.deleteNetwork(ssid);
}

void _selectAP(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  int net = operands.first().toInt();
  wcli.selectAP(net);
}

void _setMode(char *args, Stream *response) {
  Pair<String, String> operands = ParseCommand(args);
  wcli.setMode(operands.first());
}

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

Pair <String,String> ESP32WifiCLI::parseCommand(String args){
  return ParseCommand(args);
}

String ESP32WifiCLI::parseArgument(String args){
  return ParseArgument(args);
}

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
  // wcli.add("help", &_printHelp, "\tshow detail usage information");
  wcli.add("wssid", &_setSSID, "\t\tset the Wifi SSID");
  wcli.add("wpassw", &_setPASW, "\tset the WiFi password");
  wcli.add("wconnect", &_connect, "\tsave WiFi ssid & passw and connect");
  wcli.add("wlist", &_listNetworks, "\t\tlist saved WiFi networks");
  wcli.add("wselect", &_selectAP, "\tselect the default AP (default: last)");
  wcli.add("wmode", &_setMode, "\t\tset the default operation single/multi AP (slow)");
  wcli.add("wscan", &_scanNetworks, "\t\tscan WiFi networks");
  wcli.add("wstatus", &_wifiStatus, "\tWiFi status information");
  wcli.add("wdisconnect", &_disconnect, "\tWiFi disconnect");
  wcli.add("wdelete", &_deleteNetwork, "\tremove saved WiFi network by SSID\r\n");

  for (int i = this->size_ - 1; i < WCLI_MAX_CMDS; i++) {
    API_tree[i] = Commander::API_t{0, NULL, NULL, "", "", NULL};
  }

  this->shell = &shell_;
  shell->clear();
  shell->attachLogo(logo);
  commander.attachDebugChannel( &Serial );
  commander.attachTreeFunction(API_tree,sizeof(API_tree)/sizeof(API_tree[0]));
  commander.init();

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
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
ESP32WifiCLI wcli;
#endif
