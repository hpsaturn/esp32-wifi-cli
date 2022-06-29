  
#include <M5Atom.h>
#include "WiFi.h"

void printWifiSettings() {
  M5.dis.fillpix(0x00ff00);
  Serial.print("WiFi Connect To: ");
  Serial.println(WiFi.SSID());  //Output Network name.
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //Output IP Address.
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());  //Output signal strength.
}

void initSmartConfig(){
  WiFi.beginSmartConfig();
  M5.dis.fillpix(0xfff000); //Light LED with the specified RGB color F00000(Atom-Matrix has only one light).
  //Wait for the M5Atom to receive network information from the phone
  Serial.print("\nWaiting for Phone SmartConfig."); //Serial port format output string.
  while (!WiFi.smartConfigDone()) { //If the smart network is not completed.
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nSmartConfig received.");
  Serial.println("Waiting for WiFi\n");
  while (WiFi.status() != WL_CONNECTED) { //M5Atom will connect automatically upon receipt of the configuration information.
    delay(500);
    Serial.print(".");
  }
  printWifiSettings();
}

void setup() {
  M5.begin(true,false,true);  //Init Atom(Initialize serial port, LED)
  WiFi.mode(WIFI_AP_STA); // Set the wifi mode to the mode compatible with the AP and Station
  long seed=analogRead(32);
  randomSeed(seed);
  delay(10);
  Serial.println("\n\n                                          ");
}

void printString(String str) {
  Serial.printf("                                                                                \r");
  Serial.printf("%s\r", str);
}

void addCharacter(char c) {
}

void loop() {
  long randNumber = random(101);
  printString(""+String(randNumber));
  delay(100);
}