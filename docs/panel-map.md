# Reference DVD panel map

This table combines the engineer's group identification with photographed raw
column tests. Engineer numeric positions count from the right. Firmware numeric
positions below count from the left. Neither is an IC package-pin number.

| Physical function | Numeric position from left | Raw RAM column | Engineer Grid label |
|---|---:|---:|---:|
| Leftmost numeric digit | 1 | 0 | 6 |
| Numeric digit | 2 | 1 | 8 |
| Numeric digit | 3 | 7 | 7 |
| Numeric digit | 4 | 2 | 5 |
| Numeric digit | 5 | 3 | 4 |
| Numeric digit | 6 | 4 | 3 |
| Rightmost numeric digit | 7 | 5 | 2 |
| Return, left/right colons, Dolby, DTS | - | 6 | 1 |
| Wheel group | - | 8 | 9 |
| DVD, VCD, MP3, PBC, Play, Pause, remaining wheel light | - | 9 | 10 |

The engineer identifies the complete groups. The photographed `column 0 7`
then `n` test establishes numeric positions and only the subset of icons enabled
by the 7 mask. It does not identify every individual icon bit. In particular,
an isolated central-colon bit is not assigned in the game.

Numeric lookup from left to right: `{0, 1, 7, 2, 3, 4, 5}`.
Engineer Grid labels in the same physical order: `{6, 8, 7, 5, 4, 3, 2}`.
Do not obtain one by adding/subtracting one from the other.

For another panel, use [manual mapping](manual-mapping.md), retain your measured
table and adapt only the relevant sketch. Reusing this lookup is appropriate
only if your panel produces the same observed results.
