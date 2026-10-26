# Panel observations — creator report, 2026-10-26

The newly supplied sketch works across all seven visible numeric digits.
The earlier generated examples produced no display output, according to the
creator. The exact earlier build/guard settings remain unknown.

The visible numeric-digit test order is reported as:

    3 -> 1 -> 2 -> 4 -> 5 -> 6 -> 7

Positions here are counted from the LEFT, starting at ONE. This describes only
the numeric digits, not symbols or every driver output.

This is not yet a verified table of source slot/bit -> physical digit. The
working source tests slots 0..7; do not zip that eight-slot list with the seven
numeric positions or guess what an unaccounted slot controls.

The creator will provide a picture of the initial reverse engineering.
After inspecting it, confirm the order by testing individual outputs and
recording their visible positions. Keep PanelDemo's original test sequence
unchanged until that map is verified.
