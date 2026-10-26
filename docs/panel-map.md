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

**Still to measure:** correlation between these engineer Grid labels and our
raw RAM columns/source shift slots, plus individual segment/icon assignments.
Do not silently subtract one from Grid labels to make a firmware mapping.
Use [the manual diagnostic](manual-mapping.md) to record that correlation.
