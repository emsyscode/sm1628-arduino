/*
 ManualMap - interactive Uno diagnostic, based on Jose Luis Monteiro's panel test.

 HOW TO USE
 1. Use the tested panel wiring: D7 DATA, D8 CLK, D9 STB, common ground.
    Keep the tested supply arrangement. D1..D13 retain legacy output setup:
    disconnect joystick/other peripherals on those pins. D0 is serial input.
 2. Upload for Arduino Uno. Open Serial Monitor: 9600 baud, Newline (or Both).
 3. It starts BLANK. First send: column 0 7
    Expect a 7 on the LEFTMOST numeric digit of this particular panel.
 4. Then send n (next) or p (previous). Each selection holds until changed.
    Raw column order is NOT physical digit order; it also reaches symbols.
 5. For digits left to right, send: ordered
    This starts a fresh sequence with numeral 7, using the photo-derived map.
    n/p now steps only the seven numeric positions; it wraps at each end.
    Send column 0 7 to return to the original raw-column diagnostic.

 WHY 7? It matches the photographed mapping test (top + two right bars).
 digit 8 is an optional all-bars check AFTER selecting a mode; it is not
 evidence that all bars are already verified. Other panels need their own map.
 See docs/manual-mapping.md for expected results and troubleshooting.
*/
// Changes are limited to manual frame selection, serial RX and command parsing.
// PanelDemo and the untouched original remain separate hardware references.
#include <Arduino.h>
#include <avr/interrupt.h>
#include <ctype.h>
#include <string.h>

constexpr uint8_t kDataPin = 7;
constexpr uint8_t kClockPin = 8;
constexpr uint8_t kStrobePin = 9;
constexpr uint8_t kDisplayMode = 0x03;
constexpr uint8_t kFullBrightness = 0x8F;
constexpr uint8_t kFrameBytes = 14;

// Literal patterns from the working sketch, including its second-byte bits.
// Each pair corresponds to a,b,c,d,e,f,g in source order.
// Do not replace the second bytes with zeros or use a generic numeral font.
const uint8_t kPatterns[10][kFrameBytes] = {
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000010}, // 0
  {0b00000000, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010}, // 1
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010}, // 2
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010}, // 3
  {0b00000000, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010}, // 4
  {0b00000001, 0b00000010, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010}, // 5
  {0b00000001, 0b00000010, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010}, // 6
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010}, // 7
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010}, // 8
  {0b00000001, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b00000010, 0b00000000, 0b00000010, 0b00000000, 0b00000010, 0b00000001, 0b00000010, 0b00000001, 0b00000010}, // 9
};

// The original enabled Timer1 even though its active display test does not
// use the clock values. Keep that setup for this first behavior comparison.
volatile uint8_t legacySeconds = 0;
volatile bool legacyFlag = true;

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

// Manual state: no automatic test loop and no heap allocation.
// Photo-derived numeric positions, left to right; not engineer Grid labels.
constexpr uint8_t kDigitColumns[] = {0, 1, 7, 2, 3, 4, 5};
constexpr uint8_t kDigitCount = sizeof(kDigitColumns);
enum Selection { NONE, SLOT, COLUMN, ORDERED, SINGLE_BIT, CUSTOM };
Selection selected = NONE;
uint8_t selectedIndex = 0;
uint8_t numeral = 7;
uint8_t frame[kFrameBytes] = {0};
char line[48];
uint8_t lineLength = 0;
bool discardLine = false;

void sendFrame() {
  // Every command updates all fourteen bytes, including zeros elsewhere.
  delayMicroseconds(1);
  sendCommand(kDisplayMode);
  sendCommand(0x80);
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(1);
  sendBurstByte(0xC0);
  for (uint8_t i = 0; i < kFrameBytes; ++i) sendBurstByte(frame[i]);
  digitalWrite(kStrobePin, HIGH);
  sendCommand(kFullBrightness);
}

void zeroFrame() {
  for (uint8_t i = 0; i < kFrameBytes; ++i) frame[i] = 0;
}

