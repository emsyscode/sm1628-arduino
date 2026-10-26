#include <MappedPanel.h>
#include <profiles/LegacySixDigit.h>

#ifndef SM1628_BENCH_READY
#define SM1628_BENCH_READY 0
#endif

sm1628::Sm1628B driver(7, 8, 9);
sm1628::MappedPanel panel(driver);
uint32_t count = 0;
unsigned long lastUpdate = 0;
bool ready = false;

void setup() {
  if (!SM1628_BENCH_READY) return;
  // This is the 2020 six-digit example's CODE_DERIVED map, not our new panel.
  ready = panel.configure(sm1628::profiles::kLegacySixDigit, 6) &&
          driver.begin(0);
  if (ready) {
    panel.setNumber(0);
    panel.flush();
  }
}

void loop() {
  if (!ready || millis() - lastUpdate < 100) return;
  lastUpdate = millis();
  count = (count + 1) % 1000000UL;
  panel.setNumber(count);
  panel.flush();  // One send after composing the whole number.
}
