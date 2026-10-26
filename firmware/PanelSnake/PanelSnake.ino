/*
 PanelSnake - joystick Snake with a debounced START button for our SM1628B panel.
 Based on Jose Luis Monteiro's working transport and supplied joystick test.

 Uno wiring: D7 DATA, D8 CLK, D9 STB. Joystick signals: A0=X, A1=Y (initial
 assignment to verify); retain the existing tested supply/divider and ground.
 Start button: A2 to GND, internal pull-up (released HIGH, pressed LOW).
 Z is unused. The legacy setup still drives D1..D13 as outputs: disconnect
 other peripherals there. Optional passive buzzer is disabled below.

 HOW TO TEST
 1. Open Serial Monitor at 115200 baud. Keep joystick CENTRED through the
    one-second calibration. Expected: CENTRE ... then READY.
 2. Send j to print input diagnostics; move each way and check its label.
    Send c with stick centred to recalibrate; x/y swap/reversal constants below.
 3. Optional: send m to isolate the first digit's top bar. Send n to step
    a,b,c,d,e,f,g on each numeric digit left to right. These assignments still
    need a hardware check. s leaves this test and starts the game.
 4. Press START (A2 to GND), or send s, to start. Food blinks; the
    snake moves at a fixed speed even when the stick returns to centre.
 5. Collision freezes/blinks the body, then shows approximate GAME / OVER
    letters and the score. Release and press START for another round, or send s.
 Serial alternatives: w/a/d/z = up/left/right/down (s is START, not down).

 No upload/hardware validation is implied by compilation. Digit order was
 photographed with numeral 7; other individual bars remain a bench check.
*/
#include <Arduino.h>
#include <avr/interrupt.h>
#include "SnakeGame.h"

constexpr uint8_t kDataPin = 7, kClockPin = 8, kStrobePin = 9;
constexpr uint8_t kDisplayMode = 0x03, kFullBrightness = 0x8F, kFrameBytes = 14;
constexpr uint8_t kXPin = A0, kYPin = A1;
constexpr uint8_t kStartPin = A2;
constexpr unsigned long kButtonDebounceMs = 30;
constexpr bool kSwapAxes = false, kReverseX = false, kReverseY = false;
// Default signs follow supplied measurements: left/up raise voltage.
constexpr int8_t kBuzzerPin = -1; // Optional: set 3 only after wiring a suitable passive buzzer.
constexpr unsigned long kMoveMs = 450, kInputMs = 5, kDrawMs = 40;
constexpr uint8_t kDigitColumns[7] = {0, 1, 7, 2, 3, 4, 5};
// ADC-count thresholds for this FIRST trial, not measured noise guarantees.
constexpr int kEngage = 30, kRelease = 18;
// Supplied voltage travel converted to approximate relative spans at nominal5V.
// They only balance diagonal axis selection; startup measures actual centre.
constexpr int kXPositiveSpan = 97, kXNegativeSpan = 97;
constexpr int kYPositiveSpan = 185, kYNegativeSpan = 92;

