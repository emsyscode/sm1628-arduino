# 2. Separate the panel map from the driver

The old panels illustrate why the same driver name does not imply the same
display layout. Numeric positions, icons and wheel pieces can use different
outputs; the original examples also describe swapping the grid/segment roles.

Keep these three labels separate:
- Arduino GPIO number: the wire driven by the program.
- Driver RAM byte/bit: the pattern sent to the IC.
- Visible item: digit position and segment a through g, or an icon.

Use SegmentWalk to record the last two. Save observations in a copy of
profiles/template.json, with a clear front-view digit order. An unlit or ambiguous
address stays unknown until resolved. Record icons separately from numeric digits.

For each digit, supply seven Address entries in a,b,c,d,e,f,g order to setGlyph.
Bits 0 through 6 of the glyph select those named segments. Conventional digit 1,
for example, is 0x06 (b and c), but the seven RAM addresses come from YOUR board.
setGlyph first validates the whole map, so a bad address cannot partially modify
a frame. Other outputs in the frame are preserved.

For Snake, each of the 49 browser segments eventually resolves through this
measured map to a RAM bit. The browser graph and physical output mapping are
different layers. A turn in the simulator is not evidence of the hardware map.

Pass condition: all intended numeric segments can be selected individually,
their position/order is agreed, and icons remain controlled independently.
