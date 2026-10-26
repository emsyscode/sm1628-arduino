/*
  PanelSnake - Snake on a salvaged seven-digit SM1628B panel.
  Based on Jose Luis Monteiro's display transport and joystick test.

  Connections
    D7: display DATA     D8: display CLK     D9: display STB
    A0: joystick X      A1: joystick Y
    D4: START button to GND, using the internal pull-up
    D11: 2N2222 buzzer-driver control (10 kOhm base resistor), HIGH = sound
    Keep the existing supply, joystick divider and common ground. Z is unused.

  Operation
    Open Serial Monitor at 115200 baud. Keep the joystick centred during the
    one-second calibration, then press START (or send s) to play.
    The joystick steers; it does not start rounds. A fresh START press begins
    another round after game over. Holding START does not repeatedly restart.

    Serial controls: j = input readings, c = recalibrate, m = bar check,
    n = next bar, s = start/restart, w/a/d/z = up/left/right/down, ? = help.
    Send pace 300 then Enter for 300 ms per move; 50..2000 ms, lower is faster.
    Speed persists across rounds until reset; it does not change input sampling.
    The bar check starts at the first digit's top bar, then steps a..g across
    each digit from left to right. START also leaves the bar check to play.

  The compatibility setup drives D1..D13, except D4, as outputs. D0 is serial RX.
  See docs/snake-test.md for the complete wiring and operating instructions.
*/
#include <Arduino.h>
#include <avr/interrupt.h>
#include "SnakeGame.h"

// Pin assignments.
constexpr uint8_t kDataPin = 7;
constexpr uint8_t kClockPin = 8;
constexpr uint8_t kStrobePin = 9;

constexpr uint8_t kDisplayMode = 0x03;
constexpr uint8_t kFullBrightness = 0x8F;
constexpr uint8_t kFrameBytes = 14;

constexpr uint8_t kXPin = A0;
constexpr uint8_t kYPin = A1;
constexpr uint8_t kStartPin = 4;
constexpr unsigned long kButtonDebounceMs = 30;

// Joystick orientation: left/up increase the input voltage.
constexpr bool kSwapAxes = false;
constexpr bool kReverseX = false;
constexpr bool kReverseY = false;

// Active-buzzer output.
constexpr uint8_t kBuzzerPin = 11;
constexpr uint8_t kBuzzerOnLevel = HIGH;
constexpr uint8_t kBuzzerOffLevel = LOW;

// Independent clocks for gameplay, input sampling and display updates.
uint16_t moveIntervalMs = 450;

// Multi-character pace command; single-key controls still work without Enter.
char paceCommand[24] = {};
uint8_t paceLength = 0;
bool readingPace = false;
bool paceOverflow = false;
constexpr unsigned long kInputMs = 5;
constexpr unsigned long kDrawMs = 40;

// Numeric display columns, left to right.
constexpr uint8_t kDigitColumns[7] = {0, 1, 7, 2, 3, 4, 5};

// Separate engage/release thresholds prevent chatter around the centre.
constexpr int kEngage = 30;
constexpr int kRelease = 18;

// Supplied voltage travel converted to approximate relative spans at nominal 5 V.
// They only balance diagonal axis selection; startup measures actual centre.
constexpr int kXPositiveSpan = 97;
constexpr int kXNegativeSpan = 97;
constexpr int kYPositiveSpan = 185;
constexpr int kYNegativeSpan = 92;

volatile uint8_t legacySeconds = 0;
volatile bool legacyFlag = true;
uint8_t frame[14] = {};
SnakeGame game;

enum Phase : uint8_t {
  CALIBRATING,
  READY,
  PLAYING,
  DYING,
  FINISHED,
  BAR_TEST
};

Phase phase = CALIBRATING;
unsigned long phaseAt = 0;
unsigned long lastInput = 0;
unsigned long lastMove = 0;
unsigned long lastDraw = 0;
unsigned long lastReport = 0;

// Joystick calibration and readings.
unsigned long calibrationX = 0;
unsigned long calibrationY = 0;
uint16_t calibrationSamples = 0;
int rawX = 0;
int rawY = 0;
int centreX = 0;
int centreY = 0;
SnakeGame::Direction stickDirection = SnakeGame::NONE;
bool reportInputs = false;

// Button debounce state.
bool buttonRaw = HIGH;
bool buttonStable = HIGH;
bool buttonArmed = false;
unsigned long buttonChangedAt = 0;

