# Protocol notes

Initial scope is the SM1628B arrangement demonstrated by the legacy examples,
not an implementation of every 1628-family data sheet.

The implementation inherits:
- LSB-first data with rising clock edges, STB low during a transaction.
- Mode command 0x03.
- 0x40 for incrementing display-data writes.
- 0xC0 as RAM start address, followed by fourteen bytes.
- 0x80 / 0x88 plus a three-bit brightness setting for display control.
- 200 us startup allowance and conservative 5 us clock half-periods.

These are source-derived choices. Exact part review and optical/electrical bench
validation are still pending. The wire-level simulator checks what the program
sends; it does not emulate the IC or certify these values.

begin blanks the display, selects the mode, clears RAM, then enables it at the
requested brightness. Brightness 0 is enabled at its lowest setting; it is not
the off command. setBrightness rejects values above 7 without a transaction and
preserves enabled/disabled state.

writeFrame always sends data mode separately, then address plus all fourteen
bytes under one STB interval. No frame buffer is cleared implicitly after writing;
the caller can retain a static image or change only particular bits.

No key-read routine is included yet. The old readout changes the data pin's
direction; exact read length, electrical direction handling and board keys must
be checked before adding that API. IR reception belongs in a separate example,
not in the display transport.
