# Circuit inventory and component filming checklist

Updated: 2026-10-26. Source: creator's circuit inventory supplied in chat.
Values/locations below preserve that report; this is not a complete schematic.
Reference labels are assigned here for documentation and filming, not read from
a PCB silkscreen. Module internals are not itemised.

## Resistors

| Label | Quantity | Value | Reported location |
|---|---:|---|---|
| R1 | 1 | 1 kΩ | Display DATA line |
| R2 | 1 | 1 kΩ | Display CLK line |
| R3 | 1 | 1 kΩ | Display STB line |
| R4 | 1 | 1 kΩ | X potentiometer track end associated with left movement |
| R5 | 1 | 1 kΩ | X potentiometer track end associated with right movement |
| R6 | 1 | 1 kΩ | Y potentiometer track end associated with up movement |
| R7 | 1 | 1 kΩ | Y potentiometer track end associated with down movement |
| R8 | 1 | 10 kΩ | Base resistor for the 2N2222 buzzer driver |
| R9 | 1 | 680 Ω | Connection between START push-button and GND |

**Total: nine resistors — seven 1 kΩ, one 10 kΩ and one 680 Ω.**
The joystick's internal resistive tracks are not additional discrete resistors
in this count. Exact rail assignments at their ends remain a schematic detail;
do not label all four ends as +5V solely from this parts list.

## Capacitors and buzzer driver

| Label | Quantity | Part/value | Reported location or role |
|---|---:|---|---|
| C1 | 1 | 10 nF capacitor | In parallel with the START push-button |
| C2 | 1 | 470 µF, 35 V capacitor | Connection location to be recorded |
| Q1 | 1 | 2N2222 transistor | Active buzzer driver |

The 35 V marking is the capacitor's voltage rating, not a statement that the
project runs at 35 V. Capacitor construction/type, resistor power/tolerance and
transistor package/pin order have not been specified in this inventory.

## Main assemblies and controls

| Quantity | Assembly/component | Existing project identification |
|---:|---|---|
| 1 | Arduino Uno | Controller |
| 1 | Salvaged SM1628B display panel | Seven numeric positions plus three symbol groups |
| 1 | Potentiometer joystick | X/Y signals used; Z unused |
| 1 | START push-button | Momentary control |
| 1 | Active buzzer | Switched through Q1; D11 is the control signal |

Breadboard, jumpers and supply/USB connections are part of the bench setup;
their counts and a final connection drawing are not supplied by this list.

Current firmware assignments:
DATA D7, CLK D8, STB D9, X A0, Y A1, START D4, buzzer control D11.
The software enables the internal pull-up on D4. R9 is recorded in the
button-to-ground connection; do not silently turn it into a pull-down resistor.
C1 is recorded across the switch, not assumed to be across D4 and GND without
first establishing where R9 sits in that path.
Do not infer a complete RC/debounce circuit or claim a measured filtering effect.

## Terminology for the video

- The two fixed potentiometer connections are the **track ends** or **outer
  terminals**. The moving connection is the **wiper**.
- Name the control **START push-button** or **momentary push-button**.
- Name R8 the **transistor base resistor**, and Q1 the **buzzer driver transistor**.

## Completing the connection drawing

The remaining connection details are the precise R9/C1/switch nodes, C2 location,
joystick end-to-rail assignments and Q1 terminal connections. Keep those explicit
when preparing the final wiring diagram; this inventory alone is not a netlist.
