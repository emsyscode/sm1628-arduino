# SM1628 and SM1628B: one programming tutorial

The B variant is essentially the same driver for the display operations in this
tutorial. Use the same examples for either IC: there is no B-specific command
path or sketch setting to select.

![Shared driver interface and panel-specific mapping](images/driver-and-panel.svg)

## What stays the same

Both datasheets describe the three-wire DATA/CLK/STB interface, the native
seven-digit/ten-segment mode and the 14-byte display RAM at addresses
0x00–0x0D. The examples explicitly select mode `0x03`, write/auto-increment
`0x40`, and the display-control commands. They do not rely on a power-on mode
being identical between revisions.

| Detail | SM1628 | SM1628B |
|---|---|---|
| Display programming used here | Same interface and commands | Same interface and commands |
| Native mode used here | 7 digits × 10 segments | 7 digits × 10 segments |
| Display RAM | 14 bytes | 14 bytes |
| Oscillator in the referenced datasheet | Internal RC; pin 1 is NC | Pin 1 is OSC; external frequency-setting resistor to GND |
| Protection | See its electrical specifications | Datasheet describes improved electrostatic and power-interference protection |

For an assembled panel, its manufacturer has already provided the oscillator
arrangement. If building a circuit around the bare IC, follow that exact part's
datasheet: pin 1 is not interchangeable between these two documents. The B's
external setting resistor is not an external oscillator module. Improved
protection does not change the display programming or replace correct wiring.

These differences refer to the [specific Sunmoon datasheets](datasheets.md)
linked here: SM1628 p1–2 and SM1628B p1, p3. Check the revision when designing
your own board.

## What depends on the panel instead

The IC name does not tell you the connector order, the physical digit order,
which bits control icons, or whether the manufacturer swapped the roles of
the segment and digit connections.

Our reference panel uses that swap to drive **ten groups of seven lights**,
with a common-anode display arrangement. Another SM1628 or SM1628B panel can
use the native arrangement or a different order. The serial commands remain
useful; the frame construction and lookup table must match that wiring.

The hardware runs reported for this project used the reference SM1628B panel.
Support for SM1628 display programming follows the shared documented interface;
it does not establish a universal panel map or a physical test of every panel.

Start with [connections](01-connect.md), then use [ManualMap](manual-mapping.md)
for an unfamiliar panel. The [memory guide](04-memory.md) explains how to turn
the map into the bytes you send.
