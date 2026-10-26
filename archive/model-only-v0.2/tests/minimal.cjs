const assert = require('node:assert/strict');
const {transactions} = require('./run-avr.cjs')(process.argv[2],process.argv[3],false);
const blank = Array(14).fill(0), lit = [...blank]; lit[0]=1;
assert.deepEqual(transactions,[
  [0x80],[3],[0x40],[0xC0,...blank],[0x88],
  [0x40],[0xC0,...lit],[0x40],[0xC0,...blank]
]);
console.log('PASS: standalone initialization and both 500 ms blink transitions.');
