// Rehearsed route fixture checked against the original browser game model.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const avr = require(process.argv[3] ? path.resolve(process.argv[3]) : 'avr8js');
const root = path.resolve(__dirname, '..');

assert.equal(
  fs.readFileSync(path.join(root, 'firmware/PanelShowcase/SnakeGame.h'), 'utf8').replace(/\r\n/g, '\n').trimEnd(),
  fs.readFileSync(path.join(root, 'firmware/PanelSnake/SnakeGame.h'), 'utf8').replace(/\r\n/g, '\n').trimEnd(),
  'Self-contained sketch folders must use identical game rules'
);

const m = require('./avr-panel.cjs')(avr)(process.argv[2], true);
const columns = [0, 1, 7, 2, 3, 4, 5];
const route = [
  21, 28, 35, 42, 43, 44, 45, 38, 31, 24, 17, 10, 3, 4, 5, 0, 7, 14, 21,
  22, 27, 20, 13, 6, 4, 3, 10, 17, 24, 31, 30, 29, 28, 21, 14, 7, 0
];
const food = [35, 45, 3, 14, 13, 29, 0, 8];

function cellsFrame(cells) {
  const frame = Array(14).fill(0);
  for (const cell of cells) {
    frame[(cell % 7) * 2] |= 1 << columns[Math.floor(cell / 7)];
  }
  return frame;
}

function glyphFrame(items) {
  const cells = [];
  for (const [digit, mask] of items) {
    for (let segment = 0; segment < 7; ++segment) {
      if (mask & (1 << segment)) cells.push(digit * 7 + segment);
    }
  }
  return cellsFrame(cells);
}

const same = (a, b) => a.every((v, i) => v === b[i]);
m.run(400);
m.send('pace 50', 20);
m.send('snake', 20);
const first = m.frames.length - 1;
assert.deepEqual(m.frames[first].data, cellsFrame([14, 7, 0, 35]));
m.run(9600);
const output = m.output;
assert.equal((output.match(/REPLAY FOOD score=/g) || []).length, 7);
assert(output.includes('REPLAY COLLISION score=7'));
assert(output.includes('SNAKE REPLAY DONE score=7'));

const frames = m.frames.slice(first);
let body = [14, 7, 0];
let foodIndex = 0;
const states = [{ body: [...body], food: food[0] }];

for (const cell of route) {
  const eating = cell === food[foodIndex];
  assert(!body.slice(0, eating ? undefined : -1).includes(cell), 'valid fixture');
  body.unshift(cell);
  if (eating) ++foodIndex;
  else body.pop();
  assert(!body.includes(food[foodIndex]), 'food is free');
  states.push({ body: [...body], food: food[foodIndex] });
}

assert.equal(body.length, 10);
assert.equal(foodIndex, 7);
const deathFrames = frames.filter(f => f.ms > frames[0].ms + 3050 && f.ms < frames[0].ms + 4300);
const frozenBody = cellsFrame(body);
assert(deathFrames.some(f => f.data.every(v => v === 0)), 'death blink includes blank frames');
assert(deathFrames.some(f => same(f.data, frozenBody)), 'death blink shows final body');
for (const f of deathFrames) assert(f.data.every(v => v === 0) || same(f.data, frozenBody), 'death body stays frozen');
for (const state of states) {
  const solid = cellsFrame(state.body);
  const withFood = cellsFrame([...state.body, state.food]);
  assert(frames.some(f => same(f.data, solid) || same(f.data, withFood)),
    'Every legal body state must actually reach the GPIO renderer');
}

const gameOver = glyphFrame([[1, 0x3D], [2, 0x77], [3, 0x54], [4, 0x79]]);
const over = glyphFrame([[1, 0x3F], [2, 0x1C], [3, 0x79], [4, 0x50]]);
assert(frames.some(f => same(f.data, gameOver)), 'GAME frame');
assert(frames.some(f => same(f.data, over)), 'OVER frame');
assert.deepEqual(m.frames.at(-1).data, glyphFrame([[0, 0x6D], [1, 0x39], [2, 0x3F], [3, 0x50], [4, 0x79], [6, 7]]), 'final score7');
const finishedFrames = m.frames.length;
m.run(500);
assert.equal(m.frames.length, finishedFrames, 'one round holds final score');

m.send('snake-loop', 20);
const loopStart = m.output.length;
m.run(12000);
assert(m.output.slice(loopStart).includes('SNAKE REPLAY RESTART.'));
m.send('stop', 20);
assert.deepEqual(m.frames.at(-1).data, Array(14).fill(0));
const stopped = m.frames.length;
m.run(1000);
assert.equal(m.frames.length, stopped, 'stop cancels replay scheduler');

m.send('snake', 20);
m.run(1500);
m.send('hold', 20);
const frozen = m.frames.length;
m.run(1500);
assert.equal(m.frames.length, frozen, 'hold freezes gameplay and blink');

m.send('snake', 20);
assert.deepEqual(m.frames.at(-1).data, cellsFrame([14, 7, 0, 35]), 'repeatable initial state');
m.send('pace 100', 20);
const changedAt = m.frames.length;
m.run(600);
const periods = m.frames.slice(changedAt + 1).map((f, i) => f.ms - m.frames[changedAt + i].ms);
assert(periods.length > 5 && Math.max(...periods) < 50, 'render clock stays independent of pace');
m.send('stop', 20);

for (const f of m.frames) {
  for (let row = 0; row < 7; ++row) {
    assert.equal(f.data[row * 2 + 1], 0);
    assert.equal(f.data[row * 2] & 64, 0, 'symbol groups stay off');
  }
}
assert.equal(m.buzzerEdges.filter(e => e.on).length, 0, 'filming remains silent');
console.log('PASS: shared rules, all38 body states, seven foods/growth, collision, GAME/OVER/score, repeatable round, looping, hold/stop, independent pace/blinking and numeric-only silent output.');
