#include <WiFi.h>
#include <esp_system.h>

void printMacAddresses() {
  uint64_t chipId = ESP.getEfuseMac();
  uint8_t baseMac[6];

  baseMac[0] = (chipId >> 40) & 0xFF;
  baseMac[1] = (chipId >> 32) & 0xFF;
  baseMac[2] = (chipId >> 24) & 0xFF;
  baseMac[3] = (chipId >> 16) & 0xFF;
  baseMac[4] = (chipId >> 8) & 0xFF;
  baseMac[5] = chipId & 0xFF;

  WiFi.mode(WIFI_STA);
  delay(100);

  Serial.println();
  Serial.println("ESP32-D MAC address reader");
  Serial.println("--------------------------");

  Serial.print("WiFi STA MAC: ");
  Serial.println(WiFi.macAddress());

  Serial.print("Base MAC:     ");
  for (int i = 0; i < 6; i++) {
    if (baseMac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(baseMac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();
  Serial.println();
  Serial.println("Type mac and press Enter to print it again.");
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  printMacAddresses();
}

void loop() {
  static unsigned long lastReminder = 0;

  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();

    if (command == "mac") {
      printMacAddresses();
    } else if (command.length() > 0) {
      Serial.println("Type mac and press Enter.");
    }
  }

  if (millis() - lastReminder >= 5000) {
    lastReminder = millis();
    Serial.println("Waiting... type mac and press Enter.");
  }
}
