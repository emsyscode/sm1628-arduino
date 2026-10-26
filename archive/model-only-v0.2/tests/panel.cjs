const assert = require('node:assert/strict');
const reference = require('./legacy-reference.json');
const {cpu,transactions} = require('./run-avr.cjs')(process.argv[2],process.argv[3]);
const zero = () => Array(14).fill(0);
const expected = [[0x80],[3],[0x40],[0xC0,...zero()],[0x88]];
function send(frame) { expected.push([0x40],[0xC0,...frame]); }
// Independent reference: original numeral masks shifted into the legacy words.
function legacy(values) {
  const out = zero();
  values.forEach((n,d) => {
    if (n === null) return;
    const column = reference.clockOrderGridBits[d];
    for (let row=0; row<7; row++) if (reference.numeralMasks[n] & (1<<row))
      out[row*2+Math.floor(column/8)] |= 1<<(column%8);
  });
  return out;
}
for (let d=0; d<6; d++) for (let n=0; n<10; n++) {
  const digits = Array(6).fill(null); digits[d]=n; send(legacy(digits));
}
send(legacy([null,null,null,null,null,0]));
send(legacy([null,null,null,null,4,2]));
send(legacy([0,0,0,0,4,2]));
const withIcon = legacy([1,2,3,4,5,6]); withIcon[13] |= 128; send(withIcon);
const onlyIcon = zero(); onlyIcon[13]=128; send(onlyIcon);
send(zero()); send(zero()); send(zero());
expected.push([0x40],[0xCD,0xAA]); // Last RAM byte, valid bounded write.
send(zero());
const seven = zero();
for(let d=0;d<7;d++)for(let s=0;s<7;s++)if(reference.numeralMasks[9] & (1<<s)){
  const bit=d*7+s;seven[Math.floor(bit/8)]|=1<<(bit%8);
}
send([...seven]); seven[0]&=254; send(seven);
assert.deepEqual(transactions,expected);
console.log(JSON.stringify({result:'PASS',transactions:transactions.length,
  simulated_ms:cpu.cycles/16000,
  checks:['60 original glyph/position combinations','map copying and alias rejection',
    'number width, zero, padding and overflow','independent icon preservation',
    'no-op flush suppression','force/invalidate restoration','bounded RAM writes',
    '7-digit maximum and 32-bit overflow'],
  scope:'Compiled AVR software only; board profiles need bench confirmation.'},null,2));
