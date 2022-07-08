#ifndef ESP32WIFICLI_HPP
#define ESP32WIFICLI_HPP

#include <Preferences.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <SerialTerminal.hpp>

#define RW_MODE false
#define RO_MODE true

class ESP32WifiCLICallbacks;

class ESP32WifiCLI {
 public:
  Preferences cfg;
  maschinendeck::SerialTerminal* term;
  WiFiMulti wifiMulti;
  const uint32_t connectTimeoutMs = 10000;

  void begin(long baudrate = 0);
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
  void saveNetwork(String ssid, String pasw);
  String getMode();
  int getDefaultAP();

  void setCallback(ESP32WifiCLICallbacks* pcb);

 private:
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
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_ESP32WIFICLI)
extern ESP32WifiCLI wcli;
#endif

#endif
