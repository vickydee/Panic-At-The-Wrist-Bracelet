#include <Wire.h>
#include <MAX30105.h>
#include <TinyGPSPlus.h>

// ---------------------------------------------------------------------------
// Hardware validation sketch for ESP32-S3-DevKitC-1
//
// This sketch is intentionally focused on proving basic hardware connectivity
// before any full bracelet application logic is added.
//
// What this sketch validates:
// 1. MAX30102 is present on I2C and raw IR/Red values can be read.
// 2. The TTP223 output can be read as a digital signal and changes state.
// 3. The passive piezo buzzer can be turned on for a simple audible test.
// 4. The GPS UART (operates at a default baud rate of 9600, requires 3-5V) is 
//    receiving data, and latitude/longitude are printed when the module has a 
//    valid fix.
//
// Update the pin assignments below if your wiring is different.
// ---------------------------------------------------------------------------

// -----------------------------
// Pin assignments (edit as needed)
// -----------------------------
// MAX30102
const int I2C_SDA_PIN = 6;
const int I2C_SCL_PIN = 5;
const uint8_t MAX30102_I2C_ADDRESS = 0x57;

// TTP223
const int TOUCH_PIN = 11;

// Piezo Buzzer
const int BUZZER_PIN = 45;
const int BUZZER_LEDC_RESOLUTION = 10;
const int BUZZER_TEST_TONE_HZ = 2000;
const unsigned long BUZZER_TEST_DURATION_MS = 500;

// GPS
const int GPS_RX_PIN = 26;  // ESP32 receives GPS TX on this pin
const int GPS_TX_PIN = 27;  // ESP32 transmits to GPS RX on this pin
const uint32_t GPS_BAUD = 9600;

// -----------------------------
// Timing constants
// -----------------------------
const unsigned long MAX30102_PRINT_INTERVAL_MS = 1000;
const unsigned long TOUCH_REMINDER_INTERVAL_MS = 5000;
const unsigned long GPS_STATUS_INTERVAL_MS = 2000;

// Library objects
MAX30105 particleSensor;   // SparkFun library uses MAX30105 class for MAX30102 too
TinyGPSPlus gps;
HardwareSerial GPSSerial(1);

// State tracking
bool max30102Ready = false;
bool touchChanged = false;
bool gpsFixPrinted = false;

int lastTouchState = LOW;

unsigned long lastMAX30102PrintMs = 0;
unsigned long lastTouchReminderMs = 0;
unsigned long lastGPSStatusMs = 0;
unsigned long totalGPSBytes = 0;
unsigned long lastGPSByteSnapshot = 0;

void printStartupSummary();
void setupMAX30102Validation();
void setupTouchValidation();
void setupBuzzerValidation();
void setupGPSValidation();
void runBuzzerValidation();
void validateMAX30102Readings();
void validateTouchInputs();
void validateGPSData();
bool i2cAddressResponds(uint8_t address);

void setup() {
  // Force the buzzer pin low as early as possible.
  // This helps if the buzzer is sounding because the pin is floating after reset.
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(115200);
  delay(1500);

  printStartupSummary();

  setupMAX30102Validation();
  setupTouchValidation();
  setupBuzzerValidation();
  setupGPSValidation();

  // This proves the ESP32 can actively drive the buzzer output pin and make
  // one clear audible beep. Failure here usually means buzzer wiring, wrong
  // pin assignment, or using a passive buzzer without a driven signal.
  runBuzzerValidation();

  Serial.println();
  Serial.println("===== Entering continuous validation loop =====");
  Serial.println("Touch the TTP223 sensor, place a finger on the MAX30102,");
  Serial.println("and move the GPS antenna where it can see the sky if you want a fix.");
  Serial.println();
}

void loop() {
  validateMAX30102Readings();
  validateTouchInputs();
  validateGPSData();
  delay(20);
}

