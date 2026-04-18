#include <WiFi.h>

void printMacAddress() {
  uint8_t baseMac[6];
  // Get the base MAC address
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  
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