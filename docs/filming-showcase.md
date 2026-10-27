# 6. Display effects and scripted Snake

PanelShowcase is a standalone demonstration using the reference map and working
transport. No joystick is required.

1. Follow [display-only wiring](01-connect.md): D7 DATA, D8 CLK, D9 STB.
2. Keep PanelShowcase.ino, SnakeGame.h, SnakeReplay.h together in its sketch folder.
3. Select Uno/port, upload, open Serial Monitor at 115200 with Newline/Both.
4. Wait for READY. The display starts blank. Send `digits` plus Enter: expect
   one 8 sweeping the seven numeric positions from left to right.

D0 is serial RX. D1-D13 retain output setup except D4, which stays an input with
pull-up. D11 stays LOW/silent. START and joystick are not read.

| Command | Result |
|---|---|
| `digits` | Repeated numeric-position sweep |
| `groups` | Seven digits then symbol groups; a logical tour, not spatial order |
| `all` / `blink` | All ten groups held on / alternating on-off |
| `count` | All numeric positions count 0-9 together |
| `value 9995`, then `carry` | Count 9995, 9996,...10000 with decimal carry |
| `bars` | Matching a-g bars across numeric digits |
| `chase` | Decorative five-bar trail |
| `fill` | Fill49 numeric bars, hold, then erase |
| `sparkle` / `random` | Seeded random bars / numerals |
| `pace 250` | Step interval50-2000 ms, default 500 |
| `seed 7` | Repeatable effects seed1-2147483647 |
| `demo` / `loop` | Full effects sequence once / repeatedly |
| `snake` / `snake-loop` | One fixed game round / repeated round |
| `hold` | Freeze picture and cancel timing |
| `stop` / `off` | Blank and cancel |
| `status` | Print effect/timing state |
| `help` / `?` | Print menu |

One command per line, with spaces before arguments. Each effect restarts at
step 0; hold has no resume. Only groups/all/blink intentionally use symbol columns.
Numeric effects keep them clear. Decorative chase is not collision-based Snake.

For a game demonstration, send `pace 450`, then `snake`, on separate lines.
Expect an initial preview, a fixed legal route eating seven foods, growth from
3 to 10 bars, wall collision, frozen blink, GAME/OVER and SCORE 7. This is scripted
coverage, not a human player's result. It uses the same engine as PanelSnake,
with programmed turns and food. snake-loop adds a one-second blank between
rounds; the random-effects seed does not change the route. D11 stays silent.
Upload PanelSnake again for manual play.
