# Circuit parts and confirmed connections

The display-only wiring is complete in [Connect the panel](01-connect.md).
This list records additional Snake parts; it is not a complete control schematic.

| Part | Quantity | Value / use |
|---|---:|---|
| Uno | 1 | ATmega328P, 16 MHz |
| Assembled SM1628B panel | 1 | Seven digits plus three symbol groups |
| Industrial potentiometer joystick | 1 | X/Y used, rotation/Z unused |
| Resistors | 3 | 1 kΩ in DATA, CLK, STB |
| Resistors | 4 | 1 kΩ at four X/Y track ends |
| Resistor | 1 | 10 kΩ transistor base resistor |
| Resistor | 1 | 680Ω in START-to-ground path |
| Normally open START button | 1 | D4 input, internal pull-up |
| Capacitor | 1 | 10 nF across START switch |
| Capacitor | 1 | 470 µF, 35 V rating |
| Buzzer driver transistor | 1 | 2N2222-family stage |
| Active buzzer | 1 | DC on/off; D11 controls transistor stage |

Total: nine discrete resistors (seven 1 kΩ, one 10 kΩ, one 680Ω). Internal joystick
tracks are not additional discrete resistors. The capacitor's35 V marking is
its rating, not the project's supply voltage.

Fixed potentiometer ends are track ends/outer terminals; the moving contact is
the wiper. Identify the actual joystick terminals. A0=X, A1=Y are firmware
assignments, not a connector pinout. Reported internal resistance excludes the
external1 kΩ resistors.

The reported START network is D4 -> normally open switch ->680Ω ->GND, with 10 nF
across the switch. The capacitor is not automatically across D4/GND: the resistor
is after the switch. D4 uses INPUT_PULLUP; 680Ω is not an external pull-down.
Software also requires 30 ms of stable button input.

## Complete control-stage drawing

Before copying the full game circuit, these connections still need confirmation:

- the supply rail at each joystick track-end resistor;
- actual transistor package pinout, emitter/collector nodes and buzzer supply;
- both nodes and polarity of the470 µF capacitor.

A parts list/obscured wires cannot establish those nodes. D11 is base-stage
control through 10 kΩ, not buzzer power. The panel photos establish display wiring,
not these control-stage connections. Keep them explicit rather than inventing
a universal schematic.
