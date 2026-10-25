# Small standalone version

Open examples/Minimal/Minimal.ino directly in Arduino IDE. It has no dependency
on this library. Read [board bring-up](01-bring-up.md)
before enabling SM1628_BENCH_READY.

The sketch keeps only the original communication sequence:
1. Send a byte least-significant bit first.
2. Frame a command with STB.
3. Send 0x40, then 0xC0 and fourteen RAM bytes under one STB interval.
4. Initialize mode 0x03, clear, and enable at low brightness.

The loop toggles one illustrative RAM bit every 500 ms using millis().
No clock, key scanner, numeral tables, interrupts or library classes are needed
to understand this first step. The bit's physical location is board-specific.

The new initialization blanks/clears before enabling. The sender retains the
original burst sender's conservative 5 us clock half-period. Comments calling
this LCD/VFD logic were removed: they were inherited names, not device evidence.

The fourteen-byte, mode-0x03 arrangement is taken from the original SM1628B
examples, not a universal contract for all 1628-marked parts. Full sources:
[2019](https://github.com/emsyscode/SM1628B) and
[2020](https://github.com/emsyscode/SM1628B_display_6digits).

Move to [MappedPanel](api.md) when you want digits, numbers, icons and buffered
updates without adding that complexity to the small teaching sketch.
