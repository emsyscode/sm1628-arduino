# 2. Run the first display test

Complete [the display-only wiring](01-connect.md) first. The two starting paths
below depend on whether your visible map is already known.

## Reference panel: automatic numbers

1. Open `firmware/PanelDemo/PanelDemo.ino` in Arduino IDE.
2. Select Arduino Uno and its port. Upload.
3. Expect numerals 0-9 on the leftmost numeric position, then 0-9 on each
   subsequent position from left to right. Each numeral lasts about 100 ms.
   Only one numeric position is addressed at a time; symbol groups stay off.
4. After the seventh position the display clears and the test repeats.

The physical map is `0, 1, 7, 2, 3, 4, 5`. The sketch preserves the source
numeral shapes, including its open-bottom 9. A different-looking 9 is not by
itself a broken segment: the manual all-bars test checks that separately.

## Unknown panel: inspect before adapting the map

1. Open `firmware/ManualMap/ManualMap.ino`, select Uno and upload.
2. Open Serial Monitor at **9600 baud**, **Newline** or **Both NL & CR**.
3. Wait for READY; the display starts blank.
4. Send exactly `column 0 7` and Enter. On our panel this lights a 7 at the
   leftmost numeric position. On yours, record what actually lights.
5. Send `n` and Enter to advance, or `p` and Enter to go back. The result stays
   lit while you inspect it. Repeat once per position.

`n` does not select a starting column. **First `column 0 7`, then `n`.** Reset
or `clear` cancels the selection, so send the first command again afterward.
Do not substitute `column 0 8` when reproducing the photographed test: 7 was
the pattern used to establish the recorded position map.

Continue to [mapping instructions](manual-mapping.md) for all bars, individual
icons and your own lookup table. If nothing lights, follow
[troubleshooting](troubleshooting.md) before changing several things at once.
