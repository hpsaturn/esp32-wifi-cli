#ifndef ESP32WIFICLI_HPP
#define ESP32WIFICLI_HPP

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <SerialTerminal.hpp>

#define RW_MODE false
#define RO_MODE true

#define ESP32WIFICLI_VERSION "0.2.2"
#define ESP32WIFICLI_REVISION 043

class ESP32WifiCLICallbacks;

class ESP32WifiCLI {
 public:
  Preferences cfg;
  maschinendeck::SerialTerminal* term;
  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 10000;
  bool silent = false;
  bool connectInBoot = true;

  void begin(long baudrate = 0, String app_name = "wifi_cli_prefs");
  void loop();
  void printHelp();
  void printWifiStatus();
  void scan();
  void setSSID(String ssid);
  void setPASW(String pasw);
  void connect();
  void status();
  void list();
  void selectAP(int net);
  void disconnect();
  void reconnect();
  void multiWiFiConnect();
  void setMode(String mode);
  void wifiAPConnect(bool save);
  bool wifiValidation();
  bool loadAP(int net);
  void deleteNetwork(String ssid);
  void loadSavedNetworks(bool addAP = true);
  bool isSSIDSaved(String ssid);
  bool isConfigured();
  void saveNetwork(String ssid, String pasw);
  void setInt(String key, int value);
  int32_t getInt(String key, int defaultValue);
  void setString(String key, String value);
  String getString(String key, String defaultValue);
  void setSilentMode(bool enable);
  void disableConnectInBoot();
  String getMode();
  int getDefaultAP();
  void clearSettings();

  void setCallback(ESP32WifiCLICallbacks* pcb);

 private:
  String app_name;
  String temp_ssid = "";
  String temp_pasw = "";

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
