# Fuller library version

Include Sm1628B.h for raw frames, or MappedPanel.h for mapped display operations.
All APIs use fixed-size buffers and return false for invalid arguments. No heap
allocation or background timer is required. Calls are for the foreground loop;
do not call the same instance concurrently from an interrupt.

## Transport: Sm1628B

- begin(level): validate distinct pins/level, blank, select inherited mode,
  clear RAM, enable. Brightness is 0..7.
- writeFrame(frame): send all fourteen bytes.
- writeRange(start, data, length): send a validated contiguous RAM range.
  Empty/null/overflowing ranges are rejected with no bus activity.
- clear(), setBrightness(level), setEnabled(on): explicit display control.

These are synchronous software sends, not hardware acknowledgements.
writeRange is useful for a known icon byte. It still establishes incrementing
data mode and a start address; no unsupported fixed-address mode is introduced.

## Buffered display: MappedPanel

1. Construct it with a driver reference.
2. configure(maps, count) copies 1..7 DigitMap entries in chosen logical order.
   Each map contains seven RAM addresses, a through g.
3. Change digits, numbers, individual segments or raw outputs.
4. flush() sends the completed frame only if it differs from the previous send.

setDigit accepts decimal 0..9. setSegments accepts a seven-bit mask.
setNumber is unsigned, right aligned, and optionally zero padded. Values that
do not fit leave all outputs unchanged. clearDigits preserves unrelated icons;
clear clears the entire frame. configure rejects overlapping digit addresses
across the whole map before replacing the current configuration.

Use setOutput for an icon address established on the bench. It is deliberately
raw and may also alter a digit if the caller supplies a digit's RAM address.

The buffer is owned by the panel, so brightness/blanking do not discard it.
flush(true) forces restoration after an IC reset or raw driver write. invalidate()
marks the cached frame stale. After calling driver.begin() again, use one of those;
the library cannot detect a hardware reset or writes through another object.

## Legacy six-digit example

src/profiles/LegacySixDigit.h reproduces the numerical mapping inferred from the
2020 source, ordered as its clock's HHMMSS values (grid bits 9,8,7,1,2,3).
This is CODE_DERIVED, not a newly bench-verified front-view profile.
examples/BufferedCounter demonstrates the API with that explicit legacy map.
Do not select it for the new seven-digit board without matching the hardware.

## Features deliberately left for exact-part review

No generic key-read API or broad TM1628/SM1628C compatibility is claimed.
The original key scanners use different read lengths and an out-of-bounds
array index. Fixing the array alone cannot establish the correct read protocol.
Those functions require datasheet/bench confirmation before becoming an API.
