#include "ESP32WifiCLI.hpp"


void ESP32WifiCLI::setInt(String key, int value) {
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putInt(key.c_str(), value);
  wcfg.end();
}

int32_t ESP32WifiCLI::getInt(String key, int defaultValue) {
  wcfg.begin(app_name.c_str(), RO_MODE);
  int32_t out = wcfg.getInt(key.c_str(), defaultValue);
  wcfg.end();
  return out;
}

void ESP32WifiCLI::setString(String key, String value) {
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.putString(key.c_str(), value);
  wcfg.end();
}

String ESP32WifiCLI::getString(String key, String defaultValue) {
  wcfg.begin(app_name.c_str(), RO_MODE);
  String out = wcfg.getString(key.c_str(), defaultValue);
  wcfg.end();
  return out;
}

void ESP32WifiCLI::clearSettings() {
  wcfg.begin(app_name.c_str(), RW_MODE);
  wcfg.clear();
  wcfg.end();
  if (!silent) Serial.println("Settings cleared!");
}