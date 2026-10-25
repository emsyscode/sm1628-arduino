#include <MappedPanel.h>
#include <profiles/LegacySixDigit.h>
#include <string.h>

sm1628::Sm1628B driver(7, 8, 9);
sm1628::MappedPanel panel(driver);
void require(bool ok) {
  if (!ok) { pinMode(5, OUTPUT); digitalWrite(5, HIGH); while (true) {} }
}

void setup() {
  require(!panel.flush());
  require(!panel.setNumber(0));
  require(!panel.setDigit(0, 0));
  require(panel.configure(sm1628::profiles::kLegacySixDigit, 6));
  require(!panel.flush());  // A failed send must not populate the cache.
  require(driver.begin(0));

  sm1628::DigitMap bad[2] = {
    sm1628::profiles::kLegacySixDigit[0], sm1628::profiles::kLegacySixDigit[1]
  };
  bad[1].segments[0] = bad[0].segments[0];  // Cross-digit alias.
  require(!panel.configure(bad, 2));
  bad[1].segments[0] = {14,0};
  require(!panel.configure(bad, 2));
  bad[1].segments[0] = {0,8};
  require(!panel.configure(bad, 2));
  require(!panel.configure(nullptr, 6));
  require(!panel.configure(bad, 0));
  require(!panel.configure(bad, 8));
  require(panel.digitCount() == 6);

  // Exercise all ten source glyphs at every mapped digit.
  for (uint8_t d = 0; d < 6; ++d) {
    for (uint8_t n = 0; n < 10; ++n) {
      panel.clear();
      require(panel.setDigit(d, n));
      require(panel.flush());
      require(panel.flush());  // Must not send the same frame again.
    }
  }
  const sm1628::Frame before = panel.frame();
  require(!panel.setDigit(6, 1));
  require(!panel.setDigit(0, 10));
  require(!panel.setSegments(0, 0x80));
  require(!panel.setSegment(0, 7, true));
  require(!panel.setOutput({14,0}, true));
  require(memcmp(before.bytes, panel.frame().bytes, 14) == 0);
  require(panel.flush());

  require(panel.setNumber(0)); require(panel.flush());
  require(panel.setNumber(42)); require(panel.flush());
  require(panel.setNumber(42, true)); require(panel.flush());
  const sm1628::Frame padded = panel.frame();
  require(!panel.configure(bad, 2)); // Invalid remap preserves an existing picture.
  require(panel.digitCount() == 6);
  require(!panel.setNumber(1000000UL));
  require(memcmp(padded.bytes, panel.frame().bytes, 14) == 0);
  require(panel.flush());

  require(panel.setOutput({13,7}, true)); // Synthetic, non-digit icon.
  require(panel.setNumber(123456)); require(panel.flush());
  panel.clearDigits(); require(panel.flush()); // Icon stays lit.
  panel.clear(); require(panel.flush());
  require(panel.flush(true));  // Force a complete resend.
  panel.invalidate(); require(panel.flush());

  const uint8_t raw = 0xAA;
  require(driver.writeRange(13, &raw, 1));
  require(!driver.writeRange(14, &raw, 1));
  require(!driver.writeRange(13, &raw, 2));
  require(!driver.writeRange(0, nullptr, 1));
  require(!driver.writeRange(0, &raw, 0));
  panel.invalidate(); require(panel.flush()); // Restore after external write.

  // Seven digits: deliberately synthetic sequential RAM bits, not a board map.
  sm1628::DigitMap seven[7];
  for (uint8_t d = 0; d < 7; ++d)
    for (uint8_t s = 0; s < 7; ++s) {
      const uint8_t bit = d * 7 + s;
      seven[d].segments[s] = {uint8_t(bit / 8), uint8_t(bit % 8)};
    }
  require(panel.configure(seven, 7)); // configure copies; input need not persist.
  seven[0].segments[0] = {14,0};
  require(panel.setNumber(9999999UL)); require(panel.flush());
  require(!panel.setNumber(10000000UL));
  require(!panel.setNumber(0xFFFFFFFFUL));
  require(panel.flush());
  require(panel.setSegment(0, 0, false)); require(panel.flush());

  pinMode(6, OUTPUT); digitalWrite(6, HIGH);
}
void loop() {}