void printStartupSummary() {
  Serial.println();
  Serial.println("==================================================");
  Serial.println("ESP32-S3 Bracelet Hardware Validation Sketch");
  Serial.println("==================================================");
  Serial.println("Serial Monitor baud: 115200");
  Serial.println();
  Serial.println("Pin assignment summary:");
  Serial.printf("  MAX30102 I2C SDA: GPIO %d\n", I2C_SDA_PIN);
  Serial.printf("  MAX30102 I2C SCL: GPIO %d\n", I2C_SCL_PIN);
  Serial.printf("  TTP223 input    : GPIO %d\n", TOUCH_PIN);
  Serial.printf("  Piezo buzzer    : GPIO %d\n", BUZZER_PIN);
  Serial.printf("  GPS RX (ESP32)  : GPIO %d\n", GPS_RX_PIN);
  Serial.printf("  GPS TX (ESP32)  : GPIO %d\n", GPS_TX_PIN);
  Serial.printf("  GPS baud rate   : %lu\n", GPS_BAUD);
  Serial.println();
  Serial.println("This is a validation sketch only.");
  Serial.println("It checks basic wiring, power, and communication for each module.");
  Serial.println("It is not full bracelet application logic.");
  Serial.println();
}

bool i2cAddressResponds(uint8_t address) {
  Wire.beginTransmission(address);
  return (Wire.endTransmission() == 0);
}

void setupMAX30102Validation() {
  Serial.println("[MAX30102] Starting I2C validation...");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  delay(50);

  // This proves that something answers at the expected MAX30102 I2C address.
  // Failure here usually means incorrect SDA/SCL pins, wiring faults, or no power.
  if (!i2cAddressResponds(MAX30102_I2C_ADDRESS)) {
    Serial.println("[MAX30102] FAIL: No device responded at I2C address 0x57.");
    Serial.println("[MAX30102] Meaning: check power, ground, SDA/SCL wiring, and pullups.");
    Serial.println();
    return;
  }

  // This proves the SparkFun driver can initialize the sensor over I2C.
  // Failure here usually means the wrong device is on the bus or the bus is unstable.
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("[MAX30102] FAIL: Device responded on I2C, but library init failed.");
    Serial.println("[MAX30102] Meaning: bus timing, bad wiring, or wrong hardware on 0x57.");
    Serial.println();
    return;
  }

  // Minimal configuration for raw sample testing.
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);
  particleSensor.setPulseAmplitudeGreen(0);

  max30102Ready = true;
  Serial.println("[MAX30102] PASS: Sensor detected and initialized.");
  Serial.println("[MAX30102] Raw IR/Red values will be printed once per second.");
  Serial.println("[MAX30102] Put a finger over the sensor to see values change.");
  Serial.println();
}

void setupTouchValidation() {
  Serial.println("[TTP223] Starting digital input validation...");

  // This proves the ESP32 can sample the touch module as a digital input.
  // Failure to ever change state usually means wiring, power, or wrong GPIO pin.
  pinMode(TOUCH_PIN, INPUT);

  lastTouchState = digitalRead(TOUCH_PIN);

  Serial.printf("[TTP223] Initial state: %d\n", lastTouchState);
  Serial.println("[TTP223] Touch the pad and watch for state-change messages.");
  Serial.println();
}

void setupBuzzerValidation() {
  Serial.println("[BUZZER] Preparing output for a simple on/off beep test...");

  // A passive buzzer still needs a square wave to make sound, so this uses one
  // fixed-frequency PWM tone for "on" and silence for "off".
  // If you hear nothing during the beep test, check buzzer polarity, wiring,
  // shared ground, and whether the module is truly a passive buzzer.
  if (!ledcAttach(BUZZER_PIN, BUZZER_TEST_TONE_HZ, BUZZER_LEDC_RESOLUTION)) {
    Serial.println("[BUZZER] FAIL: Could not attach buzzer pin to LEDC.");
    Serial.println("[BUZZER] Meaning: this GPIO may not support LEDC output or the pin is already in use.");
    Serial.println();
    return;
  }

  ledcWriteTone(BUZZER_PIN, 0);

  Serial.println("[BUZZER] PWM output ready.");
  Serial.println();
}

void setupGPSValidation() {
  Serial.println("[GPS] Starting UART validation...");

  // This proves the ESP32 UART is configured and ready to receive NMEA data.
  // Failure to receive any bytes usually means TX/RX wiring is wrong, GPS power
  // is missing, or the GPS module uses a different baud rate.
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.printf("[GPS] Listening for NMEA data on UART1 at %lu baud.\n", GPS_BAUD);
  Serial.println("[GPS] Receiving bytes proves the UART link works.");
  Serial.println("[GPS] A valid location fix may take time and usually needs outdoor sky view.");
  Serial.println();
}

