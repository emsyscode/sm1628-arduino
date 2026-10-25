# Validation — 2026-10-25

Initial local checks passed using Arduino AVR core 1.8.6 and Uno.

| Build | Flash | Static SRAM |
| --- | ---: | ---: |
| ProtocolTest | 2,034 bytes | 29 bytes |
| RawFrame (bench guard disabled) | 1,540 bytes | 188 bytes |
| SegmentWalk (bench guard disabled) | 1,538 bytes | 188 bytes |

The protocol fixture exercises the real library. avr8js 0.21.1 executed the
compiled Uno firmware and decoded D7/D8/D9: 19 transactions passed in
16.532 ms of simulated CPU time.

Checks cover bit order, byte boundaries, initial blanking/clear, full frame
payload, brightness/on-off behavior, invalid inputs, non-contiguous segment
mapping, atomic map rejection and preservation of unrelated outputs.
Fixture D5 stays low on success and D6 signals completion.

Only inherited Arduino core new.cpp unused-parameter warnings were emitted.
The installed AVR compiler required running outside the Windows sandbox due
to its DLL relocation error. No firmware was uploaded.

To reproduce from this repository:
1. Install arduino-cli and Arduino AVR core 1.8.6.
2. Compile: arduino-cli compile --fqbn arduino:avr:uno --library . --build-path build/protocol tests/ProtocolTest
3. Install dev dependencies: npm install --ignore-scripts --no-audit --no-fund
4. Run: npm run test:avr

The local run reused the existing avr8js installation via the decoder's optional
third argument (module path). CI uses the pinned package.json dependency.

This is a GPIO/software test, not an electrical, optical or SM1628 IC simulation.
Bench wiring, exact revision commands, refresh/readability, a seven-digit map
and a playable hardware game remain unverified.
