# Optional application: Snake

Map and check your display first. The reference panel has seven numeric digits
with seven bars each: 49 playable positions. The game targets an Uno and needs
no external Arduino library.

## Connect and upload

Complete [display wiring](01-connect.md). Game connections are:

| Function | Uno pin | Requirement |
|---|---|---|
| Panel DATA / CLK / STB | D7 / D8 / D9 | Each through 1 kΩ; common ground |
| Joystick X / Y | A0 / A1 | Conditioned analog voltages within Uno input range |
| START | D4 | Normally open button toward GND; internal pull-up |
| Active buzzer stage | D11 | Transistor control through 10 kΩ, not direct buzzer power |
| Joystick Z | Unused | No game function |

Follow the [parts and wiring guide](circuit-inventory.md) for the external +5 V
supply, capacitor polarity, joystick resistors, START network and 2N2222 buzzer
stage. All grounds are common. Identify the joystick terminals and transistor
package leads before connecting them; the GPIO table is not a package pinout.

Keep PanelSnake.ino and SnakeGame.h together in firmware/PanelSnake/. Open the
.ino, select Uno/port and upload. Compatibility startup drives D1-D13 except D4
as outputs; reserve D4 for START and D11 for the buzzer stage. Keep unrelated
peripherals off that bank. A0/A1 are analog inputs.

## First run

1. Serial Monitor: 115200 baud. Keep the joystick centred for the one-second
   startup calibration. Expect CENTRE X=... Y=..., then READY.
2. Send `m`. Expect the top bar of the leftmost digit.
3. Send `n` per next bar: a/top, b/upper-right, c/lower-right, d/bottom, e/lower-left,
   f/upper-left, g/middle, then the next digit. All 49 wrap.
4. Send `j` to enable input reports while still in bar-test mode. Move each
   direction separately and check its label. Release should report CENTRE.
   Send `j` again to stop reports.
5. Press START or send `s`. Three top bars start moving right with blinking food
   on a free position. Steer before reaching a missing path.

The joystick steers; centring does not stop the snake and pushing further does
not make it faster. Eating adds one bar and one point. Direct reversal is
ignored. Turns follow bar endpoints. Gaps between digits add no cells.
An up/down turn uses the first side reached before crossing the gap; food on
the adjacent vertical bar needs an approach from the opposite direction.

Collision freezes/blinks the body, then shows approximate GAME, OVER and SCORE
with its value. Serial gives the score clearly; seven-segment letters are
approximations. The game leaves the central colon off. Its display map does not select an
individual colon; turning on the whole group would also enable other symbols.
Release and press START for another round. A button held at boot must first
be released; a press during play does not reset the current round.

## Controls

| Input | Result |
|---|---|
| `s` | Start/restart after calibration, or leave bar check |
| `c` | Blank and recalibrate; centre for one second |
| `j` | Toggle input/direction reports |
| `m`, then `n` | Start and step 49-bar check |
| `w` / `a` / `d` / `z` | Up / left / right / down; s is START |
| `pace 300` plus Enter | Move every 300 ms; 50-2000 ms, lower is faster |
| `pace 450` plus Enter | Default movement speed |
| `?` | Help |

Single-letter controls are immediate. A pace command needs its complete line:
use Newline/Both and a space before the number. Speed changes during play start
a fresh interval, and persist across rounds until reset. Invalid lines leave
speed unchanged. Input, drawing, blinking and beeps run on separate clocks.

## Settings

Compare `j` output before adjusting kSwapAxes, kReverseX, kReverseY near the top
of the sketch. Upload and recalibrate after a change. The supplied directions
have left/up increasing voltage. Startup measures the actual centre.
kEngage=30 and kRelease=18 ADC counts provide hysteresis; this game does not
depend on a 10 mV threshold. Diagonals resolve to one direction. The 5 ms software
input schedule is not a measured end-to-end latency. JoystickTest's 500 ms pause
is for readable bench output, not the game input interval.

D11 is HIGH for sound, LOW for silence. An active buzzer makes its own pitch:
the code uses DC pulses, not tone(): 40 ms at start, 60 ms for food, 180 ms at end.
Use the transistor stage and check its actual package pinout.

Upload [PanelShowcase](display-effects.md) for effects/scripted replay; upload
PanelSnake again for manual play. See [validation](validation.md) for test scope.
