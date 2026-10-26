# SM1628 panel — working-source baseline

Based on Jose Luis Monteiro's original sketch, preserved byte-for-byte under
reference/WorkingPanel. Earlier generated examples failed on this salvaged
panel and remain archived. The creator confirms the clean PanelDemo now works.

## Map the panel manually

Open [firmware/ManualMap/ManualMap.ino](firmware/ManualMap/ManualMap.ino)
in Arduino IDE, select Arduino Uno and upload with the tested wiring.
Open Serial Monitor at **9600 baud**, with **Newline** or **Both NL & CR**.

Send `column 0 8`, then `next` when ready. Each selection stays lit until
another command. Use `slot 0 8` to compare the original pattern, or `bit 0 0`
to isolate a single RAM bit. [All commands and recording instructions](docs/manual-mapping.md).

Arduino D7 = DATA, D8 = CLK, D9 = STB; these are GPIO assignments, not connector pins.
The inherited setup owns D1..D13 as outputs; ManualMap restores D0 to input
for serial reception. Do not add joystick/peripheral connections to those pins.
Keep the existing tested supply arrangement.

## Other tests and references

- [PanelDemo](firmware/PanelDemo/PanelDemo.ino): unchanged repeating original test.
- [SlotMap](firmware/SlotMap/SlotMap.ino): earlier timed, one-pass diagnostic.
- [Engineer panel map](docs/panel-map.md): numeric digits left to right are
  **Grid 6, 8, 7, 5, 4, 3, 2**, plus three symbol groups.
- [Original Portuguese explanation](docs/engineer-panel-explanation.pt.md).
- [Datasheet sources and revisions](docs/datasheets.md).
- [Baseline details](docs/baseline.md), [walkthrough](docs/walkthrough.md),
  [validation](docs/validation.md), [earlier code archive](archive/README.md).

Raw RAM columns and source slots are separate indices from the engineer's
Grid labels. Their correlation still needs recording on the actual panel.
ManualMap compiles and passes simulated UART/GPIO checks; its hardware test
is pending. It does not modify the accepted PanelDemo.

This is display-test firmware, not yet the Snake game. The computer prototype,
joystick animations and video script remain in the existing project repos.
No public release or new distribution license is claimed by this private repo.