volatile uint8_t legacySeconds = 0;
volatile bool legacyFlag = true;
uint8_t frame[14] = {};
SnakeGame game;
enum Phase : uint8_t { CALIBRATING, READY, PLAYING, DYING, FINISHED, BAR_TEST };
Phase phase = CALIBRATING;
unsigned long phaseAt = 0, lastInput = 0, lastMove = 0, lastDraw = 0, lastReport = 0;
unsigned long calibrationX = 0, calibrationY = 0;
uint16_t calibrationSamples = 0;
int rawX = 0, rawY = 0, centreX = 0, centreY = 0;
SnakeGame::Direction stickDirection = SnakeGame::NONE;
bool reportInputs = false;
bool buttonRaw = HIGH, buttonStable = HIGH, buttonArmed = false;
unsigned long buttonChangedAt = 0;
uint8_t testCell = 0;

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
  sendCommand(kDisplayMode); delayMicroseconds(1);
  sendCommand(0x40);         delayMicroseconds(1);
  sendCommand(0xC0);         delayMicroseconds(1);
  sendCommand(kFullBrightness); delayMicroseconds(1);
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
  sendCommand(kDisplayMode); sendCommand(0x80);
  digitalWrite(kStrobePin, LOW); delayMicroseconds(1);
  sendBurstByte(0xC0);
  for (uint8_t i = 0; i < 14; ++i) sendBurstByte(frame[i]);
  digitalWrite(kStrobePin, HIGH); sendCommand(kFullBrightness);
}
void beep(uint16_t frequency, uint16_t duration) {
  if (kBuzzerPin >= 0) tone(uint8_t(kBuzzerPin), frequency, duration);
}
void setCell(uint8_t cell) {
  if (cell < 49) frame[(cell % 7) * 2] |= uint8_t(1u << kDigitColumns[cell / 7]);
}
void glyph(uint8_t digit, uint8_t mask) {
  for (uint8_t segment = 0; segment < 7; ++segment)
    if (mask & (1u << segment)) setCell(digit * 7 + segment);
}
void scoreFrame() {
  // Standard a..g masks in the inherited row order; numeric font can now use
  // only our seven numeric columns, leaving every symbol column clear.
  const uint8_t numbers[10] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
  uint8_t score = game.score, digit = 6;
  do { glyph(digit--, numbers[score % 10]); score /= 10; } while (score);
}
void draw(unsigned long now) {
  memset(frame, 0, sizeof(frame));
  if (phase == BAR_TEST) setCell(testCell);
  else if (phase == PLAYING || phase == READY || phase == DYING) {
    const bool showBody = phase != DYING || ((now - phaseAt) / 160) % 2 == 0;
    if (showBody) for (uint8_t i = 0; i < game.length; ++i) setCell(game.body[i]);
    if (phase != DYING && !game.won && (now / 250) % 2 == 0) setCell(game.food);
  } else if (phase == FINISHED) {
    const uint8_t screen = ((now - phaseAt) / 1200) % 4;
    if (screen == 0) { // Seven-segment approximations: G A M E (M resembles n).
      glyph(1,0x3D); glyph(2,0x77); glyph(3,0x54); glyph(4,0x79);
    } else if (screen == 1) { // O V E R (V resembles u; R resembles r).
      glyph(1,0x3F); glyph(2,0x1C); glyph(3,0x79); glyph(4,0x50);
    } else scoreFrame();
  }
  sendFrame();
}
void beginCalibration() {
  phase = CALIBRATING; phaseAt = millis();
  calibrationX = calibrationY = 0; calibrationSamples = 0;
  stickDirection = SnakeGame::NONE;
  Serial.println(F("CENTRE joystick now; hold still for one second."));
  draw(phaseAt);
}
void startGame() {
  if (phase == CALIBRATING) {
    Serial.println(F("Wait for READY; joystick is calibrating.")); return;
  }
  game.reset(uint16_t(random(65536L)));
  phase = PLAYING; phaseAt = lastMove = millis();
  Serial.println(F("PLAY. Food blinks. w/a/d/z also steer; s restarts."));
}
void reportDirection(SnakeGame::Direction direction) {
  switch (direction) {
    case SnakeGame::UP: Serial.print(F("UP")); break;
    case SnakeGame::RIGHT: Serial.print(F("RIGHT")); break;
    case SnakeGame::DOWN: Serial.print(F("DOWN")); break;
    case SnakeGame::LEFT: Serial.print(F("LEFT")); break;
    default: Serial.print(F("CENTRE")); break;
  }
}
void readStick(unsigned long now) {
  // Discard first reading after changing ADC channel, then average four
  // samples. No 500ms delay: input and game movement have separate clocks.
  analogRead(kXPin); int x = 0;
  for (uint8_t i = 0; i < 4; ++i) x += analogRead(kXPin);
  analogRead(kYPin); int y = 0;
  for (uint8_t i = 0; i < 4; ++i) y += analogRead(kYPin);
  rawX = x / 4; rawY = y / 4;
  if (kSwapAxes) { const int temporary = rawX; rawX = rawY; rawY = temporary; }
  if (phase == CALIBRATING) {
    calibrationX += rawX; calibrationY += rawY; ++calibrationSamples;
    if (now - phaseAt >= 1000 && calibrationSamples) {
      centreX = calibrationX / calibrationSamples; centreY = calibrationY / calibrationSamples;
      randomSeed(uint32_t(micros()) ^ (uint32_t(rawX) << 16) ^ uint32_t(rawY));
      game.reset(uint16_t(random(65536L)));
      phase = READY; phaseAt = now; 
      Serial.print(F("CENTRE X=")); Serial.print(centreX);
      Serial.print(F(" Y=")); Serial.println(centreY);
      Serial.println(F("READY. Check j input labels. Press START or send s to play."));
    }
    return;
  }
  int dx = rawX - centreX, dy = rawY - centreY;
  if (kReverseX) dx = -dx;
  if (kReverseY) dy = -dy;
  const int threshold = stickDirection == SnakeGame::NONE ? kEngage : kRelease;
  SnakeGame::Direction next = SnakeGame::NONE;
  if (abs(dx) >= threshold || abs(dy) >= threshold) {
    const long nx = long(abs(dx)) * 1000 / (dx >= 0 ? kXPositiveSpan : kXNegativeSpan);
    const long ny = long(abs(dy)) * 1000 / (dy >= 0 ? kYPositiveSpan : kYNegativeSpan);
    // Each candidate must itself pass the threshold. Only one direction is sent.
    if (abs(dx) >= threshold && (abs(dy) < threshold || nx >= ny))
      next = dx > 0 ? SnakeGame::LEFT : SnakeGame::RIGHT;
    else next = dy > 0 ? SnakeGame::UP : SnakeGame::DOWN;
  }
  stickDirection = next;
  if (phase == PLAYING) game.turn(next);
  if (reportInputs && now - lastReport >= 200) {
    lastReport = now;
    Serial.print(F("X=")); Serial.print(rawX); Serial.print(F(" Y=")); Serial.print(rawY);
    Serial.print(F(" dX=")); Serial.print(dx); Serial.print(F(" dY=")); Serial.print(dy);
    Serial.print(' '); reportDirection(next); Serial.println();
  }
}
void readStartButton(unsigned long now) {
  const bool reading = digitalRead(kStartPin);
  if (reading != buttonRaw) { buttonRaw = reading; buttonChangedAt = now; }
  if (now - buttonChangedAt < kButtonDebounceMs || buttonStable == buttonRaw) return;
  buttonStable = buttonRaw;
  if (buttonStable == HIGH) buttonArmed = true;
  else {
    // One event per press. A held/bouncing button cannot restart a round.
    const bool accepted = buttonArmed;
    buttonArmed = false;
    if (accepted && (phase == READY || phase == FINISHED || phase == BAR_TEST)) startGame();
  }
}
void printBar() {
  Serial.print(F("BAR digit=")); Serial.print(testCell / 7 + 1);
  Serial.print(F(" from left, segment=")); Serial.print(char('a' + testCell % 7));
  Serial.print(F(" raw column=")); Serial.println(kDigitColumns[testCell / 7]);
}
void commands() {
  while (Serial.available()) {
    const char c = char(Serial.read());
    if (c == '\r' || c == '\n' || c == ' ') continue;
    switch (c) {
      case 's': startGame(); break;
      case 'c': beginCalibration(); break;
      case 'j': reportInputs = !reportInputs;
        Serial.println(reportInputs ? F("Input reports ON.") : F("Input reports OFF.")); break;
      case 'm':
        if (phase == CALIBRATING) { Serial.println(F("Wait for READY.")); break; }
        phase = BAR_TEST; testCell = 0; printBar(); break;
      case 'n':
        if (phase != BAR_TEST) Serial.println(F("First send m for the bar test, then n."));
        else { testCell = (testCell + 1) % 49; printBar(); }
        break;
      case 'w': game.turn(SnakeGame::UP); break;
      case 'a': game.turn(SnakeGame::LEFT); break;
      case 'd': game.turn(SnakeGame::RIGHT); break;
      case 'z': game.turn(SnakeGame::DOWN); break;
      case '?':
        Serial.println(F("s=start/restart c=calibrate j=inputs m=bar test n=next bar w/a/d/z=steer"));
        break;
      default: Serial.println(F("Unknown key; send ? for help.")); break;
    }
  }
}
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  cli();
  // Preserve legacy startup/Timer1 while isolating gameplay above it.
  TCCR1A = 0; TCCR1B = 0; TCNT1 = 0; OCR1A = 62498;
  TCCR1B |= (1 << WGM12); TCCR1B |= (1 << CS12); TIMSK1 |= (1 << OCIE1A);
  CLKPR = 0x80;
  DDRD = 0xFF; PORTD = 0; DDRB = 0xFF; PORTB = 0;
  pinMode(0, INPUT); // USB serial RX, as in ManualMap.
  pinMode(kXPin, INPUT); pinMode(kYPin, INPUT); analogReference(DEFAULT);
  pinMode(kStartPin, INPUT_PULLUP);
  buttonRaw = buttonStable = digitalRead(kStartPin);
  buttonArmed = buttonStable == HIGH; // Held at boot: release before first press.
  buttonChangedAt = millis();
  initializePanel(); clearPanel(); sei();
  Serial.println(F("PanelSnake FIRST BENCH TEST. A0=X A1=Y START=A2-GND; Z unused; optional buzzer disabled."));
  Serial.println(F("s=start c=calibrate j=inputs m=bar test n=next bar ?=help"));
  beginCalibration();
}
void loop() {
  unsigned long now = millis();
  commands();
  now = millis(); // Commands may start a new calibration/time epoch.
  if (now - lastInput >= kInputMs) { lastInput = now; readStartButton(now); readStick(now); }
  if (phase == PLAYING && now - lastMove >= kMoveMs) {
    lastMove = now; // No catch-up burst if serial or a frame took longer.
    const SnakeGame::Result result = game.step(uint16_t(random(65536L)));
    if (result == SnakeGame::ATE) {
      Serial.print(F("FOOD score=")); Serial.println(game.score); beep(1400,60);
    } else if (result == SnakeGame::DIED || result == SnakeGame::WON) {
      phase = DYING; phaseAt = now; beep(250,180);
      Serial.print(result == SnakeGame::WON ? F("BOARD COMPLETE score=") : F("DEAD score="));
      Serial.println(game.score);
    }
  }
  if (phase == DYING && now - phaseAt >= 1600) {
    phase = FINISHED; phaseAt = now;
    Serial.print(F("GAME OVER score=")); Serial.println(game.score);
    Serial.println(F("Press START (A2-GND) or send s to restart."));
  }
  if (now - lastDraw >= kDrawMs) { lastDraw = now; draw(now); }
}
ISR(TIMER1_COMPA_vect) { ++legacySeconds; legacyFlag = !legacyFlag; }


