# ManualMap: map a salvaged panel and test digits in order

This Arduino Uno sketch is based on Jose Luis Monteiro's working panel test.
It is a diagnostic, not Snake firmware. No automatic stepping or waiting for a
loop: you choose an output, inspect it, then send the next command.

## Setup

1. Open firmware/ManualMap/ManualMap.ino in Arduino IDE and select Arduino Uno.
2. Use the existing tested supply and wiring: Arduino D7 DATA, D8 CLK, D9 STB,
   and common ground. These are GPIO numbers, not panel connector pins.
3. Disconnect other peripherals on D1..D13: this inherited setup makes them
   outputs. D0 is explicitly restored as input for USB serial reception.
   This example is not yet the combined joystick/game wiring.
4. Upload. Open Serial Monitor at **9600 baud** and set **Newline** or **Both NL & CR**.
5. The display starts blank. Read the menu and READY message.

The expected positions below apply to **our salvaged panel**. Other panels may
wire the same chip differently. Start with raw columns to map an unknown panel;
do not assume our ordered map applies to yours.

## A. Reproduce the photographed raw-column test

**First send this exact command:**

```text
column 0 7
```

Here, 0 is the raw column and 7 is the numeral to draw. Expect a 7 on the leftmost
numeric digit. This selection must happen before 
` can advance it.

**Then send:**

```text
n
```

Repeat 
` once per step. `p` goes back. There is no time limit.
Do not type all the steps as one line. Each command needs its own line ending.

| Raw column | Expected with numeral 7 on our panel |
|---|---|
| 0 | First numeric digit from left |
| 1 | Second digit |
| 2 | Fourth digit |
| 3 | Fifth digit |
| 4 | Sixth digit |
| 5 | Seventh/rightmost digit |
| 6 | Return symbol and both colons |
| 7 | Third digit |
| 8 | Three wheel wedges |
| 9 | DVD, VCD and MP3 |

These results were photographed on 2026-10-26. Columns 10..15 have no established
visible result. Raw mode steps through all sixteen storage columns, wrapping
15 to 0. Sixteen is the RAM bit-column range, not the number of connected digits.

The 7 pattern tests the top bar and both right bars. It does not establish that
all seven bars or every symbol work.

## B. Test the numeric digits in physical order

Send:

```text
ordered
```

Expect a 7 on the leftmost digit. Then 
` moves to the second, third, fourth,
fifth, sixth and seventh digit. The next 
` wraps to the first. `p` moves back,
also wrapping. Symbols are skipped. Every result stays lit until changed.

The raw column map is **0, 1, 7, 2, 3, 4, 5** from left to right.
The source names it kDigitColumns. Adapt it only after mapping your own panel.

`ordered` always restarts at the first digit with numeral 7.
To return to raw stepping, send `column 0 7`.
The Serial Monitor status names the mode, raw column, expected physical
position, and selected numeral.

## Other controls

| Command | Action |
|---|---|
| `digit 8` | Optional all-bars test after selecting raw, ordered, or slot mode |
| `digit 7` | Return to the photographed pattern |
| `slot 0 7` | Clean raw slot 0 (same result as column 0 7) |
| `bit 0 0` | Isolate RAM byte 0, bit 0 |
| `set 0 0 1` | Set that bit within the current frame; use 0 to clear |
| `frame` | Print all fourteen RAM bytes in hexadecimal |
| `status` | Print selected mode and expected output |
| `show` | Resend current frame |
| `clear` | Blank and cancel the sequence |
| `help` | Print menu without changing the display |

Slot indices: 0..7; raw columns: 0..15; RAM bytes: 0..13; bits: 0..7; numerals:
0..9. In single-bit mode n/p walks all 112 storage bits. A custom frame made with
set has no stepping sequence; select a mode again before n/p.

## Legacy comparison

`legacy-slot 0 7` explicitly reproduces the old mixed digit/icon payload. Normal slot now selects only numeral strokes. column and ordered were already clean. See symbol-bit-correction.md for the byte-level explanation.

## Troubleshooting and expectations

- **n says no sequence selected:** send `column 0 7` first, or `ordered`.
  Reset and clear both remove the current selection. No display change is expected.
- **No response:** check board/port, 9600 baud and Newline. Opening Serial Monitor
  may reset the Uno; wait for READY, then send the starting command again.
- **Invalid command/range:** check spelling, arguments and ranges. Invalid input
  leaves the display unchanged. The fixed input buffer accepts 47 characters;
  longer lines are discarded.
- **Different physical position:** record the exact command and result. A different
  panel may have different wiring; the expected-position message describes ours.
- **Wrong/missing individual bar:** use bit selection and record results before
  changing the inherited pattern table. The current photos test numeral 7 only.

All display mutations send complete fourteen-byte frames. Original PanelDemo
and the supplied source remain unchanged. ManualMap retains the baseline
transport and initialization with the explicit D0 serial-input change.
The latest ordered mode passes compiled Uno UART/GPIO tests; its new physical
stepping still needs a bench run. Tests do not emulate the panel electrically.

