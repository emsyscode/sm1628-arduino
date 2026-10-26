// One-pass SLOT MAPPING diagnostic derived from the working PanelDemo.
// Same bus routines and setup; only test selection, holds and serial labels differ.
// Board: Arduino Uno / ATmega328P, 16 MHz. No external display library required.
#include <Arduino.h>
#include <avr/interrupt.h>

constexpr uint8_t kDataPin = 7;
constexpr uint8_t kClockPin = 8;
constexpr uint8_t kStrobePin = 9;
constexpr uint8_t kDisplayMode = 0x03;
constexpr uint8_t kFullBrightness = 0x8F;
constexpr uint8_t kFrameBytes = 14;

// Literal patterns from the working sketch, including its second-byte bits.
// Each pair corresponds to a,b,c,d,e,f,g in source order.
// Second bytes contain legacy icon masks and are NOT sent by this diagnostic.
const uint8_t kPatterns[10][kFrameBytes] = {
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010,
     0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000010},  // 0
    {0b00000000, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010,
     0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010},  // 1
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000000, 0b00000010, 0b00000001, 0b00000010,
     0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010},  // 2
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010,
     0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010},  // 3
    {0b00000000, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010,
     0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010},  // 4
    {0b00000001, 0b00000010, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010,
     0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010},  // 5
    {0b00000001, 0b00000010, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010,
     0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010},  // 6
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010,
     0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010},  // 7
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010,
     0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010},  // 8
    {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010,
     0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010},  // 9
};

// The original enabled Timer1 even though its active display test does not
// use the clock values. Keep that setup for this first behavior comparison.
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

void showTestNumeral(uint8_t numeral, uint8_t slot) {
  // A slot is a bit shift used by the source test, not a physical digit index.
  // The working sketch tests slots 0..7; their visible order needs recording.
  if (numeral > 9 || slot > 7)
    return;
  delayMicroseconds(1);
  sendCommand(kDisplayMode);
  sendCommand(0x80);  // Preserve the source's blanking before every frame.
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  sendBurstByte(0xC0);
  for (uint8_t i = 0; i < kFrameBytes; ++i) {
    // Only the numeral stroke bit is shifted. Clear all second-byte icon bits.
    sendBurstByte((i % 2 == 0) ? uint8_t((kPatterns[numeral][i] & 1u) << slot) : 0);
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
  // Single pass only. Numeric position and extra symbols must be observed.
  static bool completed = false;
  if (completed)
    return;
  completed = true;
  delay(3000);  // Allows time to open Serial Monitor; opening it may reset Uno.
  Serial.println(F("START: one pass, slots 0 to 7. Positions counted from LEFT."));
  for (uint8_t slot = 0; slot < 8; ++slot) {
    clearPanel();
    delay(500);  // A visible blank separates every labelled pass.
    Serial.print(F("SLOT "));
    Serial.print(slot);
    Serial.println(F(": showing 8, clean raw slot; slot6 intentionally selects symbols."));
    showTestNumeral(8, slot);  // Original 100 ms hold is inside this function.
    delay(2900);               // About 3 seconds total per slot.
  }
  clearPanel();
  Serial.println(F("DONE. No repeat. Reset the Uno to run again."));
}

ISR(TIMER1_COMPA_vect) {
  ++legacySeconds;
  legacyFlag = !legacyFlag;
}

