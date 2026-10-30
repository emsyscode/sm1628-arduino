# Read the code

Start with **PanelDemo**. Its settings choose the Uno pins, working display mode,
brightness, numeral masks and physical digit lookup. `sendCommand` transmits one
command; `sendBurstByte` transmits bytes while the caller holds STB low.
`initializePanel` sets the working mode; `clearPanel` clears RAM.
`showTestNumeral` builds a clean numeric frame. `loop` selects physical positions
and numerals. It is the small display example, without game/input machinery.

**ManualMap** adds a 14-byte buffer, held selections and a bounded line parser.
Its normal renderers construct clean columns; `OriginalPatterns.h` contains
only the optional mixed-payload compatibility diagnostic. Serial input is
restored on D0 after whole-port initialization.

**PanelShowcase** builds on the same display transport and map to construct
frames for counting, carry, sweeps, fills and other effects. Start with those
renderers to see how an application changes a buffer without changing the
driver protocol. Its serial commands select the effect and its update interval.

These display examples use the same command path for SM1628 and SM1628B.
Changing to a differently wired panel calls for a different frame/map, not a
different driver-name string. See the [variant guide](driver-variants.md).

## Optional game and input code

**JoystickTest** reads A0/A1 and prints raw ADC and estimated voltage values.
The 500 ms delay is for readable bench output. It is not the game's input interval.
The nominal 5 V reference and printed decimals are not a precision measurement.

**PanelSnake** separates display transport, frame construction, joystick
calibration/hysteresis, button debounce, timed buzzer pulses, serial commands
and the main phase machine. `SnakeGame.h` contains the 49-cell game rules;
hardware output is separate from those rules. `loop` advances input, game and
display on independent clocks. Configuration is near the top; read the
[game guide](snake-test.md) before changing axis signs or thresholds.

PanelShowcase also includes a game demonstration. `SnakeReplay.h` supplies
a fixed legal route and food positions using the same `SnakeGame.h`; it is a
scripted demonstration, not an autonomous joystick player. The duplicated game
header keeps both Arduino sketch folders self-contained; tests check equality.

## Maintenance details

The sketches configure the Uno ports directly and enable Timer1. The interrupt
updates `legacySeconds` and `legacyFlag`; the game uses `millis()` for scheduling.
Changes to the sender, clock or port setup need a panel test as well as a build
check, since compiling cannot show whether the LEDs refresh correctly.

Original source is retained under `reference/` for attribution and regression.
The exact legacy comparator lives under `tests/fixtures/`, not among tutorial
examples. They are not the starting point for new users.
