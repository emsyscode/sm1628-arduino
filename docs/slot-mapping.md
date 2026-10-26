# One-pass slot mapping

Open firmware/SlotMap/SlotMap.ino and upload to the SAME Uno and wiring used by
the working PanelDemo. Open Serial Monitor at **9600 baud**.

The test waits three seconds, then prints SLOT 0 through SLOT 7. Each slot shows
the original pattern for numeral 8 for approximately three seconds, separated
by a half-second blank. It clears the panel and stops after one pass. Reset to
repeat. A monitor-triggered reset restarts the same single pass.

Record what lights at each printed label. Number the seven numeric positions
from the LEFT starting at ONE. Record symbols separately, even if a numeric
digit also lights. Leave an uncertain entry unknown.

| Printed slot | Numeric position(s) | Symbols/other outputs |
| --- | --- | --- |
| 0 | | |
| 1 | | |
| 2 | | |
| 3 | | |
| 4 | | |
| 5 | | |
| 6 | | |
| 7 | | |

The source pattern includes nonzero second bytes, so this diagnostic may also
light symbols. It deliberately retains those bytes for the first mapping pass.
It does not yet isolate every individual RAM bit or change the confirmed demo.

## Why the apparent start can move

PanelDemo always restarts its source loop at slot 0. There is no first-cycle
special case. A repeating sequence ending in physical digit 3 and restarting
at physical digit 1 can be perceived as a sequence beginning at digit 3.
This is a possible interpretation, not a proven hardware explanation.

Latest creator observation: first observed sequence 1,2,4,5,6,7,symbols,3;
later it appears to start at 3,1,2. This supersedes treating the earlier verbal
sequence alone as a settled mapping. Use the printed slot labels to resolve it.

## Scope

Same startup, pin assignments, whole-port/Timer1 setup, pattern bytes and
command/burst senders as PanelDemo. Only the loop, selected numeral, hold schedule
and serial messages differ. It still configures Uno D0..D13 as outputs; do not
add joystick wiring to this test. No firmware was uploaded by the assistant.

Correction2026-10-26: SlotMap now sends only first-byte numeral strokes; inherited second-byte icon masks are cleared. Raw slot6 still deliberately selects the return/colon group. The earlier exact payload is preserved in LegacyPanelDemo and ManualMap legacy-slot. See symbol-bit-correction.md.


