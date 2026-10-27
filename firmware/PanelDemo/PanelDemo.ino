// PanelDemo - numeric display example for the SM1628 / SM1628B tutorial.
// Counts 0-9 on each numeric position, left to right, without symbol columns.
// Board: Arduino Uno / ATmega328P, 16 MHz. No external display library required.
#include <Arduino.h>
#include <avr/interrupt.h>

constexpr uint8_t kDataPin = 7;
constexpr uint8_t kClockPin = 8;
constexpr uint8_t kStrobePin = 9;
constexpr uint8_t kDisplayMode = 0x03;
constexpr uint8_t kFullBrightness = 0x8F;
constexpr uint8_t kFrameBytes = 14;

// a..g masks derived ONLY from the first bit in each original byte pair.
// Preserve inherited numeral shapes (including the original9 bottom-bar omission).
// Symbol columns are left clear; see docs/04-memory.md for the RAM layout.
const uint8_t kNumeralMasks[10] = {0x3F, 0x6, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7, 0x7F, 0x67};
constexpr uint8_t kDigitColumns[7] = {0, 1, 7, 2, 3, 4, 5};

// The original enabled Timer1 even though its active display test does not
// use the clock values. Retain its register setup for transport compatibility.
volatile uint8_t legacySeconds = 0;
volatile bool legacyFlag = true;

void sendCommand(uint8_t value) {
  // STB frames a single command. Keep the original command-specific timing.
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    digitalWrite(kClockPin, LOW);
    delayMicroseconds(1);
    digitalWrite(kDataPin, (value & mask) ? HIGH : LOW);
    digitalWrite(kClockPin, HIGH);
    delayMicroseconds(1);
  }
  digitalWrite(kStrobePin, HIGH);
  delayMicroseconds(1);
}

void sendBurstByte(uint8_t value) {
  // STB is controlled by the caller, allowing address + data in one packet.
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    digitalWrite(kClockPin, LOW);
    digitalWrite(kDataPin, (value & mask) ? HIGH : LOW);
    delayMicroseconds(5);
    digitalWrite(kClockPin, HIGH);
    delayMicroseconds(5);
  }
}

void initializePanel() {
  delayMicroseconds(200);
  sendCommand(kDisplayMode);
  delayMicroseconds(1);
  sendCommand(0x40);
  delayMicroseconds(1);
  sendCommand(0xC0);
  delayMicroseconds(1);
  sendCommand(kFullBrightness);
  delayMicroseconds(1);
}

void clearPanel() {
  // Preserve the reference's fourteen separate addressed writes.
  for (uint8_t address = 0; address < kFrameBytes; ++address) {
    sendCommand(0x40);
    digitalWrite(kStrobePin, LOW);
    delayMicroseconds(1);
    sendBurstByte(0xC0 | address);
    sendBurstByte(0);
    digitalWrite(kStrobePin, HIGH);
    delayMicroseconds(1);
  }
}

void showTestNumeral(uint8_t numeral, uint8_t digit) {
  if (numeral > 9 || digit > 6)
    return;
  delayMicroseconds(1);
  sendCommand(kDisplayMode);
  sendCommand(0x80);
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  sendBurstByte(0xC0);
  const uint8_t columnMask = uint8_t(1u << kDigitColumns[digit]);
  for (uint8_t row = 0; row < 7; ++row) {
    sendBurstByte((kNumeralMasks[numeral] & (1u << row)) ? columnMask : 0);
    sendBurstByte(0);  // No symbol bits in the second byte.
  }
  digitalWrite(kStrobePin, HIGH);
  sendCommand(kFullBrightness);
  delay(100);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);  // Preserved from the working setup; no new serial output.
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 62498;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);

  // Compatibility setup retained exactly from the user-tested sketch.
  // It takes ownership of ALL Uno D0..D13 pins; disconnect other peripherals.
  CLKPR = 0x80;
  DDRD = 0xFF;
  PORTD = 0;
  DDRB = 0xFF;
  PORTB = 0;
  initializePanel();
  clearPanel();
  sei();
}

void loop() {
  // Physical numeric positions, not raw source slots; symbol groups skipped.
  for (uint8_t digit = 0; digit < 7; ++digit)
    for (uint8_t numeral = 0; numeral < 10; ++numeral)
      showTestNumeral(numeral, digit);
  clearPanel();
}

ISR(TIMER1_COMPA_vect) {
  ++legacySeconds;
  legacyFlag = !legacyFlag;
}

