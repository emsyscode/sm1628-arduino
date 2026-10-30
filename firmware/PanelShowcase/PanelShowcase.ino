/*
 PanelShowcase - display effects for our salvaged SM1628B panel.
 Display demonstration in the SM1628 / SM1628B driver tutorial.
 Uno: D7 DATA, D8 CLK, D9 STB; keep the tested supply/common ground.
 D1..D13 are outputs, except D4. D0 is serial RX. D4 START remains an input; D11
 active buzzer remains silent. No joystick needed.

 UPLOAD AND USE
 Open Serial Monitor: 115200 baud, Newline or Both. Starts BLANK.
 First send digits: expect an8 moving left to right across seven numeric digits.
 Send groups to include the three symbol groups after the numeric positions.
 Send demo for one complete sequence, or loop to repeat it.
 stop clears immediately; hold freezes the current picture. No long delays.
 count makes all digits show0..9 together. carry counts from the right.
 For a visible carry: send value 9995, THEN carry (9995 ->10000).
 pace250 is NOT valid: send pace 250 on its own line.
 Send ? for all commands and status for the current state.

 snake plays one scripted round; snake-loop repeats it without joystick input.
 Use pace 450 before snake to match the game's movement interval.
 The replay uses fixed turns and food positions; use PanelSnake for manual play.
 Symbols are intentionally included only in groups/all/blink.
 Other panels may require a different column/segment map.
*/
#include <Arduino.h>
#include <avr/interrupt.h>
#include <string.h>
#include "SnakeReplay.h"
constexpr uint8_t kStartPin = 4;
constexpr uint8_t kBuzzerPin = 11;

constexpr uint8_t kDataPin = 7, kClockPin = 8, kStrobePin = 9;
constexpr uint8_t kDisplayMode = 0x03, kFullBrightness = 0x8F, kFrameBytes = 14;
constexpr uint8_t kDigitColumns[7] = {0, 1, 7, 2, 3, 4, 5};
constexpr uint8_t kGroupColumns[10] = {0, 1, 7, 2, 3, 4, 5, 6, 8, 9};
constexpr uint8_t kNumerals[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
volatile uint8_t legacySeconds = 0;
volatile bool legacyFlag = true;
uint8_t frame[14] = {};

enum Effect : uint8_t {
  OFF,
  DIGITS,
  GROUPS,
  ALL_ON,
  BLINK,
  COUNT,
  CARRY,
  CHASE,
  FILL,
  SPARKLE,
  RANDOM_DIGITS,
  BARS,
  SNAKE_REPLAY
};

uint8_t effect = OFF, cueIndex = 0;
uint32_t step = 0, counterStart = 0, seed = 1, randomState = 1;
unsigned long lastStepAt = 0;
uint16_t intervalMs = 500, individualPace = 500;
bool running = false, sequence = false, repeating = false;
char line[32];
uint8_t lineLength = 0;
bool discardLine = false;

constexpr uint16_t kReplayDrawMs = 40;
SnakeReplay replay;

// Each cue is shown immediately, then held for ticks * pace milliseconds.
// Begins with 1.5 s blank and ends blank; about 46.7 s total.
struct Cue {
  uint8_t effect;
  uint16_t pace, ticks;
  uint32_t value;
};

const Cue kCues[] = {{OFF, 1500, 1, 0},           {DIGITS, 600, 7, 0},  {GROUPS, 500, 10, 0},
                     {BLINK, 500, 8, 0},          {COUNT, 400, 10, 0},  {CARRY, 250, 20, 9995},
                     {CHASE, 100, 49, 0},         {FILL, 65, 106, 0},   {SPARKLE, 120, 35, 0},
                     {RANDOM_DIGITS, 200, 20, 0}, {ALL_ON, 1000, 1, 0}, {OFF, 1000, 1, 0}};
constexpr uint8_t kCueCount = sizeof(kCues) / sizeof(kCues[0]);

void sendCommand(uint8_t value) {
  // STB frames a single command. Keep the command-specific timing used by this sender.
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    digitalWrite(kClockPin, LOW);
    delayMicroseconds(1);
    digitalWrite(kDataPin, (value & mask) ? HIGH : LOW);
    digitalWrite(kClockPin, HIGH);
    delayMicroseconds(1);
  }
  digitalWrite(kStrobePin, HIGH);
  delayMicroseconds(1);
}

void sendBurstByte(uint8_t value) {
  // STB is controlled by the caller, allowing address + data in one packet.
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    digitalWrite(kClockPin, LOW);
    digitalWrite(kDataPin, (value & mask) ? HIGH : LOW);
    delayMicroseconds(5);
    digitalWrite(kClockPin, HIGH);
    delayMicroseconds(5);
  }
}

