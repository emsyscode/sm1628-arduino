# Validation

## Version 0.2.0 — 2026-10-25

Uno / Arduino AVR core 1.8.6; example output enabled for the active builds below.

| Build | Flash | Static SRAM |
| --- | ---: | ---: |
| Standalone Minimal | 1,076 bytes | 28 bytes |
| BufferedCounter | 2,298 bytes | 248 bytes |
| ProtocolTest | 2,034 bytes | 29 bytes |
| PanelTest | 3,288 bytes | 267 bytes |

Local compiled-AVR simulation passed:
- 19 raw protocol transactions.
- 149 mapped-panel transactions, including all ten original numeral masks
  at each of the six source-derived digit positions.
- Standalone initialization and both 500 ms blink transitions.

The panel fixture also checks seven-digit capacity, overflow, padding,
preservation of an unrelated icon, invalid/aliased maps, copied map lifetime,
suppression of unchanged frames, forced resend, restoration after external
writes and bounded RAM writes. Reference masks/grid shifts are recorded with
the original source blob identity in tests/legacy-reference.json.

Only inherited Arduino core new.cpp unused-parameter warnings were emitted.
The local compiler required execution outside the Windows sandbox because of
its DLL relocation error. No firmware was uploaded.
Pin-range rejection was added after these size measurements; CI recompiles and
tests that final revision. Sizes above identify the local pre-pin-guard build.

## Reproduce

Install arduino-cli, Arduino AVR core 1.8.6 and Node.js.
The exact compile commands, including both guarded/active examples, are in
.github/workflows/validate.yml. Build the two fixtures to build/protocol and
build/panel, and the enabled Minimal example to build/minimal, then run:

    npm install --ignore-scripts --no-audit --no-fund
    npm run test:avr

Local runs reused avr8js 0.21.1 from the existing project via the decoder's
optional third argument (module path). CI installs the pinned dev dependency.

These tests decode the real compiled AVR GPIO stream. They are not an SM1628 IC,
electrical or optical simulation. The legacy profile is code-derived; the new
seven-digit panel map and hardware refresh/readability remain unverified.

## Earlier 0.1.0 baseline

ProtocolTest: 2,034 bytes flash / 29 bytes static SRAM. RawFrame and SegmentWalk
with their bench guard disabled: 1,540/1,538 bytes flash, respectively, and
188 bytes static SRAM each. Original protocol regression: 19 transactions.
