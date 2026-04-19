#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);  // Give Serial Monitor time to connect

  Serial.println("\n\n===== ESP32 Family MAC Address Reader =====");

  // Initialize WiFi (required for MAC address access)
  WiFi.mode(WIFI_STA);
  delay(100);

  // Method 1: Using WiFi.macAddress() (works on all ESP32 variants)
  Serial.print("WiFi MAC: ");
  Serial.println(WiFi.macAddress());

  // Method 2: Get MAC as byte array for custom formatting
  uint8_t mac[6];
  WiFi.macAddress(mac);

  Serial.print("MAC Bytes: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.println("Done!");
}

void loop() {
  delay(10000);
}