void printStatus() {
  Serial.print(F("mode="));
  switch (selected) {
    case SLOT: Serial.print(F("original-slot")); break;
    case COLUMN: Serial.print(F("raw-column")); break;
    case ORDERED: Serial.print(F("ordered-digits")); break;
    case SINGLE_BIT: Serial.print(F("single-bit")); break;
    case CUSTOM: Serial.print(F("custom-frame")); break;
    default: Serial.print(F("blank")); break;
  }
  Serial.print(F(" index=")); Serial.print(selectedIndex);
  Serial.print(F(" numeral=")); Serial.println(numeral);
  if (selected == COLUMN || selected == ORDERED) {
    const uint8_t column = selected == ORDERED ? kDigitColumns[selectedIndex] : selectedIndex;
    Serial.print(F("Raw column ")); Serial.print(column);
    Serial.print(F(": expected on this panel: "));
    int8_t position = -1;
    for (uint8_t i = 0; i < kDigitCount; ++i)
      if (kDigitColumns[i] == column) position = i;
    if (position >= 0) {
      Serial.print(F("numeric position ")); Serial.print(position + 1);
      Serial.print(F(" from left, numeral ")); Serial.println(numeral);
    } else if (column == 6) {
      Serial.println(F("return/colons/Dolby/DTS group (lit icons depend on numeral)."));
    } else if (column == 8) {
      Serial.println(F("wheel group (lit wedges depend on numeral)."));
    } else if (column == 9) {
      Serial.println(F("DVD/VCD/MP3/playback group (lit icons depend on numeral)."));
    } else Serial.println(F("unmapped storage column; no visible result established."));
  }
}

void printFrame() {
  Serial.print(F("RAM 00..0D:"));
  for (uint8_t i = 0; i < kFrameBytes; ++i) {
    Serial.print(' ');
    if (frame[i] < 16) Serial.print('0');
    Serial.print(frame[i], HEX);
  }
  Serial.println();
}

void printHelp() {
  Serial.println(F("Manual panel diagnostic - 9600 baud, Newline or Both."));
  Serial.println(F("START: column 0 7 -> leftmost digit shows 7. THEN send n."));
  Serial.println(F("ordered      Start left-to-right numeric test with 7; n/p steps."));
  Serial.println(F("n/p need a selection first; clear/reset removes it."));
  Serial.println(F("slot N [D]   Original slot 0..7, optional numeral 0..9."));
  Serial.println(F("column C [D] Raw column 0..15, no extra symbol bits."));
  Serial.println(F("digit D      Change numeral on selected slot/column/ordered mode."));
  Serial.println(F("bit B I      Isolate RAM byte B (0..13), bit I (0..7)."));
  Serial.println(F("set B I V    Edit one current RAM bit; V=0 or 1."));
  Serial.println(F("next / n     Next slot, raw column or isolated bit."));
  Serial.println(F("prev / p     Previous slot, raw column or isolated bit."));
  Serial.println(F("clear / c    Blank and hold. show resends current frame."));
  Serial.println(F("frame / f    Print all fourteen bytes. status prints mode."));
  Serial.println(F("help / ?     This menu. Display holds until next command."));
  Serial.println(F("Raw columns are RAM bit positions, NOT father's Grid labels."));
}

void renderSelection() {
  zeroFrame();
  if (selected == SLOT) {
    // Exact original masks, including second-byte symbol bits.
    for (uint8_t i = 0; i < kFrameBytes; ++i)
      frame[i] = uint8_t(uint16_t(kPatterns[numeral][i]) << selectedIndex);
  } else if (selected == COLUMN || selected == ORDERED) {
    // Same a..g numeral shape, but only ONE raw column is selected.
    // 0..7 = first byte of each pair; 8..15 = second byte.
    // The range describes RAM storage, not sixteen supported physical digits.
    const uint8_t column = selected == ORDERED ? kDigitColumns[selectedIndex] : selectedIndex;
    const uint8_t byteOffset = column / 8;
    const uint8_t mask = uint8_t(1u << (column % 8));
    for (uint8_t row = 0; row < 7; ++row)
      if (kPatterns[numeral][row * 2] & 1) frame[row * 2 + byteOffset] = mask;
  } else if (selected == SINGLE_BIT) {
    frame[selectedIndex / 8] = uint8_t(1u << (selectedIndex % 8));
  }
  sendFrame();
  printStatus();
}

bool readNumber(const char* text, uint8_t maximum, uint8_t& result) {
  if (!text || !*text) return false;
  uint16_t value = 0;
  for (const char* p = text; *p; ++p) {
    if (*p < '0' || *p > '9') return false;
    value = value * 10 + (*p - '0');
    if (value > maximum) return false; // Stops before a long input can overflow.
  }
  result = uint8_t(value);
  return true;
}

void commandError() {
  Serial.println(F("ERR: invalid command/range; display unchanged. Type help."));
}

void moveSelection(int8_t direction) {
  uint8_t count = 0;
  if (selected == SLOT) count = 8;
  if (selected == COLUMN) count = 16;
  if (selected == ORDERED) count = kDigitCount;
  if (selected == SINGLE_BIT) count = 112;
  if (count == 0) {
    Serial.println(F("No sequence selected. First send column 0 7, or ordered; then n/p."));
    return;
  }
  selectedIndex = uint8_t((int(selectedIndex) + count + direction) % count);
  renderSelection();
}

