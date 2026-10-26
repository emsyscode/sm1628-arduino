# SM1628 panel — working-source baseline

This repository has been reset around the **new sketch reported working by the
creator on the actual salvaged panel**. Earlier generated driver examples failed
on that hardware and are retired from the active project.

## Start here

Open [firmware/PanelDemo/PanelDemo.ino](firmware/PanelDemo/PanelDemo.ino) in Arduino
IDE, select **Arduino Uno**, and compile/upload using the existing tested wiring.
It runs the original repeating numeral test immediately: there is no enable flag.

- Arduino D7: DATA; D8: CLK; D9: STB.
- These are Arduino GPIO assignments, not panel connector pin numbers.
- The retained setup also configures all D0..D13 as outputs. It is Uno-specific.
  Do not attach a joystick or other peripherals to those pins yet.
- Keep the power/wiring arrangement already used for the working original.

The clean sketch preserves the active sender timings, startup, full brightness,
fourteen-byte patterns, source slot order, per-frame blanking and clear sequence.
It removes the unused clock application, key reader and repeated numeral switch
branches. The original Timer1/register setup is retained and explained.

**The original was reported working by the creator. The refactor still needs
their hardware retest.** Compile/AVR comparisons do not substitute for that test.

## Identify the actual output order

Use [firmware/SlotMap/SlotMap.ino](firmware/SlotMap/SlotMap.ino) for one labelled,
non-repeating mapping pass. It shows the source 8 pattern for about three seconds
per slot and prints SLOT 0..7 at 9600 baud. Record numeric digits and symbols
separately. [Instructions and recording table](docs/slot-mapping.md).

## Reference and explanation

- [Untouched supplied sketch](reference/WorkingPanel/SM1628B_Panel_DVD_Reader_7digits_sanake_v0/SM1628B_Panel_DVD_Reader_7digits_sanake_v0.ino).
- [What is retained and why](docs/baseline.md).
- [How the test works](docs/walkthrough.md).
- [Validation and next bench comparison](docs/validation.md).
- [Why earlier code was retired](archive/README.md).

This baseline is a display test, **not Snake firmware**, despite the supplied
filename. The reported numeric order is 3,1,2,4,5,6,7 from the left; it is not yet a verified slot-to-digit map. See [panel observations](docs/panel-observations.md).
The Snake computer prototype, joystick animations and script remain in the
existing project/production repositories.

Based on Jose Luis Monteiro's work. No public release or new distribution license
is claimed by this private development repository.
