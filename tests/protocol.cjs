// Decode the actual compiled Uno GPIO stream, not a JavaScript driver copy.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const avr = require(process.argv[3] ? path.resolve(process.argv[3]) : 'avr8js');
const flash = new Uint8Array(32768);
let base = 0;
for (const line of fs.readFileSync(process.argv[2], 'utf8').trim().split(/\r?\n/)) {
  const r = Buffer.from(line.slice(1), 'hex');
  assert.equal([...r].reduce((sum, b) => sum + b, 0) % 256, 0);
  if (r[3] === 0) flash.set(r.subarray(4, 4 + r[0]), base + r.readUInt16BE(1));
  if (r[3] === 4) base = r.readUInt16BE(4) * 65536;
}
const cpu = new avr.CPU(new Uint16Array(flash.buffer));
new avr.AVRTimer(cpu, avr.timer0Config);
const portB = new avr.AVRIOPort(cpu, avr.portBConfig);
new avr.AVRIOPort(cpu, avr.portCConfig);
new avr.AVRIOPort(cpu, avr.portDConfig);
new avr.AVRUSART(cpu, avr.usart0Config, 16e6);
let bits = null;
const transactions = [];
portB.addListener((value, old) => {
  if ((old & 2) && !(value & 2)) bits = []; // D9 STB falls.
  if (bits && !(old & 1) && (value & 1)) // D8 CLK rises.
    bits.push((cpu.data[0x2B] & 128) ? 1 : 0); // D7 DATA.
  if (bits && !(old & 2) && (value & 2)) {
    assert.equal(bits.length % 8, 0, 'partial byte under STB');
    const bytes = [];
    for (let i = 0; i < bits.length; i += 8)
      bytes.push(bits.slice(i, i + 8).reduce((v, b, bit) => v | (b << bit), 0));
    transactions.push(bytes);
    bits = null;
  }
});
while (!(cpu.data[0x2B] & 64) && cpu.cycles < 1600000) {
  avr.avrInstruction(cpu);
  cpu.tick();
}
assert.equal(cpu.data[0x2B] & 32, 0, 'firmware assertion failed');
assert(cpu.data[0x2B] & 64, 'firmware did not complete');
assert.equal(bits, null, 'STB left active');
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