void executeLine() {
  char* args[5];
  uint8_t count = 0;
  char* token = strtok(line, " \t");
  while (token && count < 5) {
    args[count++] = token;
    token = strtok(nullptr, " \t");
  }
  if (!count) return;
  if (token || count > 4) { commandError(); return; }
  for (char* p = args[0]; *p; ++p) *p = char(tolower(uint8_t(*p)));

  uint8_t index = 0, value = numeral, bit = 0;
  if (!strcmp(args[0], "ordered")) {
    if (count != 1) { commandError(); return; }
    selected = ORDERED; selectedIndex = 0; numeral = 7;
    renderSelection(); return;
  }
  if (!strcmp(args[0], "slot") || !strcmp(args[0], "column")) {
    const bool original = !strcmp(args[0], "slot");
    if ((count != 2 && count != 3) ||
        !readNumber(args[1], original ? 7 : 15, index) ||
        (count == 3 && !readNumber(args[2], 9, value))) {
      commandError(); return;
    }
    selected = original ? SLOT : COLUMN;
    selectedIndex = index; numeral = value;
    renderSelection(); return;
  }
  if (!strcmp(args[0], "digit")) {
    if (count != 2 || (selected != SLOT && selected != COLUMN && selected != ORDERED) ||
        !readNumber(args[1], 9, value)) { commandError(); return; }
    numeral = value; renderSelection(); return;
  }
  if (!strcmp(args[0], "bit")) {
    if (count != 3 || !readNumber(args[1], 13, index) ||
        !readNumber(args[2], 7, bit)) { commandError(); return; }
    selected = SINGLE_BIT; selectedIndex = index * 8 + bit;
    renderSelection(); return;
  }
  if (!strcmp(args[0], "set")) {
    if (count != 4 || !readNumber(args[1], 13, index) ||
        !readNumber(args[2], 7, bit) || !readNumber(args[3], 1, value)) {
      commandError(); return;
    }
    const uint8_t mask = uint8_t(1u << bit);
    if (value) frame[index] |= mask;
    else frame[index] &= uint8_t(~mask);
    selected = CUSTOM;
    sendFrame(); printStatus(); return;
  }
  if (count != 1) { commandError(); return; }
  if (!strcmp(args[0], "next") || !strcmp(args[0], "n")) {
    moveSelection(1); return;
  }
  if (!strcmp(args[0], "prev") || !strcmp(args[0], "p")) {
    moveSelection(-1); return;
  }
  if (!strcmp(args[0], "clear") || !strcmp(args[0], "c")) {
    zeroFrame(); selected = NONE; selectedIndex = 0;
    sendFrame(); printStatus(); return;
  }
  if (!strcmp(args[0], "show")) { sendFrame(); printStatus(); return; }
  if (!strcmp(args[0], "frame") || !strcmp(args[0], "f")) {
    printFrame(); return;
  }
  if (!strcmp(args[0], "status")) { printStatus(); return; }
  if (!strcmp(args[0], "help") || !strcmp(args[0], "?")) {
    printHelp(); return;
  }
  commandError();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);  // Preserved from the working setup; no new serial output.
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 62498;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);

  // Compatibility setup retained exactly from the user-tested sketch.
  // It takes ownership of ALL Uno D0..D13 pins; disconnect other peripherals.
  // Narrowing this to three pinMode calls requires a separate bench test.
  CLKPR = 0x80;
  DDRD = 0xFF; PORTD = 0;
  DDRB = 0xFF; PORTB = 0;
  // Explicit change for manual serial input: restore Uno RX (D0) as input.
  // D1..D13 retain the baseline setup. No joystick pins are configured here.
  pinMode(0, INPUT);
  initializePanel();
  clearPanel();
  sei();
  printHelp();
  Serial.println(F("READY. Blank display. First: column 0 7. Then: n. Or start: ordered."));
}

void loop() {
  while (Serial.available()) {
    const char ch = char(Serial.read());
    if (ch == '\r' || ch == '\n') {
      if (discardLine) Serial.println(F("ERR: line too long; display unchanged."));
      else if (lineLength) { line[lineLength] = '\0'; executeLine(); }
      lineLength = 0; discardLine = false;
    } else if (!discardLine) {
      if (lineLength < sizeof(line) - 1) line[lineLength++] = ch;
      else discardLine = true;
    }
  }
}

ISR(TIMER1_COMPA_vect) {
  ++legacySeconds;
  legacyFlag = !legacyFlag;
}

