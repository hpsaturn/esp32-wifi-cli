#ifndef ESP32WIFICLI_HPP
#define ESP32WIFICLI_HPP

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <Shellminator.hpp>
#include <Shellminator-IO.hpp>
#include <Commander-API.hpp>
#include <Commander-IO.hpp>

#include "parser_utils.h"
#include "logo_wcli.h"

#define RW_MODE false
#define RO_MODE true

#define ESP32WIFICLI_VERSION "0.3.0"
#define ESP32WIFICLI_REVISION 044

#ifndef WCLI_MAX_CMDS
#define WCLI_MAX_CMDS 15 // user and public commands
#endif

#ifndef WCLI_SERVER_PORT
#define WCLI_SERVER_PORT 11000
#endif


#ifndef DISABLE_CLI_TELNET
#define WCLI_MAX_ICMDS 11 // internal commands
#else
#define WCLI_MAX_ICMDS 10 // internal commands
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
  bool connectInBoot = true;

  ESP32WifiCLI ();
  void begin(String prompt_name="wcli", String app_name = "wifi_cli_prefs");
  void add(const char* command, void(*callback)(char *args, Stream *response), const char* description = "");
  Pair <String,String> parseCommand(String args);
  String parseArgument(String args);
  void loop();
  void printHelp();
  void printNetworkHelp();
  void scan();
  void setSSID(String ssid);
  void setPASW(String pasw);
  void connect();
  void status();
  void list();
  void selectAP(int net, Stream* response = &Serial);
  void disconnect();
  void reconnect();
  void multiWiFiConnect();
  void setMode(String mode = "single", Stream *response = &Serial);
  void wifiAPConnect(bool save);
  bool wifiValidation();
  bool loadAP(int net);
  void deleteNetwork(String ssid, Stream *response = &Serial);
  void loadSavedNetworks(bool addAP = true, Stream *response = &Serial);
  bool isSSIDSaved(String ssid);
  bool isConfigured();
  void saveNetwork(String ssid, String pasw);
  void setInt(String key, int value);
  int32_t getInt(String key, int defaultValue);
  void setString(String key, String value);
  String getString(String key, String defaultValue);
  void setSilentMode(bool enable = true);
  void disableConnectInBoot();
  String getMode();
  int getDefaultAP();
  void clearSettings();
  void enableTelnet();
  void disableTelnet();
  void addNetworkCommand(const char* command, void (*callback)(char *args, Stream *response), const char* description);

  void setCallback(ESP32WifiCLICallbacks* pcb);

 private:
  String app_name;
  int size_ = 0;
  int isize_ = 0;

  String getNetKeyName(int net);

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
