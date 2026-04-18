#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("");
  Serial.println("ESP32 MAC Address");
  Serial.println("=================");
  
  // Get MAC address using WiFi library (works on all ESP32 variants)
  WiFi.mode(WIFI_STA);
  delay(100);
  
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  Serial.println("Done");
}

void loop() {
  delay(10000);
}