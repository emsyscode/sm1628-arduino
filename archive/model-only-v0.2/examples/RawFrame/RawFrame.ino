#include <Sm1628B.h>

sm1628::Sm1628B display(7, 8, 9);
constexpr bool kBenchReady = false;
// Placeholder address. Replace with an entry measured using SegmentWalk.
constexpr sm1628::Address kMeasuredSegment = {0, 0};

void setup() {
  Serial.begin(115200);
  if (!kBenchReady) {
    Serial.println(F("Set up and map your panel before enabling this example."));
    return;
  }
  display.begin(0);
  sm1628::Frame frame;
  frame.set(kMeasuredSegment, true);
  display.writeFrame(frame);
}
void loop() {}
