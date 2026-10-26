#pragma once
#include <stdint.h>

// Same segment/junction rules as the browser prototype:
// seven digits, seven body/food cells per digit; invisible gaps are skipped.
struct SnakeGame {
  enum Direction : uint8_t { UP, RIGHT, DOWN, LEFT, NONE };
  enum Result : uint8_t { MOVED, ATE, DIED, WON };
  uint8_t body[49], length = 3, food = 0, score = 0;
  int8_t headX = 5, headY = 0;
  Direction direction = RIGHT, pending = NONE;
  bool dead = false, won = false;

  bool occupied(uint8_t cell) const {
    for (uint8_t i = 0; i < length; ++i) if (body[i] == cell) return true;
    return false;
  }
  // Caller supplies randomness; choosing from free cells always terminates.
  void placeFood(uint16_t randomValue) {
    const uint8_t free = 49 - length;
    if (!free) { won = true; return; }
    uint8_t choice = randomValue % free;
    for (uint8_t cell = 0; cell < 49; ++cell)
      if (!occupied(cell) && choice-- == 0) { food = cell; return; }
  }
  void reset(uint16_t randomValue) {
    length = 3; score = 0; headX = 5; headY = 0;
    body[0] = 14; body[1] = 7; body[2] = 0;
    direction = RIGHT; pending = NONE; dead = won = false;
    placeFood(randomValue);
  }
  void turn(Direction requested) {
    // One queued turn per movement tick prevents two fast turns reversing
    // into the body before the snake has actually moved.
    if (dead || won || requested == NONE || pending != NONE ||
        requested == direction || ((requested + 2) % 4) == direction) return;
    pending = requested;
  }
  static int8_t edgeCell(int8_t x, int8_t y, int8_t nx, int8_t ny) {
    if (x < 0 || nx < 0 || x > 13 || nx > 13 ||
        y < 0 || ny < 0 || y > 2 || ny > 2) return -1;
    if (y == ny) {
      const int8_t left = x < nx ? x : nx;
      if (left % 2) return -1; // An invisible gap cannot hold food/body.
      const uint8_t segment = y == 0 ? 0 : (y == 1 ? 6 : 3);
      return (left / 2) * 7 + segment;
    }
    const int8_t top = y < ny ? y : ny;
    const uint8_t segment = x % 2 ? (top == 0 ? 1 : 2) : (top == 0 ? 5 : 4);
    return (x / 2) * 7 + segment;
  }
  Result step(uint16_t randomValue) {
    if (dead) return DIED;
    if (won) return WON;
    if (pending != NONE) direction = pending;
    pending = NONE;
    const int8_t dx = direction == RIGHT ? 1 : (direction == LEFT ? -1 : 0);
    const int8_t dy = direction == DOWN ? 1 : (direction == UP ? -1 : 0);
    int8_t x = headX, y = headY;
    if (dx && ((dx == 1 && x % 2) || (dx == -1 && !(x % 2)))) x += dx;
    const int8_t nx = x + dx, ny = y + dy;
    const int8_t next = edgeCell(x, y, nx, ny);
    const bool eating = next == food;
    // Moving into the departing tail is allowed when not growing.
    const uint8_t checked = length - (eating ? 0 : 1);
    bool collision = next < 0;
    for (uint8_t i = 0; i < checked; ++i) if (body[i] == next) collision = true;
    if (collision) { dead = true; return DIED; } // Frozen body/head.
    if (eating) ++length;
    for (uint8_t i = length - 1; i > 0; --i) body[i] = body[i - 1];
    body[0] = uint8_t(next); headX = nx; headY = ny;
    if (eating) { ++score; placeFood(randomValue); return won ? WON : ATE; }
    return MOVED;
  }
};
