# SM1628 Arduino

Clean SM1628B display code based on Jose Luis Monteiro's salvaged-panel examples.

**Experimental: software tested, hardware validation pending.** Mode 0x03 and
fourteen-byte writes follow the original SM1628B examples. The new seven-digit
board's exact IC, wiring and map remain unconfirmed. Similar names such as TM1628
or SM1628C do not establish compatibility.

## Choose a starting point

| Version | Open | Purpose |
| --- | --- | --- |
| Small standalone sketch | [Minimal.ino](examples/Minimal/Minimal.ino) | Understand initialization and frame writes in one short file; no library dependency |
| Fuller library | [MappedPanel.h](src/MappedPanel.h) | Mapped digits/numbers, individual segments/icons, buffered updates and input validation |
| Raw driver | [Sm1628B.h](src/Sm1628B.h) | Full frames, bounded RAM writes, brightness and display control |

The fuller version adds functionality alongside the small version; the teaching
sketch stays small. [What changed from the originals](docs/code-review.md).

## Follow the tutorials

1. [Identify the board and bring it up](docs/01-bring-up.md).
2. [Walk outputs and record the panel map](docs/02-mapping.md).
3. [Understand the small sketch](docs/minimal.md).
4. [Use the buffered API](docs/api.md) and [BufferedCounter](examples/BufferedCounter/BufferedCounter.ino).
5. [Read the inherited protocol](docs/protocol.md) and [validation evidence](docs/validation.md).

For library examples, install this folder as an Arduino library or pass
arduino-cli --library with its absolute path. Minimal.ino is standalone.
Examples keep hardware output disabled until the bench setup is confirmed.
Minimal and BufferedCounter use SM1628_BENCH_READY=1; earlier RawFrame and
SegmentWalk examples use their local kBenchReady flag.

The included six-digit map is CODE_DERIVED from the 2020 source; it is not a map
of the new seven-digit board. A blank [profile template](profiles/template.json)
is provided for that board's measured map.

No timer reservation, heap allocation or whole-port GPIO writes are used.
Rendering is separate from sending, so the application can read its joystick
between updates. No measured joystick/display latency is claimed.

Hardware Snake integration follows the panel map. Video scripts and animations
remain in the production project. Key-read support awaits exact-part review;
the original key code has a memory indexing bug and inconsistent read lengths.

Based on [Jose Luis Monteiro / emsyscode](https://github.com/emsyscode).
[Source history and remaining work](docs/sources.md).
No distribution license has been selected yet; agree it with the authors before
public release.
