#include <WiFi.h>
#include <esp_wifi.h>

void printMacAddress() {
  uint8_t baseMac[6];
  esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  
  Serial.print("MAC Address: ");
  for (int i = 0; i < 6; ++i) {
    Serial.printf("%02x", baseMac[i]);
    if (i < 5)
      Serial.print(":");
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n===== ESP32 MAC Address Reader =====");
  printMacAddress();
}

void loop() {
  delay(10000);
}