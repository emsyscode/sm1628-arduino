# SM1628 and SM1628B — Arduino driver tutorial

Connect an LED panel to an Arduino Uno, understand the driver's commands and
display memory, find your panel's segment map, and control numbers and symbols.
This is José Luis Monteiro's updated driver tutorial, developed and tested
together with TidusM.

**SM1628 or SM1628B? Both use the programming approach taught here.** The same
three-wire interface, display commands and 14-byte RAM layout apply to these
examples. There is no separate B-version sketch or variant selector. The main
hardware differences are the oscillator connection and the B variant's improved
protection; see the [short comparison](docs/driver-variants.md).

![Both drivers use the same interface; the panel determines the display map](docs/images/driver-and-panel.svg)

**Same driver does not mean same panel wiring.** Identify your connector and
map your display before copying the reference panel's lookup table. Changing
the IC suffix alone does not call for a new map; changing the panel can.

## Learn the driver

| Step | What you will learn | What you will achieve |
|---|---|---|
| [SM1628 and SM1628B](docs/driver-variants.md) | Shared programming and hardware differences | Know what depends on the variant |
| [1. Identify and connect the panel](docs/01-connect.md) | Supply, ground, DATA, CLK and STB | Connect the panel to an Uno |
| [2. Run the first display test](docs/02-first-display.md) | Upload a small example and check the result | Count through numeric positions |
| [3. Map your own panel](docs/manual-mapping.md) | Select columns and individual bits | Record your digit, segment and symbol map |
| [4. Understand memory and commands](docs/04-memory.md) | Native versus swapped wiring, frame construction and transmission | Choose the bytes you send |
| [5. Control numbers and effects](docs/filming-showcase.md) | Counting, carry, patterns and timing | Try repeatable display demonstrations |

The [reference panel map](docs/panel-map.md) applies these ideas to our assembled
DVD panel: seven numeric digits and three symbol groups. It is one worked
example, not a universal panel configuration.

## What you need

An **Arduino Uno / ATmega328P at 16 MHz**, a suitable SM1628 or SM1628B panel,
an appropriate panel supply, and the three 1 kΩ signal resistors used in our
connection example. No external Arduino library is required. Node.js is only
for automated developer tests.

Download the repository ZIP and extract it. Open the `.ino` inside the chosen
sketch folder, keeping its `.h` files alongside it. Select **Arduino Uno** and
the correct port, then upload. A new sketch replaces the previous program.

| Sketch | Purpose | Serial Monitor |
|---|---|---|
| [PanelDemo](firmware/PanelDemo) | Small numeric example; start here for the reference panel | Not required |
| [ManualMap](firmware/ManualMap) | Inspect and map an unfamiliar panel | 9600 baud; Newline or Both |
| [PanelShowcase](firmware/PanelShowcase) | Select numbers, counting, carry and effects | 115200 baud; Newline or Both |

The selected Uno pins are ordinary GPIO: this interface does not require the
hardware-SPI pins. These examples write display RAM; the IC's key-scan read
function and infrared decoding are not covered. The panel's IR pin is unused.

**Pin ownership:** the working sketches retain whole-port initialization.
PanelDemo drives D0–D13 as outputs; ManualMap restores D0 as serial input.
PanelShowcase also restores D4 as an input. Read the connection guide before
attaching other peripherals. Other Arduino architectures need adaptation.

## Optional application: Snake

Once you can control the display, [build the Snake example](docs/snake-test.md)
to combine the driver with a joystick, START button and active buzzer. The
[joystick test](firmware/JoystickTest) and [game sketch](firmware/PanelSnake)
are separate from the display-only learning path. PanelShowcase also includes
a repeatable scripted game demonstration.

## Further help

[Troubleshooting](docs/troubleshooting.md) · [Code guide](docs/code-guide.md)
· [Datasheets](docs/datasheets.md) · [Optional game circuit](docs/circuit-inventory.md)
· [Validation](docs/validation.md) · [Authors and references](CREDITS.md)
