# PanelShowcase — display routines for filming

This standalone Uno sketch provides controllable demonstration footage for the
SM1628/SM1628B video. It uses our salvaged panel's numeric map and the inherited
tested transport. It is a demonstration, not Snake gameplay. It can run each
effect separately or a complete repeatable sequence.

## Connect and start

1. Keep the existing tested panel supply, common ground and Uno wiring:
   D7 DATA, D8 CLK, D9 STB.
2. The legacy setup drives D1..D13 as outputs; keep other peripherals off those
   pins. D0 is restored as serial RX. Joystick signals A0/A1 and START on A2
   are not read by this sketch; the game is in PanelSnake.
3. Open firmware/PanelShowcase/PanelShowcase.ino, select Arduino Uno and upload.
   No external library or joystick is required.
4. Open Serial Monitor at **115200 baud**, select **Newline** or **Both NL & CR**.
   Expect the command menu, READY and a blank display. Opening the monitor
   may reset the Uno. Nothing animates until you send a command.
5. First send **digits**. Expect one8 moving from left to right through the
   seven numeric positions, holding each for500ms.
6. Send **stop** to clear. Send **hold** to freeze any current picture.
   Commands stay responsive between animation steps; there are no long delay loops.

Each command occupies its own line. Commands are lowercase and arguments need
a space: pace 250, not pace250. Incomplete lines wait for a line ending.

## Effects and controls

| Command | What you should see |
|---|---|
| digits | An8 sweeps the seven numeric positions, left to right, then repeats |
| groups | Same seven digits, then the right-hand symbol group, wheel group and left-hand symbol group; repeats |
| all | All ten connected display groups held on |
| blink | All ten groups alternate on/off |
| count | All seven numeric positions show0 together, then1 together, up to9, repeating |
| carry | Integer starting at value, right-aligned; leading digits blank; increments with decimal carry |
| value 9995 | Set carry starting number; THEN send carry to animate9995,9996,...10000 |
| bars | Matching a,b,c,d,e,f,g bars across all seven numeric positions |
| chase | Decorative five-bar trail through the49 numeric bars |
| fill | Build up the49 numeric bars, hold, then erase and hold blank; repeats |
| sparkle | Seeded random numeric bars |
| random | Seeded random numeral on each numeric position |
| pace 250 | Change individual-effect step interval to250ms; valid50..2000, default500 |
| seed 7 | Choose repeatable random sequence; valid1..2147483647, default1 |
| demo | Full sequence once, then clear and stop |
| loop | Repeat the full sequence until stopped |
| hold | Keep the current frame; cancels sequence/timing |
| stop / off | Clear immediately and cancel sequence/timing |
| status | Print effect, step, pace, running/sequence flags and MCU elapsed milliseconds |
| ? / help | Print command menu without changing the effect |

Starting an effect restarts its step0 and random seed. Hold has no resume
command; send an effect command to begin again. all is static until another
command. The counter wraps9999999 to0. Input validation rejects unknown names,
extra arguments, invalid ranges and lines longer than31 characters without
modifying the selected effect.

Only groups, all and blink intentionally light symbol columns. This is different
from the old unwanted symbol bits: normal numeral/effect frames still exclude
columns6,8,9. No command writes undefined columns10..15.
groups is a logical group tour, not a strictly left-to-right spatial sweep
of all symbols: some groups contain symbols in separated physical locations.

## Useful individual takes

Start the camera first, then send the command. For example, counting with carry:

```text
stop
pace 250
value 9995
carry
```

The commands above are sent separately, in that order. Expect9995 on the right;
after five increments it becomes10000. Record a little before and after the
change, then send hold or stop.

For an isolated readable sweep:

```text
pace 600
digits
```

For a quicker decorative take:

```text
pace 100
seed 7
sparkle
```

Use the same seed and pace to repeat the same visual progression.

## Full sequence

demo and loop have fixed cue timings; pace/value/seed changes during a sequence
are rejected with guidance to stop/hold first. The seed selected BEFORE starting
the sequence is used for its random effects. Individual-effect pace does not
change the fixed full-sequence timing.

| Stage | Nominal duration |
|---|---:|
| Blank pre-roll |1.50s |
| Seven numeric digits |4.20s |
| All ten groups |5.00s |
| All-on/off flashes |4.00s |
| Synchronized counting0..9 |4.00s |
| Carry from9995, twenty frames |5.00s |
| Five-bar chase, one49-step pass |4.90s |
| Build/erase |6.89s |
| Sparkle |4.20s |
| Random numerals |4.00s |
| All-on hold |1.00s |
| Blank ending |1.00s |

Total: about **46.7 seconds nominal**, followed by DONE/blank for demo, or a fresh
pre-roll for loop. Serial CUE lines give the MCU time and effect name. These are
software cues, not camera timecode or measured audiovisual sync. No camera or
audio playback is controlled, and no locked narration timing is claimed.
Individual all-on remains indefinite; the full demo's final all-on is bounded.

## Recording and review

First record a short sweep and all-on test so we can inspect camera flicker,
focus, exposure and segment readability before recording long takes. Keep the
whole display visible for group/mapping explanations. A separate wide bench
shot can show the Arduino and wiring. The creator chooses final shots/pacing.

Store original takes in the production media folder camera/panel_tests/footage/.
Record the effect, pace, seed (if used) and take number in a note or at the start
of the clip. Preserve originals; MEDIA_INDEX.md will identify useful sections.
Decorative effects should remain distinguishable from real Snake footage.

## Validation and returning to the game

Uno compilation and compiled-firmware checks cover the physical numeric map,
all-group masks, blanking, synchronized count, decimal carry/overflow, seeded
repeatability, stop/hold, invalid commands, full-sequence completion and looping.
These do not measure real optical refresh or camera readability.
The new showcase still needs its first hardware/filming test.

To play Snake again, upload firmware/PanelSnake with SnakeGame.h beside it.
Keep the joystick centred through calibration, then press START (A2 to GND).
The previously accepted joystick/game version remains in repository history.

