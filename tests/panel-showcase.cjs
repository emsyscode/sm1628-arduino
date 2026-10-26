// Execute filming firmware and decode actual14-byte GPIO frames.
const path=require('node:path'),assert=require('node:assert/strict');
const avr=require(process.argv[3]?path.resolve(process.argv[3]):'avr8js');
const m=require('./avr-panel.cjs')(avr)(process.argv[2],true);
const columns=[0,1,7,2,3,4,5];
function expected(items){
 const f=Array(14).fill(0);
 for(const [c,mask] of items)for(let row=0;row<7;row++)if(mask&(1<<row))f[row*2+Math.floor(c/8)]|=1<<(c%8);
 return f;
}
function send(cmd,wait=20){m.send(cmd,wait);}
function frame(items,label){assert.deepEqual(m.frames.at(-1).data,expected(items),label);}
m.run(400);assert(m.output.includes('READY: blank'));
send('digits');frame([[0,127]],'first digit');
for(const c of columns.slice(1)){m.run(500);frame([[c,127]],'numeric sweep');}
send('hold');const heldCount=m.frames.length;m.run(900);assert.equal(m.frames.length,heldCount);
send('stop');frame([],'blank stop');
send('groups');for(const c of [...columns.slice(1),6,8,9]){m.run(500);frame([[c,127]],'ten-group sweep');}
send('all');frame(Array.from({length:10},(_,c)=>[c,127]),'all70 group bits');
send('blink');m.run(500);frame([],'blink off');
send('count');frame(columns.map(c=>[c,63]),'all zeros');m.run(500);frame(columns.map(c=>[c,6]),'all ones');
send('stop');send('pace 50');send('value 9');send('carry');frame([[5,111]],'rightmost9');
m.run(50);frame([[4,6],[5,63]],'carry9 to10');
send('stop');send('value 9999999');send('carry');frame(columns.map(c=>[c,111]),'all nines');
m.run(50);frame([[5,63]],'overflow to0');
send('stop');send('pace 100');send('seed 7');send('sparkle');const sparkle=[...m.frames.at(-1).data];
send('hold');send('sparkle');assert.deepEqual(m.frames.at(-1).data,sparkle,'repeatable seed');
send('hold');const before=m.frames.length;
for(const cmd of ['pace 0','pace 2001','value 10000000','seed 0','blah','all extra','x'.repeat(40)])send(cmd);
assert.equal(m.frames.length,before,'invalid commands do not write display');
send('demo');send('pace 50');assert(m.output.includes('stop/hold demo before changing'));
m.run(48000);assert(m.output.includes('DEMO DONE.'),m.output.slice(-700));frame([],'demo ends blank');
const doneFrames=m.frames.length;m.run(500);assert.equal(m.frames.length,doneFrames);
const loopAt=m.output.length;send('loop');m.run(48000);
const loopOutput=m.output.slice(loopAt);
assert(!loopOutput.includes('DEMO DONE.'));
assert((loopOutput.match(/effect=off step=0/g)||[]).length>=3,'loop wraps through final blank and new pre-roll');
send('stop');frame([],'loop cancelled');const stopped=m.frames.length;m.run(2000);assert.equal(m.frames.length,stopped);
for(const f of m.frames)for(let row=0;row<7;row++)assert.equal(f.data[row*2+1]&252,0,'no undefined columns10..15');
console.log('PASS: numeric/all-group sweeps, all-on/off, blink, sync count, carry/overflow, seeded effects, freeze/stop, invalid commands, bounded full demo and repeated sequence.');
