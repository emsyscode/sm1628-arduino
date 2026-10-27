# Optional game circuit inventory

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
| Resistor | 1 | 680 Ω in START-to-ground path |
| Normally open START button | 1 | D4 input, internal pull-up |
| Capacitor | 1 | 10 nF across START switch |
| Capacitor | 1 | 470 µF, 35 V rating |
| Buzzer driver transistor | 1 | 2N2222-family stage |
| Active buzzer | 1 | DC on/off; D11 controls transistor stage |

Total: nine discrete resistors (seven 1 kΩ, one 10 kΩ, one 680 Ω). Internal joystick
tracks are not additional discrete resistors. The capacitor's 35 V marking is
its rating, not the project's supply voltage.

Fixed potentiometer ends are track ends/outer terminals; the moving contact is
the wiper. Identify the actual joystick terminals. A0=X, A1=Y are firmware
assignments, not a connector pinout. The measured internal resistance excludes the
external 1 kΩ resistors.

The START network is D4 -> normally open switch -> 680 Ω -> GND, with 10 nF
across the switch. The capacitor is not automatically across D4/GND: the resistor
is after the switch. D4 uses INPUT_PULLUP; 680 Ω is not an external pull-down.
Software also requires 30 ms of stable button input.

## Scope of this inventory

This inventory does not specify the joystick track-end rail assignments, the
transistor emitter/collector and buzzer supply wiring, or the two nodes and
polarity of the 470 µF capacitor. It cannot be used as a complete game-circuit
wiring guide. The display-only connections are covered separately.

D11 controls the transistor base stage through 10 kΩ; it does not power the
buzzer directly. Match the pinout and electrical requirements of your actual
joystick, transistor and buzzer before connecting those parts.
