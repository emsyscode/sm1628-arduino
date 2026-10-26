/*
  Joystick voltage test for Arduino Uno: signal inputs A0 and A1.
  Open Serial Monitor at 115200 baud.

  This reads voltage, not resistance. The external resistor network is part
  of the hardware; this sketch does not calculate or validate its values.
*/

constexpr uint8_t kInputA0 = A0;
constexpr uint8_t kInputA1 = A1;
constexpr unsigned long kSerialBaud = 115200;
constexpr unsigned long kReportIntervalMs = 500;

// DEFAULT uses the Uno's supply as its ADC reference. 5 V is nominal;
// replace this number with the measured reference for a better estimate.
constexpr float kReferenceVolts = 5.0f;
constexpr uint16_t kAdcMaxReading = 1023;

void printReading(const __FlashStringHelper *label, int raw) {
  // Preserve the original 0..1023 -> 0..reference endpoint conversion.
  const float volts = raw * (kReferenceVolts / kAdcMaxReading);

  Serial.print(label);
  Serial.print(raw);
  Serial.print(F(" | Voltage: "));
  Serial.print(volts, 3);  // Three displayed decimals do not imply 1 mV accuracy.
  Serial.print(F(" V"));
}

void setup() {
  analogReference(DEFAULT);
  Serial.begin(kSerialBaud);
}

void loop() {
  const int rawA0 = analogRead(kInputA0);
  const int rawA1 = analogRead(kInputA1);

  printReading(F("A0 -> Raw: "), rawA0);
  Serial.print(F("   |   "));
  printReading(F("A1 -> Raw: "), rawA1);
  Serial.println();

  // Pace the Serial output to roughly two reports per second.
  // This pause is for bench readability, not a Snake input polling interval.
  delay(kReportIntervalMs);
}

