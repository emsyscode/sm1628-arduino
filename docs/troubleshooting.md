# Troubleshooting

| Symptom | Check next |
|---|---|
| No display | Verify actual panel supply and common ground, board/port and correct sketch. Check DATA/CLK/STB labels rather than connector order. ManualMap starts blank until a command. |
| Serial text is unreadable | ManualMap uses 9600; game, joystick and showcase use 115200. |
| `n` has no sequence or is rejected | In ManualMap first send `column 0 7` plus Enter; then `n` plus Enter. Reset/clear cancels the sequence. In PanelSnake use `m`, then `n`. They are different programs. |
| Command appears to do nothing | ManualMap and Showcase need Newline/Both. A complete `pace 300` line needs Enter; game single-letter controls do not. Opening Serial Monitor can reset the Uno: wait for READY again. |
| Numbers light in the wrong order | Record raw column results and update `kDigitColumns`. Same driver does not establish same wiring. |
| Number shapes are scrambled | Verify individual segment rows with bit tests. A digit-order lookup cannot fix a different segment order or native/transposed arrangement. |
| Symbols light with a numeric digit | Inspect the 14-byte frame for extra column bits. Use clean column/ordered commands, not mixed legacy patterns. [Explanation](symbol-bit-correction.md). |
| One digit has an open-bottom 9 | That is the preserved numeral mask. Use `digit 8`/individual-bar test to distinguish the font from an unlit segment. |
| Joystick directions are wrong | Centre during calibration, send `j`, move one axis at a time, inspect readings, then adjust swap/reverse settings. |
| Snake moves at the wrong speed | `pace 450` plus Enter restores the default 450 ms per move; lower is faster. |
| START seems ignored | Wait for calibration. Release a button held at boot; make a new press. During play the button does not reset the round. |
| No sound | D11 controls a transistor stage, not a passive speaker directly. Use an active buzzer and verify that circuit and polarity. Showcase intentionally keeps D11 silent. |

For a useful issue report include the IC marking, connector labels, supply,
Arduino model, exact sketch/command, Serial output and a clear photo. Describe
what actually lit; do not assume a raw column is a physical digit number.
