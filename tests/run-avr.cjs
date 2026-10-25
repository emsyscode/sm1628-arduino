const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');

module.exports = function run(hex, modulePath, stopOnCompletion = true) {
  const avr = require(modulePath ? path.resolve(modulePath) : 'avr8js');
  const flash = new Uint8Array(32768);
  let base = 0;
  for (const line of fs.readFileSync(hex, 'utf8').trim().split(/\r?\n/)) {
    const r = Buffer.from(line.slice(1), 'hex');
    assert.equal([...r].reduce((sum, b) => sum + b, 0) % 256, 0);
    if (r[3] === 0) flash.set(r.subarray(4,4+r[0]), base+r.readUInt16BE(1));
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
    if ((old & 2) && !(value & 2)) bits = [];
    if (bits && !(old & 1) && (value & 1))
      bits.push((cpu.data[0x2B] & 128) ? 1 : 0);
    if (bits && !(old & 2) && (value & 2)) {
      assert.equal(bits.length % 8, 0, 'partial byte under STB');
      const bytes = [];
      for (let i=0; i<bits.length; i+=8)
        bytes.push(bits.slice(i,i+8).reduce((v,b,bit) => v | (b<<bit),0));
      transactions.push(bytes);
      bits = null;
    }
  });
  const limit = stopOnCompletion ? 16000000 : 17600000; // 1 s / 1.1 s.
  while ((!stopOnCompletion || !(cpu.data[0x2B] & 64)) && cpu.cycles < limit) {
    avr.avrInstruction(cpu); cpu.tick();
  }
  assert.equal(bits, null, 'STB left active');
  assert.equal(cpu.data[0x2B] & 32, 0, 'firmware assertion failed');
  if (stopOnCompletion) assert(cpu.data[0x2B] & 64, 'fixture timed out');
  return {cpu, transactions};
};