uint8_t testCell = 0;

// Active-buzzer pulse state.
bool buzzerActive = false;
unsigned long buzzerStartedAt = 0;
uint16_t buzzerDurationMs = 0;

void sendCommand(uint8_t value) {
  // STB frames a single command. Keep the original command-specific timing.
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

void silenceBuzzer() {
  digitalWrite(kBuzzerPin, kBuzzerOffLevel);
  buzzerActive = false;
}

void beep(uint16_t durationMs) {
  // An active buzzer generates its own pitch: switch it on for a short pulse.
  digitalWrite(kBuzzerPin, kBuzzerOnLevel);
  buzzerStartedAt = millis();
  buzzerDurationMs = durationMs;
  buzzerActive = true;
}

void updateBuzzer(unsigned long now) {
  if (buzzerActive && now - buzzerStartedAt >= buzzerDurationMs) {
    silenceBuzzer();
  }
}

void setCell(uint8_t cell) {
  if (cell < 49)
    frame[(cell % 7) * 2] |= uint8_t(1u << kDigitColumns[cell / 7]);
}

void glyph(uint8_t digit, uint8_t mask) {
  for (uint8_t segment = 0; segment < 7; ++segment)
    if (mask & (1u << segment))
      setCell(digit * 7 + segment);
}

void scoreFrame() {
  // SCORE and its value share the same seven-position screen.
  const uint8_t label[5] = {0x6D, 0x39, 0x3F, 0x50, 0x79};
  for (uint8_t i = 0; i < 5; ++i)
    glyph(i, label[i]);
  // Standard a..g masks in the inherited row order; numeric font can now use
  // only our seven numeric columns, leaving every symbol column clear.
  const uint8_t numbers[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
  uint8_t score = game.score, digit = 6;
  do {
    glyph(digit--, numbers[score % 10]);
    score /= 10;
  } while (score);
}

void draw(unsigned long now) {
  memset(frame, 0, sizeof(frame));
  if (phase == BAR_TEST)
    setCell(testCell);
  else if (phase == PLAYING || phase == READY || phase == DYING) {
    const bool showBody = phase != DYING || ((now - phaseAt) / 160) % 2 == 0;
    if (showBody)
      for (uint8_t i = 0; i < game.length; ++i)
        setCell(game.body[i]);
    if (phase != DYING && !game.won && (now / 250) % 2 == 0)
      setCell(game.food);
  } else if (phase == FINISHED) {
    const uint8_t screen = ((now - phaseAt) / 1200) % 4;
    if (screen == 0) {  // Seven-segment approximations: G A M E (M resembles n).
      glyph(1, 0x3D);
      glyph(2, 0x77);
      glyph(3, 0x54);
      glyph(4, 0x79);
    } else if (screen == 1) {  // O V E R (V resembles u; R resembles r).
      glyph(1, 0x3F);
      glyph(2, 0x1C);
      glyph(3, 0x79);
      glyph(4, 0x50);
    } else
      scoreFrame();
  }
  sendFrame();
}

void beginCalibration() {
  silenceBuzzer();
  phase = CALIBRATING;
  phaseAt = millis();
  calibrationX = calibrationY = 0;
  calibrationSamples = 0;
  stickDirection = SnakeGame::NONE;
  Serial.println(F("CENTRE joystick now; hold still for one second."));
  draw(phaseAt);
}

void startGame() {
  if (phase == CALIBRATING) {
    Serial.println(F("Wait for READY; joystick is calibrating."));
    return;
  }
  game.reset(uint16_t(random(65536L)));
  phase = PLAYING;
  phaseAt = lastMove = millis();
  beep(40);
  Serial.println(F("PLAY. Food blinks. w/a/d/z also steer; s restarts."));
}

void reportDirection(SnakeGame::Direction direction) {
  switch (direction) {
    case SnakeGame::UP:
      Serial.print(F("UP"));
      break;
    case SnakeGame::RIGHT:
      Serial.print(F("RIGHT"));
      break;
    case SnakeGame::DOWN:
      Serial.print(F("DOWN"));
      break;
    case SnakeGame::LEFT:
      Serial.print(F("LEFT"));
      break;
    default:
      Serial.print(F("CENTRE"));
      break;
  }
}

void readStick(unsigned long now) {
  // Discard first reading after changing ADC channel, then average four
  // samples. No 500ms delay: input and game movement have separate clocks.
  analogRead(kXPin);
  int x = 0;
  for (uint8_t i = 0; i < 4; ++i)
    x += analogRead(kXPin);
  analogRead(kYPin);
  int y = 0;
  for (uint8_t i = 0; i < 4; ++i)
    y += analogRead(kYPin);
  rawX = x / 4;
  rawY = y / 4;
  if (kSwapAxes) {
    const int temporary = rawX;
    rawX = rawY;
    rawY = temporary;
  }
  if (phase == CALIBRATING) {
    calibrationX += rawX;
    calibrationY += rawY;
    ++calibrationSamples;
    if (now - phaseAt >= 1000 && calibrationSamples) {
      centreX = calibrationX / calibrationSamples;
      centreY = calibrationY / calibrationSamples;
      randomSeed(uint32_t(micros()) ^ (uint32_t(rawX) << 16) ^ uint32_t(rawY));
      game.reset(uint16_t(random(65536L)));
      phase = READY;
      phaseAt = now;
      Serial.print(F("CENTRE X="));
      Serial.print(centreX);
      Serial.print(F(" Y="));
      Serial.println(centreY);
      Serial.println(F("READY. Check j input labels. Press START or send s to play."));
    }
    return;
  }
  int dx = rawX - centreX, dy = rawY - centreY;
  if (kReverseX)
    dx = -dx;
  if (kReverseY)
    dy = -dy;
  const int threshold = stickDirection == SnakeGame::NONE ? kEngage : kRelease;
  SnakeGame::Direction next = SnakeGame::NONE;
  if (abs(dx) >= threshold || abs(dy) >= threshold) {
    const long nx = long(abs(dx)) * 1000 / (dx >= 0 ? kXPositiveSpan : kXNegativeSpan);
    const long ny = long(abs(dy)) * 1000 / (dy >= 0 ? kYPositiveSpan : kYNegativeSpan);
    // Each candidate must itself pass the threshold. Only one direction is sent.
    if (abs(dx) >= threshold && (abs(dy) < threshold || nx >= ny))
      next = dx > 0 ? SnakeGame::LEFT : SnakeGame::RIGHT;
    else
      next = dy > 0 ? SnakeGame::UP : SnakeGame::DOWN;
  }
  stickDirection = next;
  if (phase == PLAYING)
    game.turn(next);
  if (reportInputs && now - lastReport >= 200) {
    lastReport = now;
    Serial.print(F("X="));
    Serial.print(rawX);
    Serial.print(F(" Y="));
    Serial.print(rawY);
    Serial.print(F(" dX="));
    Serial.print(dx);
    Serial.print(F(" dY="));
    Serial.print(dy);
    Serial.print(' ');
    reportDirection(next);
    Serial.println();
  }
}

void readStartButton(unsigned long now) {
  const bool reading = digitalRead(kStartPin);
  if (reading != buttonRaw) {
    buttonRaw = reading;
    buttonChangedAt = now;
  }
  if (now - buttonChangedAt < kButtonDebounceMs || buttonStable == buttonRaw)
    return;
  buttonStable = buttonRaw;
  if (buttonStable == HIGH)
    buttonArmed = true;
  else {
    // One event per press. A held/bouncing button cannot restart a round.
    const bool accepted = buttonArmed;
    buttonArmed = false;
    if (accepted && (phase == READY || phase == FINISHED || phase == BAR_TEST))
      startGame();
  }
}

void printBar() {
  Serial.print(F("BAR digit="));
  Serial.print(testCell / 7 + 1);
  Serial.print(F(" from left, segment="));
  Serial.print(char('a' + testCell % 7));
  Serial.print(F(" raw column="));
  Serial.println(kDigitColumns[testCell / 7]);
}

void finishPaceCommand() {
  char* end = nullptr;
  const long value = strtol(paceCommand + 5, &end, 10);
  if (!paceOverflow && paceLength <= 9 && strncmp(paceCommand, "pace ", 5) == 0 &&
      end != paceCommand + 5 && *end == '\0' && value >= 50 && value <= 2000) {
    moveIntervalMs = uint16_t(value);
    lastMove = millis();  // Start a full interval at the new speed.
    Serial.print(F("PACE "));
    Serial.print(moveIntervalMs);
    Serial.println(F(" ms per move. Lower = faster."));
  } else {
    Serial.println(F("Use pace 50..2000, then Enter. Example: pace 450"));
  }
  readingPace = false;
  paceLength = 0;
  paceOverflow = false;
  memset(paceCommand, 0, sizeof(paceCommand));
}

void commands() {
  while (Serial.available()) {
    const char c = char(Serial.read());
    if (readingPace) {
      if (c == '\r' || c == '\n') {
        finishPaceCommand();
      } else if (paceLength < sizeof(paceCommand) - 1) {
        paceCommand[paceLength++] = c;
      } else {
        paceOverflow = true;
      }
      continue;
    }
    if (c == 'p') {
      readingPace = true;
      paceCommand[paceLength++] = c;
      continue;
    }
    if (c == '\r' || c == '\n' || c == ' ')
      continue;
    switch (c) {
      case 's':
        startGame();
        break;
      case 'c':
        beginCalibration();
        break;
      case 'j':
        reportInputs = !reportInputs;
        Serial.println(reportInputs ? F("Input reports ON.") : F("Input reports OFF."));
        break;
      case 'm':
        if (phase == CALIBRATING) {
          Serial.println(F("Wait for READY."));
          break;
        }
        phase = BAR_TEST;
        testCell = 0;
        printBar();
        break;
      case 'n':
        if (phase != BAR_TEST)
          Serial.println(F("First send m for the bar test, then n."));
        else {
          testCell = (testCell + 1) % 49;
          printBar();
        }
        break;
      case 'w':
        game.turn(SnakeGame::UP);
        break;
      case 'a':
        game.turn(SnakeGame::LEFT);
        break;
      case 'd':
        game.turn(SnakeGame::RIGHT);
        break;
      case 'z':
        game.turn(SnakeGame::DOWN);
        break;
      case '?':
        Serial.println(
            F("s=start/restart c=calibrate j=inputs m=bar test n=next bar w/a/d/z=steer; pace "
              "50..2000 + Enter"));
        break;
      default:
        Serial.println(F("Unknown key; send ? for help."));
        break;
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  cli();
  // Preserve legacy startup/Timer1 while isolating gameplay above it.
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
  pinMode(0, INPUT);  // USB serial RX, as in ManualMap.
  pinMode(kXPin, INPUT);
  pinMode(kYPin, INPUT);
  analogReference(DEFAULT);
  pinMode(kStartPin, INPUT_PULLUP);
  pinMode(kBuzzerPin, OUTPUT);
  silenceBuzzer();
  buttonRaw = buttonStable = digitalRead(kStartPin);
  buttonArmed = buttonStable == HIGH;  // Held at boot: release before first press.
  buttonChangedAt = millis();
  initializePanel();
  clearPanel();
  sei();
  Serial.println(F("PanelSnake. A0=X A1=Y START=D4-GND BUZZER=D11 (active)."));
  Serial.println(
      F("s=start c=calibrate j=inputs m=bar test n=next bar pace 450 + Enter=speed ?=help"));
  beginCalibration();
}

void loop() {
  unsigned long now = millis();
  commands();
  now = millis();  // Commands may start a new calibration/time epoch.
  updateBuzzer(now);
  if (now - lastInput >= kInputMs) {
    lastInput = now;
    readStartButton(now);
    readStick(now);
  }
  if (phase == PLAYING && now - lastMove >= moveIntervalMs) {
    lastMove = now;  // No catch-up burst if serial or a frame took longer.
    const SnakeGame::Result result = game.step(uint16_t(random(65536L)));
    if (result == SnakeGame::ATE) {
      Serial.print(F("FOOD score="));
      Serial.println(game.score);
      beep(60);
    } else if (result == SnakeGame::DIED || result == SnakeGame::WON) {
      phase = DYING;
      phaseAt = now;
      beep(180);
      Serial.print(result == SnakeGame::WON ? F("BOARD COMPLETE score=") : F("DEAD score="));
      Serial.println(game.score);
    }
  }
  if (phase == DYING && now - phaseAt >= 1600) {
    phase = FINISHED;
    phaseAt = now;
    Serial.print(F("GAME OVER score="));
    Serial.println(game.score);
    Serial.println(F("Press START (D4-GND) or send s to restart."));
  }
  if (now - lastDraw >= kDrawMs) {
    lastDraw = now;
    draw(now);
  }
}

ISR(TIMER1_COMPA_vect) {
  ++legacySeconds;
  legacyFlag = !legacyFlag;
}
