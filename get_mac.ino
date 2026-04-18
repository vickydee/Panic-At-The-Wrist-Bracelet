#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Get MAC address in different formats
  
  // Format 1: As a byte array
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  Serial.print("MAC Address (hex): ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
  
  // Format 2: Using WiFi.macAddress() - easier method
  Serial.print("MAC Address (WiFi): ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  delay(10000);
}