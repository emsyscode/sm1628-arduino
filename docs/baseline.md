# Baseline and evidence

## Supplied source

File: SM1628B_Panel_DVD_Reader_7digits_sanake_v0.ino, supplied 2026-10-26.
SHA-256: 4E9521DC761201B3C6FEB347DB51230398487458F13151BA866038D02293B9B4.

The creator reports this sketch works across all seven numeric digits on their built-in display panel. Their reported visible order is recorded separately in [panel observations](panel-observations.md), pending mapping tests and their reverse-engineering photo. This
supersedes the earlier generated library as our practical starting point.
The original is copied without changes under reference/WorkingPanel.

Its active loop is test_panel_DVD(), then clear_VFD(). It does not run Snake,
the clock loop, readButtons(), write_panel_DVD() or test_DSP().
Those functions are present in the reference but inactive.

## Preserved in PanelDemo

- Uno D7 DATA, D8 CLK, D9 STB and the original whole-port setup.
- Timer1 setup and interrupt-side state changes, despite being unused by the
  active display test. They remain solely for conservative comparison.
- Startup: 200 us delay, commands 03,40,C0,8F and their extra 1 us waits.
- Separate 1 us command timing and 5 us burst timing.
- Fourteen individually addressed zero writes for clear.
- All 140 numeral-pattern bytes, including second-byte bits.
- Slots 0..7, numerals 0..9, byte truncation after shifting.
- For every numeral: 03,80, a C0+fourteen-byte packet, then 8F.
- The 100 ms hold after each test numeral.

Slots are source bit-shift values, not verified front-view digit numbers.
Eight test slots do not establish eight visible numeric digits on a seven-digit
panel. Record which items light at each slot before naming a board profile.

## Refactoring performed

The ten repeated switch branches become one table lookup. The wider integer
arrays become literal byte tables only at the point where the original already
truncated transmitted values to bytes. Unused includes/variables/functions and
the inactive clock application are omitted. Function/pin names describe the
actual role rather than inherited LCD/VFD terminology.

No generic glyph map, guessed icon positions, reduced brightness, alternate
startup or abbreviated refresh sequence was substituted.
All new code remains a hardware-retest candidate.

## Why not immediately remove the AVR registers?

The working setup drives both DDRD and DDRB as outputs and clears their PORT
registers. That affects much more than the three bus wires. We do not yet know
whether any other connection on the current bench depends on those pin states.
Keeping this setup avoids changing that variable while testing the refactor.
It also means this baseline must not be combined with joystick wiring yet.

The reference includes an inactive key-reader out-of-bounds write. That code
is preserved only in the untouched source, not enabled or copied into PanelDemo.
