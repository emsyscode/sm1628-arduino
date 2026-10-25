# SM1628 Arduino

A cleaner development base for reusing salvaged seven-segment display panels,
starting with SM1628B and a Snake project.

**Status: experimental, not yet bench tested.** The initial transport follows
the original SM1628B examples' mode 0x03 and 14-byte RAM write. The exact IC,
electrical setup and segment map of the new seven-digit board remain pending.
TM1628, SM1628C and every board bearing a similar marking are not automatically
compatible.

## Start here

1. [Identify the board and bring it up](docs/01-bring-up.md).
2. [Discover and record its segment map](docs/02-mapping.md).
3. Try SegmentWalk, then RawFrame from the Arduino examples menu.
4. [Read the protocol and implementation notes](docs/protocol.md).
5. [Review sources and remaining work](docs/sources.md).

Install this folder as an Arduino library (or use arduino-cli --library with
its absolute path). The examples keep hardware output disabled until their
kBenchReady flag is deliberately changed after checking the setup.

## Structure

- src/: transport, a small raw frame buffer and explicit glyph mapping.
- examples/: small standalone steps, separate from the game.
- profiles/: board map template; no fabricated working map.
- tests/: compiled AVR protocol/mapping fixture and GPIO decoder.
- docs/: tutorials, evidence, validation and work still required.

No timer is reserved and no entire AVR port is reconfigured. The library changes
only its three assigned GPIOs and contains no clock demo, key scanner or game loop.
An entire frame is sent in one address/data transaction. This removes the old
application's long reporting/demo delays; hardware timing still needs measurement.

The Snake browser prototype and video-production assets remain in their existing
projects for now. Hardware Snake integration follows a verified panel map.

Based on work by [Jose Luis Monteiro / emsyscode](https://github.com/emsyscode).
See [source history](docs/sources.md). No new distribution license has been chosen
yet; settle that with the authors before public release.
