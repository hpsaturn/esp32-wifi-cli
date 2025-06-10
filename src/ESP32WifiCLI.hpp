#ifndef ESP32WIFICLI_HPP
#define ESP32WIFICLI_HPP

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <Shellminator.hpp>
#include <Shellminator-IO.hpp>
#include <Commander-API.hpp>
#include <Commander-IO.hpp>
#include <mutex>

#include "parser_utils.h"
#include "logo_wcli.h"

#define RW_MODE false
#define RO_MODE true

#define ESP32WIFICLI_VERSION "0.3.5"
#define ESP32WIFICLI_REVISION 049

#ifndef WCLI_MAX_CMDS
#define WCLI_MAX_CMDS 15 // user and public commands
#endif

#ifndef WCLI_SERVER_PORT
#define WCLI_SERVER_PORT 11000
#endif


#ifndef DISABLE_CLI_TELNET
#define WCLI_MAX_ICMDS 12 // internal commands
#else
#define WCLI_MAX_ICMDS 11 // internal commands
#endif

class ESP32WifiCLICallbacks;

class ESP32WifiCLI {
 public:
  Preferences wcfg;
  Commander::API_t API_tree[WCLI_MAX_CMDS];
  Commander commander;
  Commander::API_t API_internal_tree[ WCLI_MAX_ICMDS ];
  Commander internal;
  Shellminator* shell;
  Shellminator* shellTelnet;
  String prompt;
  WiFiClient* client;
  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 10000;
  bool silent = true;

  ESP32WifiCLI ();
  void begin(String prompt_name="wcli", String app_name = "wifi_cli_prefs"); 
  void loop();
  void printHelp();
  void printNetworkHelp();

  String getMode();
  void setMode(String mode = "single", Stream *response = &Serial);
  int getDefaultAP();
  void selectAP(int net, Stream* response = &Serial);
  void wifiAPConnect(bool save);
  bool loadAP(int net);
  bool loadAP(String ssid);
  void multiWiFiConnect();
  bool wifiValidation();
  void enableTelnet();
  void disableTelnet();
  bool isTelnetEnable();
  bool isTelnetRunning();
  void disableAutoConnect();
  void enableAutoConnect();
  bool isAutoConnectEnable();
  void radioOn();
  void radioOff();
  void scan();
  void setSSID(String ssid);
  void setPASW(String pasw); 
  String getCurrentSSID();
  String getCurrentPASW();
  void connect();
  void status(Stream *response = &Serial);
  void list();
  void disconnect();
  void reconnect();
  void forceTxPower();

  void deleteNetwork(String ssid, Stream *response = &Serial);
  void loadSavedNetworks(bool addAP = true, Stream *response = &Serial);
  bool isSSIDSaved(String ssid, int* net_number = nullptr);
  bool isConfigured();
  void saveNetwork(String ssid, String pasw);
  void setInt(String key, int value);
  int32_t getInt(String key, int defaultValue);
  void setString(String key, String value);
  String getString(String key, String defaultValue); 
  void clearSettings();

  void setSilentMode(bool enable = true);
 
  void add(const char* command, void(*callback)(char *args, Stream *response), const char* description = "");
  void addNetworkCommand(const char* command, void (*callback)(char *args, Stream *response), const char* description);
  
  Pair <String,String> parseCommand(String args);
  String parseArgument(String args);
  int parseEnableDisable(String args);

  void setCallback(ESP32WifiCLICallbacks* pcb);

 private:
  String app_name = "wifi_cli_prefs";
  int size_ = 0;
  int isize_ = 0;
  std::mutex cli_mtx;
  bool isForcedTxPower = false;

  String getNetKeyName(uint8_t net);

  ESP32WifiCLICallbacks* cb = nullptr;
};

class ESP32WifiCLICallbacks {
public:
    virtual ~ESP32WifiCLICallbacks() {};
    virtual void onWifiStatus(bool isConnected);
    virtual void onHelpShow();
    virtual void onNewWifi(String ssid, String passw);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
extern ESP32WifiCLI wcli;
#endif

#endif
