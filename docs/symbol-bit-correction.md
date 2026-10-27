# Why a numeral can also light symbols

On this transposed panel, one byte pair is a segment row across several groups.
An entire two-byte value is not a single character. Bit 0 of the first byte
selects raw column 0, while bit 1 of the second byte selects raw column 9.

The original mixed pattern for numeral 7 begins with `{0x01,0x02}` for its top
row. Sending both bytes at column 0 requests the numeric top bar **and** a light
in the DVD/playback symbol group. It does not require electrical ghosting to
explain those extra symbols.

Normal numeric rendering now starts with a clean buffer and sets only the chosen
numeric column's a-g bits. `column`, `ordered` and normal `slot` commands use
that principle. `legacy-slot` explicitly reproduces the mixed source payload
for diagnosis; it should not be used to isolate a numeral.

Do not fix this by forcing all second bytes to zero in the low-level sender.
Second-byte bits also control legitimate symbol columns 8/9. The renderer must
choose the intended columns; the transport must send the resulting frame.
Individual bit tests can then map icons without corrupting numeric output.