void initializePanel() {
  delayMicroseconds(200);
  sendCommand(kDisplayMode);
  delayMicroseconds(1);
  sendCommand(0x40);
  delayMicroseconds(1);
  sendCommand(0xC0);
  delayMicroseconds(1);
  sendCommand(kFullBrightness);
  delayMicroseconds(1);
}

void clearPanel() {
  // Preserve the reference's fourteen separate addressed writes.
  for (uint8_t address = 0; address < kFrameBytes; ++address) {
    sendCommand(0x40);
    digitalWrite(kStrobePin, LOW);
    delayMicroseconds(1);
    sendBurstByte(0xC0 | address);
    sendBurstByte(0);
    digitalWrite(kStrobePin, HIGH);
    delayMicroseconds(1);
  }
}

void sendFrame() {
  delayMicroseconds(1);
  sendCommand(kDisplayMode);
  sendCommand(0x80);
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  sendBurstByte(0xC0);
  for (uint8_t i = 0; i < 14; ++i)
    sendBurstByte(frame[i]);
  digitalWrite(kStrobePin, HIGH);
  sendCommand(kFullBrightness);
}

void columnGlyph(uint8_t column, uint8_t mask) {
  if (column > 9)
    return;  // Only ten established panel groups.
  for (uint8_t row = 0; row < 7; ++row)
    if (mask & (1u << row))
      frame[row * 2 + column / 8] |= uint8_t(1u << (column % 8));
}

void cell(uint8_t id) {
  columnGlyph(kDigitColumns[id / 7], uint8_t(1u << (id % 7)));
}

uint32_t randomNext() {
  randomState ^= randomState << 13;
  randomState ^= randomState >> 17;
  randomState ^= randomState << 5;
  return randomState;
}

const __FlashStringHelper* effectName(uint8_t e) {
  switch (e) {
    case DIGITS:
      return F("digits");
    case GROUPS:
      return F("groups");
    case ALL_ON:
      return F("all");
    case BLINK:
      return F("blink");
    case COUNT:
      return F("count");
    case CARRY:
      return F("carry");
    case CHASE:
      return F("chase");
    case FILL:
      return F("fill");
    case SPARKLE:
      return F("sparkle");
    case RANDOM_DIGITS:
      return F("random");
    case BARS:
      return F("bars");
    case SNAKE_REPLAY:
      return F("snake-replay");
    default:
      return F("off");
  }
}

void render() {
  memset(frame, 0, sizeof(frame));
  switch (effect) {
    case DIGITS:
      columnGlyph(kDigitColumns[step % 7], 0x7F);
      break;
    case GROUPS:
      columnGlyph(kGroupColumns[step % 10], 0x7F);
      break;
    case ALL_ON:
    case BLINK:
      if (effect == ALL_ON || step % 2 == 0)
        for (uint8_t col = 0; col < 10; ++col)
          columnGlyph(col, 0x7F);
      break;
    case COUNT:
      for (uint8_t digit = 0; digit < 7; ++digit)
        columnGlyph(kDigitColumns[digit], kNumerals[step % 10]);
      break;
    case CARRY: {
      // Blank leading positions; every written digit uses its physical map.
      uint32_t value = (counterStart + (step % 10000000UL)) % 10000000UL;
      int8_t digit = 6;
      do {
        columnGlyph(kDigitColumns[digit--], kNumerals[value % 10]);
        value /= 10;
      } while (value && digit >= 0);
      break;
    }
    case CHASE:
      for (uint8_t tail = 0; tail < 5; ++tail)
        cell(uint8_t((step + 49 - tail) % 49));
      break;
    case FILL: {
      const uint8_t p = step % 106;
      const uint8_t count = p < 49 ? p + 1 : (p < 53 ? 49 : (p < 102 ? 101 - p : 0));
      for (uint8_t i = 0; i < count; ++i)
        cell(i);
      break;
    }
    case SPARKLE:
      for (uint8_t i = 0; i < 12; ++i)
        cell(randomNext() % 49);
      break;
    case RANDOM_DIGITS:
      for (uint8_t digit = 0; digit < 7; ++digit)
        columnGlyph(kDigitColumns[digit], kNumerals[randomNext() % 10]);
      break;
    case BARS:
      for (uint8_t digit = 0; digit < 7; ++digit)
        columnGlyph(kDigitColumns[digit], uint8_t(1u << (step % 7)));
      break;
    case SNAKE_REPLAY:
      replay.render(frame, kDigitColumns, millis());
      break;
    default:
      break;
  }
  sendFrame();
}

