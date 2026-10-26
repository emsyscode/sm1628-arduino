# Why selecting the leftmost digit also lit the left-hand symbols

Reported by the creator, 2026-10-26: original looping/individual slot tests
light the left-side predefined symbols with the first numeric digit, while
ordered mode isolates that digit correctly.

## Byte-level cause

The working source stores seven pairs of bytes for each numeral. In this panel's
transposed arrangement, each pair addresses one a..g row across several display
groups, not a self-contained character. In the first byte, bit0 selects raw
column0 (leftmost numeric position). In the second byte, bit1 selects raw
column9 (the DVD/VCD/MP3/playback symbol group). Both can be selected together.

For example, numeral7 begins with these pairs for a and b:

| Row | Original pair | What is selected at source slot0 |
|---|---|---|
| a | 0x01,0x02 | Numeric top bar AND one bit of symbol column9 |
| b | 0x01,0x00 | Numeric upper-right bar only |

Most second bytes are0x02, even for rows whose numeric stroke is off.
The original loop shifts and sends BOTH bytes:
`uint8_t(uint16_t(pattern[i]) << slot)`.
At slot0 the unwanted0x02 survives in the second byte, selecting symbol column9.
At later slots it moves to higher second-byte bits or is truncated; it does not
stay attached to the same physical left-hand symbol group.

Thus the extra symbols are explicitly requested by the payload. No ghosting,
failed clear or electrical crosstalk is needed to explain this difference.
The original author's intent for the extra bits is unknown; the error is treating
this mixed digit/icon payload as a pure numeral when trying to isolate a digit.

## Why ordered was clean

Ordered mode starts with fourteen zero bytes, extracts only the a..g stroke bits
from the first byte of each pair, and writes them to ONE mapped numeric column.
Raw column mode already did the same for its selected storage column.
No second-byte icon mask is copied. PanelSnake independently uses this same
numeric-only principle and is unchanged by this correction.

## Correction

- PanelDemo: numeric-only numeral0..9 test on seven physical numeric positions,
  left to right, using columns0,1,7,2,3,4,5. Symbol groups skipped.
- ManualMap slot: now uses only numeral strokes at raw column0..7, matching
  column mode for that range. slot0 7 and column0 7 isolate the same leftmost7.
- ManualMap ordered/column: already clean; unchanged rendering.
- SlotMap: timed raw-slot test now removes copied second-byte icon bits.
  Raw slot6 still intentionally selects the return/colon symbol column.
- legacy-slot N D: explicitly requests the previous mixed digit/icon masks.
- LegacyPanelDemo: exact former PanelDemo retained for source comparison.
- Untouched supplied source remains byte-identical under reference/WorkingPanel.

Normal numeric rendering clears all unused bits in the complete14-byte frame.
Raw storage diagnostics must still permit deliberately selecting symbol groups;
clearing every second byte unconditionally in the low-level sender would break
those legitimate tests. The correction belongs in frame construction.

## What was preserved before, and why

The first refactor deliberately reproduced the known-working sketch's complete
payloads, including these extra bits, after earlier generated drivers failed.
That verified communication compatibility, not that every bit was required for
isolated digits. Carrying the same payload into the default slot command made
an isolated-digit command misleading. The legacy behavior is now explicitly
named; ordinary numeral commands build clean frames.

## Validation

Compiled Uno checks cover all70 numeric numeral/position combinations and reject
any unintended symbol bits. Manual checks compare clean slot/column/ordered
frames, reproduce legacy symbol bits, and verify that switching back to clean
slot removes those bits. The legacy demo still matches all380 original
transactions. Corrected sketches compile; panel confirmation is pending.
