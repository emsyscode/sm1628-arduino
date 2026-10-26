#include "MappedPanel.h"
#include <string.h>

namespace sm1628 {
namespace {
// Original 2020 a..g numeral masks (9 deliberately omits its bottom bar).
const uint8_t kNumerals[10] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67
};
}

bool MappedPanel::configure(const DigitMap* maps, uint8_t count) {
  if (!maps || count == 0 || count > kMaxDigits) return false;
  uint8_t used[Frame::kSize] = {0};
  for (uint8_t d = 0; d < count; ++d) {
    for (uint8_t s = 0; s < 7; ++s) {
      const Address a = maps[d].segments[s];
      if (a.byte >= Frame::kSize || a.bit >= 8) return false;
      const uint8_t mask = uint8_t(1u << a.bit);
      if (used[a.byte] & mask) return false;
      used[a.byte] |= mask;
    }
  }
  for (uint8_t d = 0; d < count; ++d) maps_[d] = maps[d];
  count_ = count;
  clear();
  invalidate();
  return true;
}

bool MappedPanel::setSegment(uint8_t digit, uint8_t segment, bool on) {
  if (digit >= count_ || segment >= 7) return false;
  return frame_.set(maps_[digit].segments[segment], on);
}

bool MappedPanel::setSegments(uint8_t digit, uint8_t mask) {
  if (digit >= count_) return false;
  return setGlyph(frame_, maps_[digit].segments, mask);
}

bool MappedPanel::setDigit(uint8_t digit, uint8_t value) {
  if (digit >= count_ || value > 9) return false;
  return setSegments(digit, kNumerals[value]);
}

bool MappedPanel::setNumber(uint32_t value, bool leadingZeros) {
  if (count_ == 0) return false;
  uint32_t rest = value;
  for (uint8_t d = 0; d < count_; ++d) rest /= 10;
  if (rest != 0) return false;  // Check width BEFORE changing any digits.

  for (uint8_t offset = 0; offset < count_; ++offset) {
    const uint8_t digit = count_ - 1 - offset;
    if (offset == 0 || value != 0 || leadingZeros)
      setDigit(digit, uint8_t(value % 10));
    else
      setSegments(digit, 0);
    value /= 10;
  }
  return true;
}

void MappedPanel::clearDigits() {
  for (uint8_t d = 0; d < count_; ++d) setSegments(d, 0);
}

bool MappedPanel::flush(bool force) {
  if (count_ == 0) return false;
  if (!force && sent_ && memcmp(frame_.bytes, last_.bytes, Frame::kSize) == 0)
    return true;
  if (!driver_.writeFrame(frame_)) return false;
  last_ = frame_;
  sent_ = true;
  return true;
}
}  // namespace sm1628
