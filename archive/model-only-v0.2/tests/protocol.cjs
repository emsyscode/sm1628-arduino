const assert = require('node:assert/strict');
const {cpu,transactions} = require('./run-avr.cjs')(process.argv[2],process.argv[3]);
const blank = Array(14).fill(0);
const raw = Array.from({length: 14}, (_, i) => (0x81 ^ (i * 13)) & 255);
const glyph = Array(14).fill(0);
glyph[0] = 4; glyph[5] = 64; glyph[6] = 0xA5;
assert.deepEqual(transactions, [
  [0x80], [0x03], [0x40], [0xC0, ...blank], [0x88],
  [0x40], [0xC0, ...raw], [0x8F], [0x87], [0x82], [0x8A],
  [0x40], [0xC0, ...blank],
  [0x40], [0xC0, ...glyph],
  [0x40], [0xC0, ...glyph],
  [0x40], [0xC0, ...glyph],
]);
console.log(JSON.stringify({
  result: 'PASS', transactions: transactions.length,
  simulated_ms: cpu.cycles / 16000,
  checks: ['LSB-first bytes and STB framing', 'blank before enable',
    'complete 14-byte frame and clear', 'brightness preserves blanking',
    'invalid calls send no transactions', 'scattered glyph mapping',
    'invalid maps are atomic', 'unrelated outputs preserved'],
  limit: 'Compiled AVR GPIO only; no SM1628 IC, electrical or optical model.'
}, null, 2));
