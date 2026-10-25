#pragma once
#include "Sm1628B.h"

namespace sm1628 {
struct DigitMap {
  Address segments[7];  // a,b,c,d,e,f,g, in that order.
};

// Optional higher-level layer. No heap allocation, timers or hidden refresh.
// Render into a RAM buffer, then flush once when the complete picture is ready.
class MappedPanel {
 public:
  static const uint8_t kMaxDigits = 7;
  explicit MappedPanel(Sm1628B& driver)
      : driver_(driver), count_(0), sent_(false) {}

  // Copies the map. All digit outputs must be distinct, within fourteen bytes.
  // An invalid replacement leaves the previous configuration and frame intact.
  bool configure(const DigitMap* maps, uint8_t count);
  uint8_t digitCount() const { return count_; }
  bool setSegment(uint8_t digit, uint8_t segment, bool on);
  bool setDigit(uint8_t digit, uint8_t value);  // Values 0..9.
  bool setSegments(uint8_t digit, uint8_t mask); // bit0=a ... bit6=g.
  // Right-aligned unsigned decimal. Overflow leaves the frame unchanged.
  bool setNumber(uint32_t value, bool leadingZeros = false);
  bool setOutput(Address address, bool on) { return frame_.set(address, on); }
  void clearDigits();   // Preserve unrelated icons.
  void clear() { frame_.clear(); }  // Clear digits and icons.
  const Frame& frame() const { return frame_; }

  // Same frame => no bus write. force=true restores after a reset/external write.
  // true means accepted/sent by software; the IC provides no acknowledgement.
  bool flush(bool force = false);
  void invalidate() { sent_ = false; }

 private:
  Sm1628B& driver_;
  DigitMap maps_[kMaxDigits];
  Frame frame_, last_;
  uint8_t count_;
  bool sent_;
};
}  // namespace sm1628
