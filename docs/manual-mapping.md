# 3. Map your own panel

Start with [display-only wiring](01-connect.md) and upload **ManualMap**.
Serial Monitor: **9600 baud**, **Newline** or **Both NL & CR**. Wait for READY.
Commands below go on separate lines. Each result is held until changed.

## Discover raw columns

First send:

```text
column 0 7
```

Then send `n` once per step. `p` goes back. Record the command, physical position
and visible bars/symbols. A column number describes RAM bits, not digit order.

| Raw column | What numeral 7 lights on the reference panel |
|---|---|
| 0 | First numeric digit from left |
| 1 | Second digit |
| 2 | Fourth digit |
| 3 | Fifth digit |
| 4 | Sixth digit |
| 5 | Seventh/rightmost digit |
| 6 | Return and both colons |
| 7 | Third digit |
| 8 | Three wheel wedges |
| 9 | DVD, VCD and MP3 |

Raw stepping includes columns 0-15 and wraps. Sixteen is the storage-bit range,
not sixteen connected digits. No visible result is established for 10-15.
The numeral 7 tests only a, b and c. It does not identify every symbol or bar.

## Test physical numeric order

Send `ordered`: it starts again at the leftmost digit with numeral 7. Then
`n`/`p` walks the seven numeric positions, wrapping at either end and skipping
symbol groups. The built-in left-to-right lookup is:

```cpp
constexpr uint8_t kDigitColumns[] = {0, 1, 7, 2, 3, 4, 5};
```

This is our panel's lookup. Change it only after recording your own raw results.
`ordered` always resets the selection; `column 0 7` returns to raw stepping.

## Check all bars and isolate symbols

- After selecting a mode, `digit 8` lights its complete numeral shape;
  `digit 7` restores the photographed pattern.
- `bit 0 0` selects byte 0, bit 0 alone. Now `n`/`p` walks all 112 storage bits.
- `set 0 0 1` sets byte 0, bit 0 in the current frame; `set 0 0 0` clears it.
- `frame` prints all 14 bytes. `show` resends the current frame.
- `clear` blanks and cancels stepping. `status` describes the selection.
- `help` prints the full command menu without changing the picture.

Valid bytes: 0-13; bits: 0-7; numerals: 0-9. A frame made with `set` is a
custom frame, with no stepping sequence; select a column or bit again for `n`.
To investigate raw column 6, test byte 0 bit 6, byte 2 bit 6, through byte 12 bit 6
separately. Record the exact symbol for each. Do not enable the whole group
merely to add a colon: that can also enable return/other symbols.

## Record the map

Use one row per observed light, including icons:

| Byte | Bit | Physical position | Bar/icon | Confirmed result |
|---|---|---|---|---|
| 0 | 0 | Leftmost numeric digit on our panel | a/top | Reference example |
| ... | ... | ... | ... | ... |

For the reference transposed panel, segment a-g corresponds to byte-pair row
0-6. Verify each row on another panel before copying that assumption. PanelSnake
also offers `m`, then `n`, at 115200 baud to inspect its 49 mapped bars.

## Apply a different map

The self-contained display sketches each carry `kDigitColumns`; change the
lookup in each sketch you intend to use. A different bar order requires changing
the segment-to-row mapping too, not only this digit lookup. A native-wired panel
requires a different frame layout: swapping two arrays alone cannot transpose
the electrical arrangement. Start from [the memory explanation](04-memory.md).

`slot N D` is a clean shorthand for raw columns 0-7. `legacy-slot N D` remains
an explicit compatibility diagnostic for mixed numeral/icon payloads; it is not
the normal isolated-digit command. See [unwanted symbols](symbol-bit-correction.md).
Invalid lines leave the picture unchanged; lines longer than 47 characters are
discarded. If `n` has no selected sequence, send `column 0 7` again.
