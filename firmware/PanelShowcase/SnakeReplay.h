#pragma once
#include <stdint.h>
#include "SnakeGame.h"

// A rehearsed round: genuine game steps, with repeatable turns and food.
struct ReplayLeg {
  SnakeGame::Direction direction;
  uint8_t moves;
};

static const ReplayLeg kReplayLegs[] = {
    {SnakeGame::RIGHT, 4}, {SnakeGame::DOWN, 2}, {SnakeGame::LEFT, 7}, {SnakeGame::UP, 2},
    {SnakeGame::RIGHT, 4}, {SnakeGame::DOWN, 1}, {SnakeGame::LEFT, 4}, {SnakeGame::DOWN, 1},
    {SnakeGame::RIGHT, 5}, {SnakeGame::UP, 2},   {SnakeGame::LEFT, 6},
};

static const uint8_t kReplayFood[] = {35, 45, 3, 14, 13, 29, 0, 8};

struct SnakeReplay {
  enum Phase : uint8_t {
    INTRO,
    PLAYING,
    DEATH,
    GAME_OVER,
    FINISHED,
    BETWEEN
  };

  enum Event : uint8_t {
    QUIET,
    FOOD,
    COLLISION,
    DONE,
    RESTARTED
  };

  SnakeGame game;
  Phase phase = INTRO;
  bool repeat = false;
  uint16_t moveIntervalMs = 500;
  uint32_t startedAt = 0;
  uint32_t phaseAt = 0;
  uint32_t lastMoveAt = 0;
  uint8_t legIndex = 0;
  uint8_t legMove = 0;
  uint8_t foodIndex = 0;

  void begin(uint32_t now, bool looping, uint16_t pace) {
    repeat = looping;
    moveIntervalMs = pace;
    startedAt = phaseAt = lastMoveAt = now;
    legIndex = legMove = foodIndex = 0;
    phase = INTRO;

    game.reset(0);
    game.food = kReplayFood[0];
  }

  Event update(uint32_t now) {
    switch (phase) {
      case INTRO:
        if (now - phaseAt >= 1000) {
          phase = PLAYING;
          phaseAt = lastMoveAt = now;
        }
        break;

      case PLAYING:
        if (now - lastMoveAt >= moveIntervalMs) {
          lastMoveAt = now;
          const uint8_t legCount = sizeof(kReplayLegs) / sizeof(kReplayLegs[0]);

          // Stop safely if a future route edit ends without a collision.
          if (legIndex >= legCount) {
            game.dead = true;
            phase = DEATH;
            phaseAt = now;
            return COLLISION;
          }

          game.turn(kReplayLegs[legIndex].direction);
          const SnakeGame::Result result = game.step(0);

          if (++legMove == kReplayLegs[legIndex].moves) {
            ++legIndex;
            legMove = 0;
          }

          if (result == SnakeGame::DIED || result == SnakeGame::WON) {
            phase = DEATH;
            phaseAt = now;
            return COLLISION;
          }

          if (result == SnakeGame::ATE) {
            ++foodIndex;
            if (foodIndex < sizeof(kReplayFood)) {
              game.food = kReplayFood[foodIndex];
            }
            return FOOD;
          }
        }
        break;

      case DEATH:
        if (now - phaseAt >= 1600) {
          phase = GAME_OVER;
          phaseAt = now;
        }
        break;

      case GAME_OVER:
        if (now - phaseAt >= 4800) {
          phase = repeat ? BETWEEN : FINISHED;
          phaseAt = now;
          if (!repeat) {
            return DONE;
          }
        }
        break;

      case BETWEEN:
        if (now - phaseAt >= 1000) {
          begin(now, true, moveIntervalMs);
          return RESTARTED;
        }
        break;

      case FINISHED:
        break;
    }

    return QUIET;
  }

  void drawCell(uint8_t* frame, const uint8_t* columns, uint8_t cell) const {
    if (cell < 49) {
      frame[(cell % 7) * 2] |= uint8_t(1u << columns[cell / 7]);
    }
  }

  void drawGlyph(uint8_t* frame, const uint8_t* columns, uint8_t digit, uint8_t mask) const {
    for (uint8_t segment = 0; segment < 7; ++segment) {
      if (mask & (1u << segment)) {
        drawCell(frame, columns, digit * 7 + segment);
      }
    }
  }

  void render(uint8_t* frame, const uint8_t* columns, uint32_t now) const {
    // The caller clears the complete frame first. Symbols remain off.
    if (phase == INTRO || phase == PLAYING || phase == DEATH) {
      const bool showBody = phase != DEATH || ((now - phaseAt) / 160) % 2 == 0;

      if (showBody) {
        for (uint8_t i = 0; i < game.length; ++i) {
          drawCell(frame, columns, game.body[i]);
        }
      }

      if (phase != DEATH && ((now - startedAt) / 250) % 2 == 0) {
        drawCell(frame, columns, game.food);
      }
    } else if (phase == GAME_OVER || phase == FINISHED) {
      const uint8_t screen = phase == FINISHED ? 2 : (now - phaseAt) / 1200;

      if (screen == 0) {
        // Same seven-segment GAME approximation as PanelSnake.
        drawGlyph(frame, columns, 1, 0x3D);
        drawGlyph(frame, columns, 2, 0x77);
        drawGlyph(frame, columns, 3, 0x54);
        drawGlyph(frame, columns, 4, 0x79);
      } else if (screen == 1) {
        drawGlyph(frame, columns, 1, 0x3F);
        drawGlyph(frame, columns, 2, 0x1C);
        drawGlyph(frame, columns, 3, 0x79);
        drawGlyph(frame, columns, 4, 0x50);
      } else {
        const uint8_t numerals[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};
        uint8_t score = game.score;
        uint8_t digit = 6;

        do {
          drawGlyph(frame, columns, digit--, numerals[score % 10]);
          score /= 10;
        } while (score);
      }
    }
  }
};

