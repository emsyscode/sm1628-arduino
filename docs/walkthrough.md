CURRENT ROUTING (2026-10-26): the exact former PanelDemo described in this historical baseline record is now firmware/LegacyPanelDemo. The new PanelDemo is a numeric-only physical-order test. See docs/symbol-bit-correction.md for the deliberate correction and new checks. The original refactor was subsequently reported working by the creator; the latest corrected numeric renderer still needs a hardware run.

# Reading the clean sketch

## The three wires

DATA carries a byte least-significant bit first. CLK pulses the bits. STB frames
a command or an address-and-data packet. The clean sketch preserves both sender
routines because the original uses different delays for commands and burst data.

## The fourteen-byte patterns

Each numeral is stored as fourteen bytes, in the same order as the reference.
The source groups these as pairs for a,b,c,d,e,f,g. Its second bytes contain
nonzero values associated with the particular panel. Preserve those values until
their exact output meaning has been measured; do not reduce this to a standard
seven-bit numeral lookup.

For each test slot, every byte is shifted left by that slot and truncated to
eight bits, exactly as in the original send call. Slot order 0..7 is a test order.

## One displayed test frame

showTestNumeral() selects the source mode, blanks using 0x80, sends address 0xC0
and all fourteen shifted pattern bytes under one STB interval, re-enables at 0x8F,
and holds for 100 ms. The main loop repeats all eighty numeral/slot combinations.

clearPanel() retains fourteen separate address-plus-zero writes. This is more
verbose on the bus than a burst clear, but changing it now would add another
difference from the known working program.

## Small changes, one at a time

First compare the clean repeating test with the original on the SAME wiring.
Then consider simplifying port/Timer1 setup as a separate experiment.
Next record the actual slot and individual-output map. Only after that should
we add joystick reading, a segment API and hardware Snake.

The display is already built into its panel. Its manufacturer chose the wiring;
we are documenting and adapting that wiring, not choosing it ourselves.

