# SM1628 panel — working-source baseline

Based on Jose Luis Monteiro's original sketch, preserved byte-for-byte under
reference/WorkingPanel. Earlier generated examples failed on this salvaged
panel and remain archived. The creator confirms the clean PanelDemo now works.

## Map the panel manually

Open [firmware/ManualMap/ManualMap.ino](firmware/ManualMap/ManualMap.ino)
in Arduino IDE, select Arduino Uno and upload with the tested wiring.
Open Serial Monitor at **9600 baud**, with **Newline** or **Both NL & CR**.

FIRST send `column 0 7`. Expect 7 on the leftmost digit. THEN send `n` when ready. Each selection stays lit until
another command. Reset/clear cancels the selection; n/p needs a new selection first. Send `ordered` to start a left-to-right numeric sequence with 7, then n/p to step (wrapping at either end). Use `slot 0 7` to compare the original pattern, or `bit 0 0`
to isolate a single RAM bit. [All commands and recording instructions](docs/manual-mapping.md).

Arduino D7 = DATA, D8 = CLK, D9 = STB; these are GPIO assignments, not connector pins.
The inherited setup owns D1..D13 as outputs; ManualMap restores D0 to input
for serial reception. Do not add joystick/peripheral connections to those pins.
Keep the existing tested supply arrangement.

## Other tests and references

- [PanelDemo](firmware/PanelDemo/PanelDemo.ino): unchanged repeating original test.
- [SlotMap](firmware/SlotMap/SlotMap.ino): earlier timed, one-pass diagnostic.
- [Engineer panel map](docs/panel-map.md): numeric digits left to right are
  **Grid 6, 8, 7, 5, 4, 3, 2**, plus three symbol groups. Raw numeric columns left to right are **0,1,7,2,3,4,5**.
- [Original Portuguese explanation](docs/engineer-panel-explanation.pt.md).
- [Datasheet sources and revisions](docs/datasheets.md).
- [Baseline details](docs/baseline.md), [walkthrough](docs/walkthrough.md),
  [validation](docs/validation.md), [earlier code archive](archive/README.md).

Raw RAM columns and source slots are separate indices from the engineer's
Grid labels. The numeric map is established by the photographed column-7-pattern test; remaining bars/icons need separate testing.
ManualMap compiles and passes simulated UART/GPIO checks; its hardware test
is pending. It does not modify the accepted PanelDemo.

This is display-test firmware, not yet the Snake game. The computer prototype,
joystick animations and video script remain in the existing project repos.
No public release or new distribution license is claimed by this private repo.

