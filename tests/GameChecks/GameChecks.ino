#include <Arduino.h>
#include "SnakeGame.h"
bool passed = true;
void check(bool condition, uint8_t id) {
  if (!condition) { passed = false; Serial.print(F("FAIL ")); Serial.println(id); }
}
void setup() {
  Serial.begin(115200);
  SnakeGame g; g.reset(0);
  check(g.length == 3 && !g.occupied(g.food), 1);
  g.food = 48; check(g.step(0) == SnakeGame::MOVED && g.body[0] == 21 && g.headX == 7, 2);
  g.reset(0); g.food = 21;
  check(g.step(1) == SnakeGame::ATE && g.length == 4 && g.score == 1 && !g.occupied(g.food), 3);
  g.reset(0); g.food = 48;
  for (uint8_t i = 0; i < 4; ++i) check(g.step(0) == SnakeGame::MOVED, 4);
  uint8_t frozen[49]; memcpy(frozen,g.body,g.length);
  check(g.step(0) == SnakeGame::DIED && g.headX == 13 &&
        !memcmp(frozen,g.body,g.length), 5);
  check(g.step(0) == SnakeGame::DIED && !memcmp(frozen,g.body,g.length), 6);
  g.reset(0); g.turn(SnakeGame::LEFT); check(g.pending == SnakeGame::NONE,7);
  g.turn(SnakeGame::DOWN); g.turn(SnakeGame::LEFT); check(g.pending == SnakeGame::DOWN,8);
  g.reset(0); g.length=4; g.body[0]=14;g.body[1]=15;g.body[2]=16;g.body[3]=17;
  g.food=48;g.turn(SnakeGame::DOWN);check(g.step(0)==SnakeGame::DIED,9);
  g.reset(0);g.length=4;g.body[0]=14;g.body[1]=19;g.body[2]=20;g.body[3]=15;
  g.food=48;g.turn(SnakeGame::DOWN);
  check(g.step(0)==SnakeGame::MOVED && g.body[0]==15 && g.length==4,10);
  g.reset(0);g.length=48;uint8_t at=0;
  for(uint8_t cell=0;cell<49;++cell)if(cell!=21)g.body[at++]=cell;
  g.food=21;check(g.step(0)==SnakeGame::WON && g.length==49 && g.won && g.score==1,11);
  // Every visible edge has one cell; invisible horizontal gap has none.
  check(SnakeGame::edgeCell(0,0,1,0)==0 &&
        SnakeGame::edgeCell(1,0,2,0)==-1 &&
        SnakeGame::edgeCell(13,1,13,2)==44,12);
  Serial.println(passed ? F("ENGINE PASS") : F("ENGINE FAILED"));
}
void loop() {}
