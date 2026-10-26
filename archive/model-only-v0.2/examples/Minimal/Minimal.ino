// Small standalone version, based on Jose Luis Monteiro's SM1628B examples.
// No library installation needed. See docs/minimal.md for the original sources.
#include <Arduino.h>

#ifndef SM1628_BENCH_READY
#define SM1628_BENCH_READY 0  // Set to 1 after confirming your IC and wiring.
#endif

constexpr uint8_t kData = 7;    // Arduino GPIOs, not display connector positions.
constexpr uint8_t kClock = 8;
constexpr uint8_t kStrobe = 9;
constexpr uint8_t kFrameBytes = 14;  // Seven RAM words, low byte then high byte.
constexpr uint8_t kBrightness = 0;   // 0..7; zero is dim, not off.
uint8_t frame[kFrameBytes] = {0};
bool segmentOn = false;
unsigned long lastChange = 0;

void sendByte(uint8_t value) {
  // First transmitted bit is bit 0; data is stable before the rising CLK edge.
  for (uint8_t bit = 0; bit < 8; ++bit) {
    digitalWrite(kClock, LOW);
    digitalWrite(kData, (value & 1) ? HIGH : LOW);
    delayMicroseconds(5);
    digitalWrite(kClock, HIGH);
    delayMicroseconds(5);
    value >>= 1;
  }
}

void sendCommand(uint8_t command) {
  digitalWrite(kStrobe, LOW);
  delayMicroseconds(1);
  sendByte(command);
  digitalWrite(kStrobe, HIGH);
  delayMicroseconds(1);
}

void writeFrame() {
  sendCommand(0x40);  // Write display RAM with automatic address increment.
  digitalWrite(kStrobe, LOW);
  delayMicroseconds(1);
  sendByte(0xC0);     // Start at RAM address zero; keep STB low for the data.
  for (uint8_t i = 0; i < kFrameBytes; ++i) sendByte(frame[i]);
  digitalWrite(kStrobe, HIGH);
  delayMicroseconds(1);
}

void setup() {
  if (!SM1628_BENCH_READY) return;
  digitalWrite(kStrobe, HIGH);
  digitalWrite(kClock, HIGH);
  digitalWrite(kData, LOW);
  pinMode(kStrobe, OUTPUT);
  pinMode(kClock, OUTPUT);
  pinMode(kData, OUTPUT);
  delayMicroseconds(200);

  sendCommand(0x80); // Display off while its initial contents are established.
  sendCommand(0x03); // Mode inherited from the original SM1628B examples.
  writeFrame();     // The zero-filled buffer clears the fourteen RAM bytes.
  sendCommand(0x88 | kBrightness);
}

void loop() {
  if (!SM1628_BENCH_READY || millis() - lastChange < 500) return;
  lastChange = millis();
  segmentOn = !segmentOn;
  // Illustrative RAM bit, NOT a known digit/segment on an unidentified board.
  // Replace this address/mask with one established using SegmentWalk.
  frame[0] = segmentOn ? 0x01 : 0x00;
  writeFrame();
}
