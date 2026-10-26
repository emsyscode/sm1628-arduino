#include "Sm1628B.h"

namespace sm1628 {
void Frame::clear() {
  for (uint8_t i = 0; i < kSize; ++i) bytes[i] = 0;
}

bool Frame::set(Address address, bool on) {
  if (address.byte >= kSize || address.bit >= 8) return false;
  const uint8_t mask = static_cast<uint8_t>(1u << address.bit);
  if (on) bytes[address.byte] |= mask;
  else bytes[address.byte] &= static_cast<uint8_t>(~mask);
  return true;
}

bool setGlyph(Frame& frame, const Address (&map)[7], uint8_t segments) {
  if (segments & 0x80) return false;
  for (uint8_t i = 0; i < 7; ++i) {
    if (map[i].byte >= Frame::kSize || map[i].bit >= 8) return false;
    for (uint8_t j = 0; j < i; ++j) {
      if (map[i].byte == map[j].byte && map[i].bit == map[j].bit)
        return false;
    }
  }
  for (uint8_t i = 0; i < 7; ++i)
    frame.set(map[i], (segments & (1u << i)) != 0);
  return true;
}

void Sm1628B::sendByte(uint8_t value) {
  // LSB first, sampled on rising CLK. Delays inherit the conservative
  // 5 us half-period used in the original burst sender.
  for (uint8_t bit = 0; bit < 8; ++bit) {
    digitalWrite(clock_, LOW);
    digitalWrite(data_, (value & 1) ? HIGH : LOW);
    delayMicroseconds(5);
    digitalWrite(clock_, HIGH);
    delayMicroseconds(5);
    value >>= 1;
  }
}

void Sm1628B::command(uint8_t value) {
  digitalWrite(strobe_, LOW);
  delayMicroseconds(1);
  sendByte(value);
  digitalWrite(strobe_, HIGH);
  delayMicroseconds(1);
}

void Sm1628B::displayControl() {
  command(static_cast<uint8_t>(0x80 | (enabled_ ? 0x08 : 0) | brightness_));
}

bool Sm1628B::begin(uint8_t brightness) {
  if (brightness > 7 || data_ == clock_ || data_ == strobe_ ||
      clock_ == strobe_) return false;
#if defined(NUM_DIGITAL_PINS)
  if (data_ >= NUM_DIGITAL_PINS || clock_ >= NUM_DIGITAL_PINS ||
      strobe_ >= NUM_DIGITAL_PINS) return false;
#endif
  // Establish idle levels before enabling the three outputs.
  digitalWrite(strobe_, HIGH);
  digitalWrite(clock_, HIGH);
  digitalWrite(data_, LOW);
  pinMode(strobe_, OUTPUT);
  pinMode(clock_, OUTPUT);
  pinMode(data_, OUTPUT);
  delayMicroseconds(200);
  brightness_ = brightness;
  enabled_ = false;
  ready_ = true;
  displayControl();       // Blank before configuration and RAM clearing.
  command(0x03);         // Legacy seven-scan layout; exact part review pending.
  clear();
  enabled_ = true;
  displayControl();
  return true;
}

bool Sm1628B::writeFrame(const Frame& frame) {
  return writeRange(0, frame.bytes, Frame::kSize);
}

bool Sm1628B::writeRange(uint8_t start, const uint8_t* data, uint8_t length) {
  if (!ready_ || !data || length == 0 || start >= Frame::kSize ||
      length > Frame::kSize - start) return false;
  command(0x40);  // Auto-increment writes, also for a partial range.
  digitalWrite(strobe_, LOW);
  delayMicroseconds(1);
  sendByte(uint8_t(0xC0 | start));
  for (uint8_t i = 0; i < length; ++i) sendByte(data[i]);
  digitalWrite(strobe_, HIGH);
  delayMicroseconds(1);
  return true;
}

bool Sm1628B::clear() {
  const Frame blank;
  return writeFrame(blank);
}

bool Sm1628B::setBrightness(uint8_t level) {
  if (!ready_ || level > 7) return false;
  brightness_ = level;
  displayControl();
  return true;
}

bool Sm1628B::setEnabled(bool enabled) {
  if (!ready_) return false;
  enabled_ = enabled;
  displayControl();
  return true;
}
}  // namespace sm1628
