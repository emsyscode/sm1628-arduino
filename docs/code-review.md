# Review of the original examples

Original source identities and video links are in [sources.md](sources.md).
The two source files were reviewed again for this refactor. Their repositories
were not edited.

## Retained

The data/clock/strobe arrangement, LSB-first sender, conservative burst timing,
command framing, mode-0x03 initialization, paired RAM bytes and seven-word frame
come directly from the old examples. Binary/numeral patterns retain a..g meaning.
The small sketch keeps the communication visible instead of hiding it in classes.

## Reworked

- Separate transport, physical panel mapping and application state.
- Replace repeated numeral-specific switches with small a..g masks.
- Compose an entire frame before sending; avoid hidden clearing tied to one digit.
- Configure only the selected three GPIOs instead of whole DDRD/DDRB registers.
- Remove the unrelated clock ISR, demonstration delays and unused variables.
- Validate addresses/maps/ranges/number widths before mutation or bus activity.
- Preserve brightness independently from display enabled state.
- Keep small examples separate from the buffered library API.

## Concrete issue in the old key scanner

Both source files declare byte array[8], then loop h from 8 down to 1 and write
array[h]. The first iteration writes index 8, outside indices 0..7, while the
printed loop reads indices 7..0. That can corrupt adjacent memory and misalign
the printed bits. The 2019 loop reads five bytes; the 2020 loop reads four.
These observations are not enough to infer the correct byte count or clock
sampling for the exact part. No guessed replacement key protocol was added.

Other comments mix LCD/VFD names with this LED-display code; some command
comments do not match the transmitted operation. The new documentation distinguishes
source-derived implementation from manufacturer/bench confirmation.

## Evidence limits

The legacy six-digit profile is derived from the 2020 code's table and grid
shifts, not a new physical measurement. Our new seven-digit board is still unmapped.
Passing compiled AVR tests establishes software behavior, not board compatibility.
