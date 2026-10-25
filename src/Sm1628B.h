#pragma once
#include <Arduino.h>

namespace sm1628 {

struct Address {
  uint8_t byte;
  uint8_t bit;
};

struct Frame {
  static const uint8_t kSize = 14;
  uint8_t bytes[kSize];

  Frame() { clear(); }
  void clear();
  // These are RAM byte/bit addresses, not physical display pin numbers.
  bool set(Address address, bool on);
};

// a,b,c,d,e,f,g order. Caller supplies a measured, board-specific map.
// Rejects an invalid/aliased map before changing the frame.
bool setGlyph(Frame& frame, const Address (&map)[7], uint8_t segments);

class Sm1628B {
 public:
  Sm1628B(uint8_t data, uint8_t clock, uint8_t strobe)
      : data_(data), clock_(clock), strobe_(strobe),
        brightness_(0), enabled_(false), ready_(false) {}

  // Uses mode 0x03 and 14 RAM bytes from the original SM1628B examples.
  // Brightness 0 is the lowest enabled setting; setEnabled(false) blanks.
  bool begin(uint8_t brightness = 0);
  bool writeFrame(const Frame& frame);
  // Nonempty, bounded auto-increment write; invalid ranges send nothing.
  bool writeRange(uint8_t start, const uint8_t* data, uint8_t length);
  bool clear();
  bool setBrightness(uint8_t level);
  bool setEnabled(bool enabled);

 private:
  void sendByte(uint8_t value);
  void command(uint8_t value);
  void displayControl();
  uint8_t data_, clock_, strobe_, brightness_;
  bool enabled_, ready_;
};
}  // namespace sm1628
