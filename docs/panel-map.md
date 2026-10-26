# Engineer's panel map

Source: Jose Luis Monteiro's explanation relayed by the creator, 2026-10-26.
The creator confirmed all numeric positions below are counted **from the right**.
Original Portuguese: [preserved explanation](engineer-panel-explanation.pt.md).

| Engineer's Grid label | Panel group |
|---|---|
| 1 | return, left colon, right colon, Dolby, DTS |
| 2 | first numeric digit from right |
| 3 | second numeric digit from right |
| 4 | third numeric digit from right |
| 5 | fourth numeric digit from right |
| 6 | seventh numeric digit from right (leftmost) |
| 7 | fifth numeric digit from right |
| 8 | sixth numeric digit from right |
| 9 | Wheels symbol |
| 10 | DVD, VCD, MP3, PBC, Play, Pause, last Wheels symbol |

Numeric digits **left to right: Grid 6, 8, 7, 5, 4, 3, 2**.

The panel has seven numeric positions and three symbol groups. The manufacturer
swapped segment and grid/digit roles to obtain ten groups of seven segments,
using a common-anode display in place of the native common-cathode arrangement.
Existing panel wiring dictates the software map.

The native memory has seven two-byte rows. Under the panel's transposed wiring,
a displayed group is represented by a vertical bit column across those rows.
Our implementation sends complete fourteen-byte frames to update the display.
The chip protocol also supports addressed writes; full frames are our mapping
and update strategy, not a claim that every partial protocol write is invalid.

## Photographed raw-column correlation — 2026-10-26

Creator ran column 0 7 followed by one n per photo. Observed numeric digits
left to right use raw columns **0,1,7,2,3,4,5**. These correspond by physical
position to the engineer Grid labels above; do not derive one by subtracting one.
Raw6 lit return/both colons, raw8 three wheel wedges, raw9 DVD/VCD/MP3.
The 7 pattern confirms the numeric positions and three bars, not all bars/icons.

ManualMap now offers ordered mode using this map. Its new ordered stepping
still needs a hardware run; raw-column selection has the photo evidence.
[Instructions, expected outputs and troubleshooting](manual-mapping.md).
