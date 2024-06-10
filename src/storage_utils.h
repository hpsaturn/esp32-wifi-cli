#include "ESP32WifiCLI.hpp"


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