void status() {
  Serial.print(F("CUE ms="));
  Serial.print(millis());
  Serial.print(F(" effect="));
  Serial.print(effectName(effect));
  Serial.print(F(" step="));
  Serial.print(step);
  Serial.print(F(" pace="));
  Serial.print(intervalMs);
  Serial.print(F(" running="));
  Serial.print(running);
  Serial.print(F(" sequence="));
  Serial.println(sequence);
}

void activate(uint8_t e, uint16_t pace, uint32_t value) {
  effect = e;
  intervalMs = pace;
  counterStart = value;
  step = 0;
  randomState = seed;
  lastStepAt = millis();
  running = e != OFF && e != ALL_ON;
  render();
  status();
}

void startReplay(bool repeat) {
  sequence = false;
  repeating = false;
  effect = SNAKE_REPLAY;
  intervalMs = kReplayDrawMs;
  step = 0;
  lastStepAt = millis();

  replay.begin(lastStepAt, repeat, individualPace);
  running = true;
  render();
  status();

  Serial.println(repeat ? F("SNAKE REPLAY LOOP: scripted turns/food; stop or hold to finish.")
                        : F("SNAKE REPLAY: scripted round; final score holds."));
}

void updateReplay(unsigned long now) {
  const SnakeReplay::Event event = replay.update(now);

  if (event == SnakeReplay::FOOD) {
    Serial.print(F("REPLAY FOOD score="));
    Serial.println(replay.game.score);
  } else if (event == SnakeReplay::COLLISION) {
    Serial.print(F("REPLAY COLLISION score="));
    Serial.println(replay.game.score);
  } else if (event == SnakeReplay::DONE) {
    running = false;
    render();
    Serial.print(F("SNAKE REPLAY DONE score="));
    Serial.println(replay.game.score);
  } else if (event == SnakeReplay::RESTARTED) {
    Serial.println(F("SNAKE REPLAY RESTART."));
  }
}

void beginCue() {
  const Cue& cue = kCues[cueIndex];
  activate(cue.effect, cue.pace, cue.value);
  running = true;  // Blank/all-on cues still have a bounded duration in demo.
}

void beginSequence(bool repeat) {
  sequence = true;
  repeating = repeat;
  cueIndex = 0;
  Serial.println(repeat ? F("LOOP: full sequence repeats until stop/hold.")
                        : F("DEMO: one full sequence, then blank."));
  beginCue();
}

void stop(bool clear) {
  sequence = false;
  repeating = false;
  running = false;
  if (clear) {
    effect = OFF;
    memset(frame, 0, sizeof(frame));
    sendFrame();
  }
  Serial.println(clear ? F("STOP: blank.")
                       : F("HOLD: current frame frozen; start an effect to resume."));
}

bool number(const char* text, uint32_t maximum, uint32_t& value) {
  if (!text || !*text)
    return false;
  value = 0;
  for (const char* p = text; *p; ++p) {
    if (*p < '0' || *p > '9')
      return false;
    const uint8_t digit = *p - '0';
    if (value > maximum / 10 || (value == maximum / 10 && digit > maximum % 10))
      return false;
    value = value * 10 + digit;
  }
  return true;
}

void help() {
  Serial.println(F("PanelShowcase: 115200 baud, Newline/Both. START: digits"));
  Serial.println(F("snake=scripted round; snake-loop=repeats; final score7."));
  Serial.println(F("Use pace 450 then snake for game-speed playback; default pace500."));
  Serial.println(
      F("digits=7 numeric positions left-to-right; groups=7 digits then3 symbol groups"));
  Serial.println(F("all=all connected groups ON; off/stop=blank; blink=all groups flash"));
  Serial.println(F("count=0000000..9999999 in sync; carry=right-aligned incrementing integer"));
  Serial.println(F("value 9995 THEN carry shows rollover to10000. value range0..9999999."));
  Serial.println(F("chase=5-bar trail; fill=build/erase49 bars; bars=matching a..g across digits"));
  Serial.println(F("sparkle=random bars; random=random numerals; seed 1=reproducible effects"));
  Serial.println(F("pace 250 sets individual step interval50..2000ms (default500)."));
  Serial.println(F("demo=full sequence ONCE; loop=repeats; hold=freeze; stop=blank immediately"));
  Serial.println(F("Demo timings are fixed; pace is rejected during demo. status=state; ?=help."));
  Serial.println(F("Snake mode plays a fixed round without joystick input."));
}

