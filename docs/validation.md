# Validation and reproducing checks

The reference panel and joystick-controlled game have a working hardware
demonstration. That establishes this build's operation, not compatibility with
every panel or an optical measurement of every feature. Individual icon bits
and the full control-stage drawing have separate documented limits.

Tests compile the actual C++ for Uno and execute the AVR code with simulated
GPIO/UART/ADC. They check frames and behavior, not electrical/thermal operation
or camera readability.

Install Arduino CLI, AVR core 1.8.6, Node.js22 and the pinned test dependency:

```text
arduino-cli core install arduino:avr@1.8.6
npm ci --ignore-scripts --no-audit --no-fund
```

Node is not needed to upload sketches. Complete build/test commands are in
[the CI workflow](../.github/workflows/validate.yml), including the original,
comparison fixture, five tutorial sketches and engine check sketch.

- Original-source hash remains unchanged; its comparator preserves all 380
  original transactions, sender timing and setup.
- PanelDemo checks70 numeral/position combinations, physical order, zero
  unintended icon bits and100 ms application holds.
- ManualMap checks start-before-next, clean/compatibility rendering, raw,
  ordered and individual-bit commands, wrapping and invalid-line behavior.
- PanelSnake checks calibration, direction interpretation, 49-bar mapping,
  growth/collision/turn queue, death/end screens, D4 debounce/restart, D11 DC
  pulse timing, SCORE/value and live pace validation.
- Showcase checks effects/cancellation/repeat/silent D11; replay checks the fixed
  legal route and end sequence. Both game headers must remain identical.

Regression fixtures under reference/ and tests/fixtures/ support maintainers;
they are not extra tutorial programs. Changes to transport, port ownership,
power wiring or a panel map also need an appropriate hardware comparison.
