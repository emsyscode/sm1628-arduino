# Reference DVD panel map

This table relates the visible digits and symbols to RAM columns and the Grid
labels on the [wiring drawing](../reference/panel-reverse-engineering-original.jpeg).
The drawing numbers numeric positions from the right; this table numbers them
from the left. Neither numbering system refers to IC package pins.

| Physical function | Numeric position from left | Raw RAM column | Grid label on drawing |
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

The complete symbol groups are listed above. The `column 0 7`, then `n` test
lights only the bars and icons selected by the 7 mask. Use individual-bit tests
to map the rest of a group. The game does not select an isolated central-colon bit.

Numeric lookup from left to right: `{0, 1, 7, 2, 3, 4, 5}`.
Grid labels on the drawing, in the same physical order: `{6, 8, 7, 5, 4, 3, 2}`.
Do not obtain one by adding/subtracting one from the other.

For another panel, use [manual mapping](manual-mapping.md), retain your measured
table and adapt only the relevant sketch. Reusing this lookup is appropriate
only if your panel produces the same observed results.
