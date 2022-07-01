
/*********************************************************************
 This sample file is part of the esp32-wifi-cli:
 Copyright (c) 2022, @hpsaturn, Antonio Vanegas
 https://hpsaturn.com, All rights reserved.
 https://github.com/hpsaturn/esp32-wifi-cli

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/

#include <M5Atom.h>
#include <SerialTerminal.hpp>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>

maschinendeck::SerialTerminal* term;
WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 10000;

#define RW_MODE false
#define RO_MODE true

Preferences cfg;

String temp_ssid = "";
String temp_pasw = "";

void printWifiSettings() {
  Serial.print("\nWiFi SSID \t: [");
  Serial.println(WiFi.SSID()+"]");       //Output Network name.
  Serial.print("IP address  \t: ");
  Serial.println(WiFi.localIP());        //Output IP Address.
  Serial.print("RSSI signal \t: ");
  Serial.println(WiFi.RSSI());           //Output signal strength.
  Serial.print("MAC Address\t: ");
  Serial.println(WiFi.macAddress());     //Output MAC address.
  Serial.print("Hostname\t: ");
  Serial.println(WiFi.getHostname());       //Output hostname.
}

void printHelp(String opts) {
  Serial.println("\nUsage:\n");
  Serial.println("setSSID     \t\"YOUR SSID\"");
  Serial.println("setPassword \t\"YOUR PASSWORD\"");
  Serial.println("connect");
  Serial.println("scan");
  Serial.println("status");
  Serial.println("disconnect");
}

void scanNetworks(String opts) {
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.print("\nscan done: ");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found\n");
    for (int i = 0; i < n; ++i) {
      String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "[O]" : "[*]";
      Serial.printf("%02d %s[%i][%s]\r\n",i+1,enc.c_str(),WiFi.RSSI(i),WiFi.SSID(i).c_str());
      delay(10);
    }
  }
}

void wifiLoop () {
  static uint_least64_t wifiTimeStamp = 0;
  if (millis() - wifiTimeStamp > 1000) {
    // wifiMulti.run(connectTimeoutMs);
    wifiTimeStamp = millis();
    if(WiFi.status() == WL_CONNECTED) M5.dis.fillpix(0x00ff00); // set LED to green
    else M5.dis.fillpix(0xffff00); // set LED to yellow
  }
}

void wifiStatus(String opts) {
  if(WiFi.status() == WL_CONNECTED) {
    printWifiSettings();
  } else {
    Serial.println("\nWiFi is not connected");
  }
}

String getNetKeyName(int net) {
  if (net > 99) return "";
  char key[11];
  sprintf(key, "key_net%02d", net);
  return String(key);
}

void loadSavedNetworks() {
  cfg.begin("wifi_cli_prefs", RO_MODE);
  int net = 1;
  Serial.println("\nLoading networks..");
  while (cfg.isKey(String(getNetKeyName(net)+"_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid = cfg.getString(String(key + "_ssid").c_str(), "");
    String pasw = cfg.getString(String(key + "_pasw").c_str(), "");
    Serial.printf("%d: [%s][%s]\r\n", net, ssid.c_str(), pasw.c_str());
    wifiMulti.addAP(ssid.c_str(), pasw.c_str());
    net++;
  }
  cfg.end();
}

bool isSSIDSaved(String ssid) {
  int net = 1;
  while (cfg.isKey(String(getNetKeyName(net)+"_ssid").c_str())) {
    String key = getNetKeyName(net);
    String ssid_ = cfg.getString(String(key + "_ssid").c_str(), "");
    if (ssid_ == ssid) return true;
    net++;
  }
  return false;
}

void saveNetwork(String ssid, String pasw) {
  cfg.begin("wifi_cli_prefs", RW_MODE);
  int net = cfg.getInt("net_count", 0);
  String key = getNetKeyName(net+1);
  Serial.printf("Saving network: [%s][%s][%s]\r\n",key.c_str(), ssid.c_str(), pasw.c_str());
  cfg.putString(String(key + "_ssid").c_str(), ssid);
  cfg.putString(String(key + "_pasw").c_str(), pasw);
  cfg.putInt("net_count", net+1);
  cfg.end();
}

void setSSID(String opts) {
  temp_ssid = maschinendeck::SerialTerminal::ParseArgument(opts);
  if (temp_ssid.length() == 0) {
    Serial.println("\nSSID is empty, please set a valid SSID into quotes");
    printHelp(opts);
  }
  else {
    Serial.println("\nsaved ssid to   \t: " + temp_ssid);
  }
}

void setPassword(String opts) {
  temp_pasw = maschinendeck::SerialTerminal::ParseArgument(opts);
  Serial.println("\nsaved password to \t: " + temp_pasw);
}

void disconnect(String opts) {
  Serial.println("\nDisconnecting...");
  WiFi.disconnect();
}

void multiWiFiConnect() {
  int retry = 0;
  Serial.print("\nConnecting...");
  while (wifiMulti.run(connectTimeoutMs) != WL_CONNECTED && retry++< 10) {
    delay(500);
    Serial.print(".");
  } 
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("connected!");
    wifiStatus("");
  }
  else {
    Serial.println("failed!");
  }
}

void connect(String opts) {
  if (WiFi.status() == WL_CONNECTED && temp_ssid == WiFi.SSID()) {
    Serial.println("\nWiFi is already connected");
    return;
  }
  else if (WiFi.status() == WL_CONNECTED) {
    disconnect(opts);
    delay(1000);
  }
  if(isSSIDSaved(temp_ssid) || temp_ssid.length() == 0) {
    multiWiFiConnect();
    return;
  }
  else {
    Serial.print("\nConnecting to " + temp_ssid + "...");
    wifiMulti.addAP(temp_ssid.c_str(), temp_pasw.c_str());
    int retry = 0;
    WiFi.begin(temp_ssid.c_str(), temp_pasw.c_str());
    while (WiFi.status() != WL_CONNECTED && retry++<20) { // M5Atom will connect automatically
      delay(1000);
      Serial.print(".");
    }
    delay(100);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("connected!");
      saveNetwork(temp_ssid, temp_pasw);
      wifiStatus(opts);
    }
    else {
      Serial.println("connection failed!");
    }
  }
}

void setup() {
  M5.begin(true,false,true);  //Init Atom(Initialize serial port, LED)
  M5.dis.fillpix(0xffffff);   //Light LED with the specified RGB color. 
  WiFi.mode(WIFI_STA);
  cfg.begin("wifi_cli_prefs", RO_MODE);
  if (cfg.getInt("net_count", 0) >= 1) {
    loadSavedNetworks();
    multiWiFiConnect();
  }
  cfg.end();
  delay(100);
  Serial.flush();
  Serial.println("\n\n");
  term = new maschinendeck::SerialTerminal(115200);
  term->add("help", &printHelp, "\tshow help and usage information");
  term->add("setSSID", &setSSID, "\tset the Wifi SSID");
  term->add("setPassword", &setPassword, "set the WiFi password"); 
  term->add("connect", &connect, "\tconnect or add WiFi network");
  term->add("scan", &scanNetworks, "\tscan WiFi networks");
  term->add("status", &wifiStatus, "\tWiFi status information");
  term->add("disconnect", &disconnect, "WiFi disconnect");
 
}

void loop() {
  term->loop();
  wifiLoop();
}
