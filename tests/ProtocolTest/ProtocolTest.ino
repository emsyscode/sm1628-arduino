#include <Sm1628B.h>

// Simulation fixture: D7 DATA, D8 CLK, D9 STB, D6 completion, D5 failure.
// Not a panel demonstration or a verified electrical model.
sm1628::Sm1628B display(7, 8, 9);
void require(bool value) {
  if (!value) {
    pinMode(5, OUTPUT);
    digitalWrite(5, HIGH);
    while (true) {}
  }
}
void setup() {
  sm1628::Frame frame;
  require(!display.writeFrame(frame));
  require(!display.setBrightness(1));
  require(!display.setEnabled(true));
  require(!display.begin(8));
  sm1628::Sm1628B invalid(7, 7, 9);
  require(!invalid.begin());
  sm1628::Sm1628B invalidPin(255, 8, 9);
  require(!invalidPin.begin());
  require(display.begin(0));
  for (uint8_t i = 0; i < sm1628::Frame::kSize; ++i)
    frame.bytes[i] = uint8_t(0x81u ^ (i * 13u));
  require(!frame.set({14, 0}, true));
  require(!frame.set({0, 8}, true));
  require(display.writeFrame(frame));
  require(display.setBrightness(7));
  require(!display.setBrightness(8));
  require(display.setEnabled(false));
  require(display.setBrightness(2));  // Must stay disabled.
  require(display.setEnabled(true));
  require(display.clear());

  // Synthetic, deliberately non-contiguous mapping; no hardware profile.
  sm1628::Address map[7] = {{13,7},{0,2},{5,6},{4,1},{10,4},{9,0},{2,3}};
  frame.clear();
  frame.bytes[6] = 0xA5;  // Unrelated outputs must survive glyph changes.
  require(sm1628::setGlyph(frame, map, 0x06)); // b,c only.
  require(display.writeFrame(frame));
  map[6] = {14,0};
  require(!sm1628::setGlyph(frame, map, 0x7F));
  require(display.writeFrame(frame)); // Failed operation must be atomic.
  map[6] = map[0];
  require(!sm1628::setGlyph(frame, map, 0x7F));
  require(!sm1628::setGlyph(frame, map, 0x80));
  require(display.writeFrame(frame));
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
}
void loop() {}
