CURRENT ROUTING (2026-10-26): the exact former PanelDemo described in this historical baseline record is now firmware/LegacyPanelDemo. The new PanelDemo is a numeric-only physical-order test. See docs/symbol-bit-correction.md for the deliberate correction and new checks. The original refactor was subsequently reported working by the creator; the latest corrected numeric renderer still needs a hardware run.

# Validation of the new baseline

## Evidence levels

- Original: creator reports it works on their physical panel.
- Refactor: compiles and matches the original's captured transaction sequence.
- Refactor on hardware: **not yet retested by the creator**.

Neither compilation nor the AVR comparison certifies panel electrical behavior.

## Local results — Arduino Uno, AVR core 1.8.6

| Sketch | Flash | Static SRAM |
| --- | ---: | ---: |
| Untouched supplied reference | 3,084 bytes | 468 bytes |
| Clean PanelDemo | 2,522 bytes | 327 bytes |

Original source hash checked:
4E9521DC761201B3C6FEB347DB51230398487458F13151BA866038D02293B9B4

Both real .hex files were executed with avr8js 0.21.1. One complete cycle
matched at all 380 STB transactions: startup, initial clear, eighty numeral/slot
combinations and final clear. Every literal byte, bit order, boundary, source
blanking command, full-brightness command, 100 ms hold and port direction state
was checked. Expected patterns are parsed from the untouched reference, not from
the new sketch.

Simulated durations: original 8223.250 ms; refactor 8223.024 ms.
Maximum difference between corresponding packet first-clock-to-STB-end durations:
8.813 us. Refactoring changes CPU instruction timing slightly even while the
explicit delays are retained. This comparison does not claim cycle-exact
electrical waveform identity.

The original emits F_CPU redefinition and unused-variable warnings; it is left
untouched. The clean sketch emitted only inherited Arduino core new.cpp warnings.
No Arduino upload was performed. The Windows sandbox could not launch avr-g++;
compilation succeeded outside the sandbox with user approval.

## Reproduce

Install Arduino CLI, Arduino AVR core 1.8.6 and Node.js.

    arduino-cli compile --fqbn arduino:avr:uno --build-path build/reference reference/WorkingPanel/SM1628B_Panel_DVD_Reader_7digits_sanake_v0
    arduino-cli compile --fqbn arduino:avr:uno --build-path build/refactor firmware/LegacyPanelDemo
    npm install --ignore-scripts --no-audit --no-fund
    npm test

The local comparison reused an existing avr8js installation via the optional
fourth CLI argument (module directory). CI installs the pinned dependency.
The active workflow builds both sketches and performs this comparison.

## Smallest useful hardware check

Use the same Uno, wiring and power arrangement on which the original worked.
Upload the original, observe a full cycle, then upload PanelDemo and compare
the same visible behavior. Record which physical digit/icon corresponds to each
source slot. Stop further driver cleanup until that comparison succeeds.

No game logic, key reader, joystick or new pin assignment is introduced in this
retest. The original's whole-port direction setup must be resolved before adding
those inputs.

