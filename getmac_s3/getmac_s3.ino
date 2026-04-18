#include <WiFi.h>
#include <esp_mac.h>  // ESP32-S3 specific MAC functions

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give Serial Monitor time to connect

  Serial.println("\n\n===== ESP32-S3 MAC Address Reader =====");

  // Method 1: Using WiFi.macAddress() (simplest)
  Serial.print("WiFi MAC: ");
  Serial.println(WiFi.macAddress());

  // Method 2: Using ESP-IDF functions (more reliable on S3)
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  Serial.print("ESP-IDF MAC: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  // Method 3: Get base MAC address (factory burned)
  uint8_t baseMac[6];
  esp_efuse_mac_get_default(baseMac);

  Serial.print("Base MAC: ");
  for (int i = 0; i < 6; i++) {
    if (baseMac[i] < 16) Serial.print("0");
    Serial.print(baseMac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.println("Done!");
}

void loop() {
  delay(10000);
}