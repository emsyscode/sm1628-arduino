# Sources and development status

## SM1628 / SM1628B and transposed panel wiring — creator report, 2026-10-26

Latest explanation supplied by the creator, retained as project technical input.
This records the report; no new manufacturer-document verification, pinout,
oscillator implementation or hardware measurement was performed in this update.

Verbatim:
> The SM1628 / SM1628B are basically the same, but the B has an internal protection anti static and it has an external oscillator. Otherwise they are the same.
>
> The important part is that most manufacturers of the panels they are on, to deal with the fact that they were limited to 7 digits, they swapped the grids with the segments. In practice, this means inverting the whole code and pins to control that.

Working interpretation:
- Treat SM1628 and SM1628B as the same basic control family on the creator's
  explanation, with the stated B-version differences above. Preserve "external
  oscillator" as reported; do not invent oscillator components or connections.
- Panel manufacturers swap the roles of grid and segment outputs to work around
  the seven-grid/digit limitation. The creator describes this as common practice;
  its prevalence is reported, not independently quantified here.
- "Inverting the code and pins" refers here to transposing the display's
  grid/segment relationship and adapting the output/bit map and pin-role labels.
  It does not by itself specify complemented bits, reversed serial bit order,
  swapped Arduino DATA/CLK/STB wires or a changed command protocol.
- Keep the IC variant distinction separate from how a particular panel is wired.
  The map must record which driver output reaches each physical digit/bar/icon.
  Existing map-based design can express that relationship; no universal map is
  supplied by this explanation alone.
- This clarification concerns SM1628 and SM1628B. It does not establish TM1628
  or SM1628C equivalence, connector interchangeability or an oscillator pinout.
- Exact panel wiring/output map remains the next implementation input. No code
  or wiring was changed solely on the phrase "invert the whole code."



The project builds on Jose Luis Monteiro's experiments, not on an assumption that
the old examples are interchangeable libraries.

- [SM1628 video, 2019](https://www.youtube.com/watch?v=YcZyTewgw1s).
- [Original SM1628B code](https://github.com/emsyscode/SM1628B/blob/master/SM1628B_display_7Segments_DVD.cpp),
  blob a4e16c99b3107ec594a4b2fd392266de01b136b8.
- [SM1628B panel video, 2020](https://www.youtube.com/watch?v=RpfVqr31mYc).
- [Original six-digit code](https://github.com/emsyscode/SM1628B_display_6digits/blob/main/SM1628B_Panel_DVD_Reader_placa_com_DSP_maior_v4_youtube.ino),
  blob 80ed1edaeed22f4f69b1af71d771bdae7e4625f8.

Both English caption files were downloaded through the channel's signed-in
YouTube Studio on 2026-10-25 and reviewed alongside the code. Captions contain
transcription errors; technical claims still need part-specific confirmation.

The 2019 explanation deliberately keeps code basic and emphasizes reordered
grids, swapped grid/segment roles, wiring, buttons and binary pattern tables.
The 2020 explanation revisits the panel layout, notes an unused segment output,
warns about SM1628B versus TM1628 memory differences, and says the clock demo
was not carefully calibrated. Those observations guide this refactor's scope.

## Still required before a hardware release

- Exact board/IC identification and manufacturer datasheet comparison.
- Verified wiring photograph/diagram and measured seven-digit profile.
- Bench test of frame writes, brightness, readback if added and filming behavior.
- Key scanning as an independent, documented feature after verification.
- Snake hardware adapter, joystick thresholds and response measurements.
- Explicit author/license decision before public release.

Do not replace unknown board mappings with plausible defaults. Keep initial
implementation experimental until the engineer confirms it on the actual board.
