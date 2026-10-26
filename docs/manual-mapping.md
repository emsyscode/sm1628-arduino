# Manual mapping diagnostic

Open firmware/ManualMap/ManualMap.ino, select Arduino Uno, then upload using
the existing working wiring. Open Serial Monitor at **9600 baud** and choose
**Newline** or **Both NL & CR**. It starts blank and holds every selection until
you send another command. Reset starts blank again.

| Command | Purpose |
|---|---|
| `slot 0 8` | Source slot 0, numeral 8, including inherited symbol bits |
| `column 0 8` | One raw RAM column, numeral 8, without inherited extra bits |
| `next` / `prev` | Step the current slot, column, or isolated bit |
| `digit 1` | Change numeral on selected slot/column |
| `bit 0 0` | Isolate byte 0, bit 0; byte range 0..13, bit range 0..7 |
| `set 0 0 1` | Set one bit in the current frame; use 0 to clear it |
| `frame` | Print all fourteen RAM bytes |
| `clear` | Blank and hold |
| `show`, `status`, `help` | Resend, inspect mode, or list commands |

Source slots are 0..7. Raw columns are 0..15: 0..7 selects bits in the first
byte of each pair; 8..15 selects bits in the second. This is a storage range,
**not sixteen connected display groups**. Some columns will have no visible output.
Neither index is automatically the engineer's Grid number.

Start with `column 0 8`, record which numeric position (from the left, starting
at 1) or symbols light, then send `next` when ready. Use `bit` to identify
individual bars/icons. Compare with `slot` if an original pattern lights symbols.
Record the exact command and visible result, including unexpected symbols.

Every display update sends all fourteen bytes. Transport helpers and startup
remain based on the working source. **Explicit setup change:** D0 is restored
to input after the inherited whole-port setup, allowing serial reception.
D1..D13 retain the baseline direction configuration; do not connect joystick
or other peripherals to these pins during this diagnostic.

The original PanelDemo is unchanged. This diagnostic compiles and has simulated
UART/GPIO checks, but its new manual operation still needs a panel bench test.
