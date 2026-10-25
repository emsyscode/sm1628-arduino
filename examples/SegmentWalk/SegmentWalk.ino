#include <Sm1628B.h>

sm1628::Sm1628B display(7, 8, 9);  // Arduino pins: DATA, CLK, STB.

// Change only after checking the exact IC, supply and board connector.
// These legacy Arduino pin assignments are not a board connector pinout.
constexpr bool kBenchReady = false;
uint8_t output = 0;
unsigned long lastStep = 0;

void setup() {
  Serial.begin(115200);
  if (!kBenchReady) {
    Serial.println(F("Bench setup pending: read docs/01-bring-up.md first."));
    return;
  }
  display.begin(0);  // Lowest enabled brightness.
}

void loop() {
  if (!kBenchReady || millis() - lastStep < 500) return;
  lastStep = millis();
  sm1628::Frame frame;
  const sm1628::Address address = {uint8_t(output / 8), uint8_t(output % 8)};
  frame.set(address, true);
  display.writeFrame(frame);
  Serial.print(F("RAM byte=")); Serial.print(address.byte);
  Serial.print(F(" bit=")); Serial.println(address.bit);
  output = (output + 1) % (sm1628::Frame::kSize * 8);
}
