#include <ESP32WifiCLI.hpp>

void ESP32WifiCLI::printWifiStatus() {
  Serial.print("\nWiFi SSID \t: [");
  Serial.println(WiFi.SSID() + "]");  // Output Network name.
  Serial.print("IP address  \t: ");
  Serial.println(WiFi.localIP());  // Output IP Address.
  Serial.print("RSSI signal \t: ");
  Serial.println(WiFi.RSSI());  // Output signal strength.
  Serial.print("MAC Address\t: ");
  Serial.println(WiFi.macAddress());  // Output MAC address.
  Serial.print("Hostname \t: ");
  Serial.println(WiFi.getHostname());  // Output hostname.
  Serial.println("");
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

void ESP32WifiCLI::status() {
  if (WiFi.status() == WL_CONNECTED) {
    printWifiStatus();
  } else {
    Serial.println("\nWiFi is not connected");
  }
}

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

void ESP32WifiCLI::loop() {
  term->loop();
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 1000) {
    wifiTimeStamp = millis();
    if(cb != nullptr) cb->onWifiStatus(WiFi.status() == WL_CONNECTED); 
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

void _scanNetworks(String opts) {
  wcli.scan();
}

void _printHelp(String opts) {
  wcli.printHelp();
}

void _setSSID(String opts) {
  String ssid = maschinendeck::SerialTerminal::ParseArgument(opts);
  wcli.setSSID(ssid);
}

void _setPASW(String opts) {
  String pasw = maschinendeck::SerialTerminal::ParseArgument(opts);
  wcli.setPASW(pasw);
}

void _connect(String opts) {
  wcli.connect();
}

void _disconnect(String opts) {
  wcli.disconnect();
}

void _listNetworks(String opts) {
  wcli.loadSavedNetworks(false);
}

void _wifiStatus(String opts) {
  wcli.status();
}

void _deleteNetwork(String opts) {
  String ssid = maschinendeck::SerialTerminal::ParseArgument(opts);
  wcli.deleteNetwork(ssid);
}

void _selectAP(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  int net = operands.first().toInt();
  wcli.selectAP(net);
}

void _setMode(String opts) {
  maschinendeck::Pair<String, String> operands = maschinendeck::SerialTerminal::ParseCommand(opts);
  wcli.setMode(operands.first());
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
  term = new maschinendeck::SerialTerminal(baudrate);
  term->add("help", &_printHelp, "\tshow detail usage information");
  term->add("setSSID", &_setSSID, "\tset the Wifi SSID");
  term->add("setPASW", &_setPASW, "\tset the WiFi password");
  term->add("connect", &_connect, "\tsave and connect to WiFi network");
  term->add("list", &_listNetworks, "\tlist saved WiFi networks");
  term->add("select", &_selectAP, "\tselect the default AP (default: last)");
  term->add("mode", &_setMode, "\tset the default operation single/multi AP (slow)");
  term->add("scan", &_scanNetworks, "\tscan WiFi networks");
  term->add("status", &_wifiStatus, "\tWiFi status information");
  term->add("disconnect", &_disconnect, "WiFi disconnect");
  term->add("delete", &_deleteNetwork, "\tremove saved WiFi network by SSID\r\n");
}

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
ESP32WifiCLI wcli;
#endif
