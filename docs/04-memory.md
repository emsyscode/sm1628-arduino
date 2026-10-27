# 4. Understand the display memory

Three different things must be kept separate: **the IC's native outputs**, the
**panel's physical wiring**, and the **order we want on screen**.

## Native mode and this panel

The datasheets describe a native seven-digit, ten-segment mode. Its display RAM
has seven pairs of bytes, at addresses 0x00-0x0D. The display manufacturer used
the segment-output side as ten group-common connections and the digit-output
side as seven shared segment connections. As the engineer explains, this uses
a common-anode display arrangement instead of the native common-cathode layout.

That produces ten groups of seven possible lights on this assembled panel:
seven numeric digits and three symbol groups. It does not add more RAM or
turn the IC into a different part. The wiring is already made; software must
represent it correctly. Another SM1628 panel need not use this arrangement.

## A displayed digit becomes a vertical bit column

In this project each a-g segment row is stored across two bytes:

| Segment row | RAM byte indices | Addresses |
|---|---|---|
| a / top | 0, 1 | 0x00, 0x01 |
| b / upper-right | 2, 3 | 0x02, 0x03 |
| c / lower-right | 4, 5 | 0x04, 0x05 |
| d / bottom | 6, 7 | 0x06, 0x07 |
| e / lower-left | 8, 9 | 0x08, 0x09 |
| f / upper-left | 10, 11 | 0x0A, 0x0B |
| g / middle | 12, 13 | 0x0C, 0x0D |

Columns 0-7 are bits 0-7 of the first byte in each pair. Columns 8-9 are bits
0-1 of the second byte. Other storage bits are not additional connected groups
in this mode. Our leftmost numeric digit is column 0; its top bar is byte 0 bit 0.
The third numeric digit is column 7, so its top bar is byte 0 bit 7.

To draw a 7 in the leftmost position, start with fourteen zero bytes and set
bit 0 in bytes 0, 2, 4. To draw it in the third position, set bit 7 in those bytes.
To add a digit without erasing the others, update that column in the current
buffer rather than clearing the entire buffer first.

```cpp
// Transposed layout: one raw column, one a-g mask, a complete RAM buffer.
void putGroup(uint8_t frame[14], uint8_t column, uint8_t segments) {
  if (column >= 10) return;
  const uint8_t mask = uint8_t(1u << (column % 8));
  for (uint8_t row = 0; row < 7; ++row) {
    const uint8_t address = row * 2 + column / 8;
    frame[address] &= uint8_t(~mask);
    if (segments & (1u << row)) frame[address] |= mask;
  }
}
```

This snippet illustrates our layout; it is not a universal panel renderer.
The sketches use the same column/row principle with their own frame builders.
The raw-column number is also not the engineer's Grid label. The full
[reference map](panel-map.md) records both explicitly.

## Sending the frame

The working sender shifts each byte **least significant bit first**. STB frames
a transaction; CLK rising edges carry input data. Our initialization selects
mode 0x03, write/auto-increment 0x40 and display on/full brightness 0x8F.
For a frame we preserve the working sequence: mode 0x03, display off 0x80,
STB LOW, address 0xC0, fourteen bytes, STB HIGH, display on 0x8F.

A transposed digit spans seven byte-pair rows. Sending the whole buffer keeps
the other numeric and symbol columns consistent and gives every frame builder
one predictable update path. That is why this implementation sends all 14 bytes.
The IC also supports addressed writes; a blanket claim that the protocol
rejects partial writes would be incorrect. The existing clear routine itself
uses individual addressed zero writes. A partial-update implementation would
have to preserve the other bits and update every affected address correctly.

The MCU updates RAM; the SM1628 scans the LEDs internally. A 40 ms game frame
schedule is not the same thing as the IC's multiplex scan frequency. Transport
delays, blanking and register setup remain those of the working implementation.
See [datasheet page references](datasheets.md) and [code guide](code-guide.md).