void runBuzzerValidation() {
  Serial.println("[BUZZER] Running beep validation...");
  Serial.printf("[BUZZER] Turning buzzer ON at %d Hz for %lu ms\n",
                BUZZER_TEST_TONE_HZ, BUZZER_TEST_DURATION_MS);

  ledcWriteTone(BUZZER_PIN, BUZZER_TEST_TONE_HZ);
  delay(BUZZER_TEST_DURATION_MS);
  ledcWriteTone(BUZZER_PIN, 0);

  Serial.println("[BUZZER] Buzzer OFF.");
  Serial.println("[BUZZER] PASS if you heard one clear beep.");
  Serial.println("[BUZZER] FAIL if silent: verify passive buzzer wiring and pin.");
  Serial.println();
}

void validateMAX30102Readings() {
  if (!max30102Ready) {
    return;
  }

  if (millis() - lastMAX30102PrintMs < MAX30102_PRINT_INTERVAL_MS) {
    return;
  }

  lastMAX30102PrintMs = millis();

  // This proves the sensor is returning live ADC sample values over I2C.
  // If values stay stuck at zero or never change with a finger present, check
  // power, sensor orientation, finger placement, and LED/sample configuration.
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  Serial.printf("[MAX30102] IR=%ld  RED=%ld", irValue, redValue);

  if (irValue == 0 && redValue == 0) {
    Serial.print("  -> WARNING: both channels are zero; sensor may not be reading properly.");
  } else {
    Serial.print("  -> PASS: raw values are being read.");
  }

  Serial.println();
}

void validateTouchInputs() {
  int currentTouchState = digitalRead(TOUCH_PIN);

  // This proves the touch output line can toggle and be observed by the ESP32.
  // If the sensor never changes state, the pad may not be wired correctly or may
  // not be powered.
  if (currentTouchState != lastTouchState) {
    touchChanged = true;
    Serial.printf("[TTP223] State changed: %d -> %d  -> PASS\n", lastTouchState, currentTouchState);
    lastTouchState = currentTouchState;
  }

  if (millis() - lastTouchReminderMs >= TOUCH_REMINDER_INTERVAL_MS) {
    lastTouchReminderMs = millis();

    if (!touchChanged) {
      Serial.println("[TTP223] State has not changed yet. Touch the pad now to validate GPIO input.");
    }
  }
}

void validateGPSData() {
  while (GPSSerial.available() > 0) {
    char c = static_cast<char>(GPSSerial.read());
    gps.encode(c);
    totalGPSBytes++;
  }

  // This proves the UART path is active even before a satellite fix exists.
  // If byte count never increases, likely causes are swapped TX/RX, wrong baud,
  // no GPS power, or wrong UART pins.
  if (millis() - lastGPSStatusMs >= GPS_STATUS_INTERVAL_MS) {
    lastGPSStatusMs = millis();

    unsigned long bytesSinceLastStatus = totalGPSBytes - lastGPSByteSnapshot;
    lastGPSByteSnapshot = totalGPSBytes;

    if (bytesSinceLastStatus > 0) {
      Serial.printf("[GPS] PASS: received %lu UART bytes in the last %lu ms.\n",
                    bytesSinceLastStatus, GPS_STATUS_INTERVAL_MS);
    } else {
      Serial.println("[GPS] WARNING: no UART bytes received yet. Check TX/RX wiring, baud, and GPS power.");
    }

    if (!gps.location.isValid()) {
      Serial.println("[GPS] No valid fix yet. This does not always mean failure; many modules need outdoor sky view.");
    }
  }

  // This proves that valid NMEA data is being decoded into a real position.
  // If UART bytes are present but no fix ever appears, the module may simply
  // need more time or better antenna visibility.
  if (gps.location.isUpdated()) {
    gpsFixPrinted = true;
    Serial.printf("[GPS] FIX: lat=%.6f  lon=%.6f", gps.location.lat(), gps.location.lng());

    if (gps.satellites.isValid()) {
      Serial.printf("  sats=%lu", gps.satellites.value());
    }

    if (gps.hdop.isValid()) {
      Serial.printf("  hdop=%.2f", gps.hdop.hdop());
    }

    Serial.println();
  } else if (!gpsFixPrinted && totalGPSBytes > 0 && gps.location.isValid()) {
    gpsFixPrinted = true;
    Serial.printf("[GPS] FIX: lat=%.6f  lon=%.6f\n", gps.location.lat(), gps.location.lng());
  }
}
