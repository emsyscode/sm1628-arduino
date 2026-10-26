// Compiled Uno gameplay test: ADC input, UART controls and decoded panel frames.
// No electrical/optical model of the real panel.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const avr=require(process.argv[4]?path.resolve(process.argv[4]):'avr8js');
const machine=require('./avr-panel.cjs')(avr);
const checks=machine(process.argv[3]);checks.run(500);
assert(checks.output.includes('ENGINE PASS')&&!checks.output.includes('FAIL'),checks.output);
const m=machine(process.argv[2],true);m.run(1300);
assert(m.output.includes('READY.'),m.output);
assert.equal(m.cpu.data[0x2A],238); // D4 START and D0 RX are inputs; remaining low bank stays output.
assert.equal(m.cpu.data[0x27]&3,0); // A0/A1 inputs.
assert.equal(m.buzzerEdges.filter(e=>e.on).length,0,'buzzer quiet before start');
m.send('j');
for(const [x,y,label] of [[3.190,2.709,'LEFT'],[2.240,2.709,'RIGHT'],[2.716,3.612,'UP'],[2.716,2.259,'DOWN']]){
 m.adc.channelValues[0]=x;m.adc.channelValues[1]=y;m.run(260);
 assert(m.output.trim().endsWith(label),m.output.slice(-300));
 m.adc.channelValues[0]=2.716;m.adc.channelValues[1]=2.709;m.run(220);
}
assert(!m.output.includes('PLAY.'),'joystick must not start a round');
assert.equal(m.buzzerEdges.filter(e=>e.on).length,0,'buzzer quiet before start');
m.send('j');m.send('c',1200);assert(m.output.includes('Press START or send s'),m.output.slice(-200));
m.send('m');const columns=[0,1,7,2,3,4,5];
for(let cell=0;cell<49;cell++){
 const expected=Array(14).fill(0);expected[(cell%7)*2]=1<<columns[Math.floor(cell/7)];
 assert.deepEqual(m.frames.at(-1).data,expected,'isolated bar '+cell);
 m.send('n',60);
}
assert(m.output.includes('BAR digit=7 from left, segment=g raw column=5'));
// Short contact bounce is ignored; held button produces only one start.
m.portD.setPin(4,false);m.run(10);m.portD.setPin(4,true);m.run(50);
assert(!m.output.includes('PLAY.'));
m.portD.setPin(4,false);m.run(50);
assert.equal((m.output.match(/PLAY\./g)||[]).length,1);
m.run(500);
m.portD.setPin(4,true);m.run(40);m.portD.setPin(4,false);m.run(40);
assert.equal((m.output.match(/PLAY\./g)||[]).length,1,'button must not reset live round');
m.run(2300);
assert(m.output.includes('DEAD score='),m.output);
const deathEnd=m.cpu.cycles/16000;
m.run(900);
const blink=m.frames.filter(f=>f.ms>deathEnd+40 && f.ms<deathEnd+850).map(f=>f.data);
const nonblank=blink.filter(f=>f.some(Boolean));assert(nonblank.length>0);
for(const f of nonblank)assert.deepEqual(f,nonblank[0],'body must stay frozen through blink');
assert(blink.some(f=>f.every(v=>v===0)),'death includes blank blink frames');
m.run(1000);assert(m.output.includes('GAME OVER score='));
m.run(2600);
const score = Number(m.output.match(/GAME OVER score=(\d+)/)[1]);
const masks = [0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F];
const scoreFrame = Array(14).fill(0);
for (const [digit, mask] of [[0,0x6D],[1,0x39],[2,0x3F],[3,0x50],[4,0x79],[6,masks[score % 10]], ...(score >= 10 ? [[5,masks[Math.floor(score/10)]]] : [])]) {
 for(let row=0;row<7;row++) if(mask & (1<<row)) scoreFrame[row*2] |= 1<<columns[digit];
}
assert.deepEqual(m.frames.at(-1).data, scoreFrame, 'SCORE and value share one frame');
assert.equal((m.output.match(/PLAY\./g)||[]).length,1,'held button must not restart after death');
m.portD.setPin(4,true);m.run(50);m.portD.setPin(4,false);m.run(50);
assert.equal((m.output.match(/PLAY\./g)||[]).length,2,'fresh press restarts');
const held=machine(process.argv[2],true,true);held.run(1500);
assert(!held.output.includes('PLAY.'),'held at boot must not start');
held.portD.setPin(4,true);held.run(50);held.portD.setPin(4,false);held.run(50);
assert(held.output.includes('PLAY.'),'release then press starts after boot');
m.send('s');assert.equal((m.output.match(/PLAY\./g)||[]).length,3,'serial start retained');
for(const f of m.frames)for(let row=0;row<7;row++){
 assert.equal(f.data[row*2+1],0,'no symbol columns 8/9');
 assert.equal(f.data[row*2]&64,0,'symbol column6 remains clear');
}
const pulses=[];
for(let i=0;i<m.buzzerEdges.length-1;i++){
 if(m.buzzerEdges[i].on && !m.buzzerEdges[i+1].on)pulses.push(m.buzzerEdges[i+1].ms-m.buzzerEdges[i].ms);
}
assert(pulses.some(ms=>ms>=38&&ms<55),'active buzzer start pulse then off');
assert(pulses.some(ms=>ms>=178&&ms<200),'active buzzer death pulse then off');
assert.equal(m.cpu.data[0x24]&8,8,'D11 remains output');
console.log('PASS: engine food/growth/walls/self/tail/win/turn queue; compiled ADC directions, calibration, all49 mapped bars, frozen death, game over, debounced START/held-at-boot/live-round protection, no joystick auto-start, serial restart, D11 active-buzzer pulses/off timing, numeric-only frames.');


const speed = machine(process.argv[2], true);
speed.run(1300);
speed.send('pace 2000\n', 20);
assert(speed.output.includes('PACE 2000 ms per move.'));
speed.send('s', 20);
const initial = speed.frames.at(-1).data;
speed.run(1000);
for (const cell of [14, 7, 0]) {
  const row = cell % 7 * 2, bit = 1 << columns[Math.floor(cell / 7)];
  assert(speed.frames.at(-1).data[row] & bit, 'slow pace preserves initial body before first move');
}
for (const invalid of ['pace 0', 'pace 2001', 'pace 99999999999999999999', 'pace 300x']) {
  speed.send(invalid + '\n', 20);
  assert(speed.output.trim().endsWith('Use pace 50..2000, then Enter. Example: pace 450'));
}
speed.send('pace 50\n', 20);
speed.run(700);
assert(speed.output.includes('DEAD score='), 'new fast pace takes effect during the round');
assert.equal((speed.output.match(/PLAY\./g) || []).length, 1, 'pace commands never trigger start');
console.log('PASS: pace range/overflow/trailing-text rejection and live movement timing.');