void executeLine() {
  char* cmd = strtok(line, " \t");
  if (!cmd)
    return;
  char* arg = strtok(nullptr, " \t");
  if (strtok(nullptr, " \t")) {
    Serial.println(F("ERR: too many arguments; unchanged."));
    return;
  }
  uint32_t value = 0;
  if (!strcmp(cmd, "pace") || !strcmp(cmd, "value") || !strcmp(cmd, "seed")) {
    const uint32_t maximum =
        !strcmp(cmd, "pace") ? 2000UL : (!strcmp(cmd, "value") ? 9999999UL : 2147483647UL);
    if (!number(arg, maximum, value) || (!strcmp(cmd, "pace") && value < 50) ||
        (!strcmp(cmd, "seed") && !value)) {
      Serial.println(F("ERR: use pace50..2000, value0..9999999 or seed1..2147483647."));
      return;
    }
    if (sequence) {
      Serial.println(F("ERR: stop/hold demo before changing settings."));
      return;
    }
    if (!strcmp(cmd, "pace")) {
      individualPace = uint16_t(value);
      if (effect == SNAKE_REPLAY) {
        replay.moveIntervalMs = individualPace;
        intervalMs = kReplayDrawMs;
      } else {
        intervalMs = individualPace;
      }
      lastStepAt = millis();
    } else if (!strcmp(cmd, "value")) {
      counterStart = value;
      step = 0;
      if (effect == CARRY)
        render();
    } else {
      seed = value;
      randomState = seed;
    }
    Serial.println(F("OK setting."));
    return;
  }
  if (arg) {
    Serial.println(F("ERR: this command takes no argument; unchanged."));
    return;
  }
  if (!strcmp(cmd, "?") || !strcmp(cmd, "help")) {
    help();
    return;
  }
  if (!strcmp(cmd, "status")) {
    status();
    return;
  }
  if (!strcmp(cmd, "stop") || !strcmp(cmd, "off")) {
    stop(true);
    return;
  }
  if (!strcmp(cmd, "hold")) {
    stop(false);
    return;
  }
  if (!strcmp(cmd, "snake") || !strcmp(cmd, "snake-loop")) {
    startReplay(!strcmp(cmd, "snake-loop"));
    return;
  }
  if (!strcmp(cmd, "demo") || !strcmp(cmd, "loop")) {
    beginSequence(!strcmp(cmd, "loop"));
    return;
  }
  uint8_t next = OFF;
  if (!strcmp(cmd, "digits"))
    next = DIGITS;
  else if (!strcmp(cmd, "groups"))
    next = GROUPS;
  else if (!strcmp(cmd, "all"))
    next = ALL_ON;
  else if (!strcmp(cmd, "blink"))
    next = BLINK;
  else if (!strcmp(cmd, "count"))
    next = COUNT;
  else if (!strcmp(cmd, "carry"))
    next = CARRY;
  else if (!strcmp(cmd, "chase"))
    next = CHASE;
  else if (!strcmp(cmd, "fill"))
    next = FILL;
  else if (!strcmp(cmd, "sparkle"))
    next = SPARKLE;
  else if (!strcmp(cmd, "random"))
    next = RANDOM_DIGITS;
  else if (!strcmp(cmd, "bars"))
    next = BARS;
  else {
    Serial.println(F("ERR: unknown command; send ?; unchanged."));
    return;
  }
  sequence = false;
  repeating = false;
  activate(next, individualPace, counterStart);
}

void commands() {
  while (Serial.available()) {
    const char c = char(Serial.read());
    if (c == '\r' || c == '\n') {
      if (discardLine)
        Serial.println(F("ERR: line too long; unchanged."));
      else if (lineLength) {
        line[lineLength] = '\0';
        executeLine();
      }
      lineLength = 0;
      discardLine = false;
    } else if (!discardLine) {
      if (lineLength < sizeof(line) - 1)
        line[lineLength++] = c;
      else
        discardLine = true;
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 62498;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  CLKPR = 0x80;
  DDRD = 0xFF;
  PORTD = 0;
  DDRB = 0xFF;
  PORTB = 0;
  pinMode(0, INPUT);
  pinMode(kStartPin, INPUT_PULLUP);
  pinMode(kBuzzerPin, OUTPUT);
  digitalWrite(kBuzzerPin, LOW);
  initializePanel();
  clearPanel();
  sei();
  help();
  Serial.println(F("READY: blank. First send digits, demo, or loop."));
}

void loop() {
  commands();
  const unsigned long now = millis();

  if (running && effect == SNAKE_REPLAY) {
    updateReplay(now);
  }

  if (running && now - lastStepAt >= intervalMs) {
    lastStepAt = now;
    ++step;  // No late catch-up burst.
    if (sequence && step >= kCues[cueIndex].ticks) {
      ++cueIndex;
      if (cueIndex == kCueCount) {
        if (repeating) {
          cueIndex = 0;
          beginCue();
        } else {
          stop(true);
          Serial.println(F("DEMO DONE."));
        }
      } else
        beginCue();
    } else
      render();
  }
}

ISR(TIMER1_COMPA_vect) {
  ++legacySeconds;
  legacyFlag = !legacyFlag;
}

