# 1. Identify and bring up the panel

This guide starts at the bench, before assuming that a downloaded sketch matches
a salvaged board.

1. Photograph the front, back, IC marking and connector. Record the exact suffix.
2. Obtain the datasheet for that exact part. Check supply, signal levels, pinout,
   supported display mode and RAM format. Record any discrepancy with old code.
3. Trace the board connector to power, ground, data, clock and strobe. Record
   connector positions separately from Arduino GPIO numbers and IC pin numbers.
4. Ask the engineer to confirm the wiring, supply arrangement and current limits.
   This project does not yet supply a verified universal wiring diagram.
5. For the legacy arrangement, the examples select Arduino D7 DATA, D8 CLK,
   D9 STB. Reassign these to match the actual wiring. Never copy wire colours
   from a different board.
6. Confirm the part supports this driver's inherited mode 0x03 and 14-byte write.
7. Enable kBenchReady in SegmentWalk. Begin at the lowest enabled brightness,
   and record which physical item responds to each printed RAM address/bit.
8. Stop and investigate if behavior conflicts with the documented supply or map.

Pass condition: a repeatable single-output test, a recorded connector map and
a confirmed driver revision. A successful compile alone is not this pass condition.
Next: [make the segment map](02-mapping.md).

Capture useful teaching material while doing this: a clear board overview, the
IC marking, each connection, and the single-output test paired with its address.
