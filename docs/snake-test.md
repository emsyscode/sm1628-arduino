# PanelSnake — first hardware Snake test

This is an experimental, self-contained Arduino Uno game for **our existing
SM1628B DVD panel**. The driver transport comes from Jose Luis Monteiro's working
panel sketch. Game junction/segment rules follow the seven-segment browser
prototype. The original display tests remain separate and unchanged.

The creator reports the original joystick-controlled game works correctly on the panel. This update adds a START button and removes joystick auto-start; the new button wiring/behavior still needs its hardware test.

## Wiring and upload

| Signal | Uno pin |
|---|---|
| Panel DATA | D7 |
| Panel CLK | D8 |
| Panel STB | D9 |
| Joystick X signal | A0 (initial assignment to verify) |
| Joystick Y signal | A1 (initial assignment to verify) |
| Joystick Z | Unused |
| Momentary START button | D4 to GND; internal pull-up |
| Buzzer control | D11 |

Retain the **already tested** display supply, joystick resistor network and
common ground. This is not a replacement wiring/power schematic. A0/A1 are the
two pins in the supplied joystick test; which one is physically X must be
checked with direction reports. Do not connect signals outside the Uno's input
range. The inherited compatibility setup still drives D1..D13, except D4, as outputs;
Reserve D4 for START and D11 for the buzzer; keep other peripherals off that output bank.

Open firmware/PanelSnake/PanelSnake.ino in Arduino IDE with SnakeGame.h beside
it, select Arduino Uno, then upload. No external display/game library is needed.
Open Serial Monitor at **115200 baud**, Newline or Both. Opening the monitor may
reset the board.

## First bench run — commands and expected results

1. **Keep the joystick centred during startup.** For approximately one second
   the display is blank while the sketch measures its actual resting values.
   Expect CENTRE X=... Y=..., followed by READY. Do not push during calibration.
2. Send **m**. Expect only the **top bar of the leftmost numeric digit** to light.
   The log says BAR digit=1 ... segment=a ... raw column=0.
   Send **n** for each next bar: a top, b upper-right, c lower-right, d bottom,
   e lower-left, f upper-left, g middle, then the next digit from the left.
   After all 49 bars it wraps. This tests the four bars the numeral7 photos did
   not cover. If a bar is wrong, record its logged digit/segment and a photo.
3. While still in this bar-test mode, send **j** to enable direction reports.
   Push left, right, up and down. Expect the matching label; release should
   report CENTRE. This mode lets you test the joystick without starting play.
   Send j again to stop reports.
4. Press the **START button** (or send **s**) to start Snake. Three top bars form the initial snake; blinking
   food appears on a free bar. The snake initially heads right.
5. Steer before the end. Moving back into the opposite direction is ignored.
   Centring the joystick does not stop the snake. The initial movement period
   is **450ms per step**, independent of joystick displacement.
6. Eating food grows the snake and increments score. A wall/missing path or
   self-collision freezes and blinks the body for about1.6 seconds. The display
   then alternates approximate GAME, OVER and numeric score. Serial prints the
   unambiguous GAME OVER and score. Seven-segment M/V/R are approximations.
7. After game over, release and press START for another round. Serial s also works. Joystick movement no longer starts/restarts the game. Holding the button does not restart it repeatedly.

The game treats each illuminated bar as one cell (49 total), and each endpoint
as a junction. Horizontal gaps between digits are crossed without becoming body
or food cells. Turns must follow a real segment; the layout is deliberately
restrictive, just as in the browser prototype. Filling all 49 cells ends the round
and reports BOARD COMPLETE before the end sequence.

## START button

Use a normally open momentary button between D4 and GND. INPUT_PULLUP holds the input HIGH when released; a press connects it to GND (LOW). No extra pull-up resistor is required for this input. D4 is explicitly restored as an input after the inherited whole-port setup. On a four-leg tactile button, use contacts that are open when released and joined only when pressed.

The code requires 30ms of stable input and one new press. Holding at startup requires release before pressing to start. A press during play does not reset the round; after GAME OVER, a fresh press starts again. The button can also leave the bar test and start play. Calibration must finish first. Serial s remains available.

## Quick controls

| Input | Meaning |
|---|---|
| s | Start/restart after calibration, or leave the bar test and play |
| c | Blank and recalibrate; centre the stick for one second |
| j | Toggle raw X/Y, centre offsets and interpreted direction reports |
| m, then n | Start isolated-bar check, then step one bar at a time |
| w / a / d / z | Serial up / left / right / down; s is reserved for start |
| ? | Print command help |

These are **single-character commands**, not ManualMap's column commands.
ManualMap remains available for column 0 7 then n diagnostics.

## Adjusting the joystick

The supplied test reads A0/A1 at 115200 baud and waits 500 ms between reports.
PanelSnake reuses those inputs but polls on a separate 5 ms schedule, with four
samples per axis and a discarded first sample after switching ADC channels.
These are software settings, not a measured end-to-end latency guarantee.

Default signs follow the supplied measurements: left and up increase voltage;
right and down decrease it. Initial axis settings are at the top of the sketch:
kSwapAxes, kReverseX and kReverseY. Change only after comparing j reports with
actual stick movement. Re-upload and recalibrate after changing them.

kEngage=30 and kRelease=18 are candidate ADC-count thresholds, introducing
hysteresis around the measured centre. At a nominal 5 V reference they correspond
to roughly 0.147 V and 0.088 V. This is not the earlier unverified 10 mV claim.
Positive/negative travel spans based on the supplied readings balance diagonal
axis selection; only one direction is queued per movement tick. These thresholds
and axis assignment still need your hardware test. Use c if rest has moved.

## Display mapping and differences from the reference

Numeric columns, left to right: **0,1,7,2,3,4,5**. This position map was photographed
with numeral7. Rows a..g use the working sketch's pair order; the full individual
bar assignment is not yet bench-verified. The renderer writes all 14 bytes, leaving
symbol columns 6,8,9 clear. Food blinks while the body stays solid during play.

Command/burst timing, startup, scan mode 03, full brightness 8F, per-frame blanking,
initial clears and compatibility register setup are inherited. This sketch adds D0 serial RX, D4 START input with pull-up, D11 buzzer control, and A0/A1 analog inputs; no joystick is attached to the inherited
D1..D13 output bank. Game state and input timing do not use the retained Timer1
flag. A new frame is sent on a 40 ms schedule; the chip performs its own scanning.

## Buzzer

Sound is enabled on D11. The active-buzzer configuration uses HIGH to sound and LOW to silence. Pulses are timed without blocking gameplay: 40ms at start, 60ms for food and 180ms for death/end. The display's D7/D8/D9 signals remain unchanged. The buzzer driver uses a 2N2222 transistor with a 10 kΩ base resistor; D11 is the control signal for this stage. See circuit-inventory.md for the reported components and connection details.

## Validation and limits

Uno compile passed; simulated checks execute the compiled firmware with ADC/UART
and decode GPIO payloads. Engine checks cover growth, food placement, wall/self
collision, entering a departing tail, turn queue, frozen death and a full board.
Integration checks cover the supplied voltage directions, calibration, all 49
isolated bar payloads, death blink, game over, restart and exclusion of symbol
columns. These are not electrical/optical panel tests.

The new button-equipped sketch has **not yet been run on your actual hardware**; the earlier joystick-controlled game was reported working.
Check direction labels and bar assignments before judging gameplay/readability.
If the display stays blank, report startup output; if controls are wrong, report
j values at centre and each direction. Keep working PanelDemo/ManualMap for
comparison. No driver-wide compatibility or final production firmware is claimed.

