#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give Serial Monitor time to connect
  
  Serial.println("\n\n===== ESP32-S3 MAC Address Reader =====");
  
  // Method 1: Get MAC as formatted string (simplest)
  Serial.print("MAC Address (String): ");
  Serial.println(WiFi.macAddress());
  
  // Method 2: Get MAC as byte array
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  Serial.print("MAC Address (Hex): ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

void loop() {
  delay(10000);
}