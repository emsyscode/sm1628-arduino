# Sources and development status

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
