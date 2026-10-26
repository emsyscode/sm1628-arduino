// Compiled Uno gameplay test: ADC input, UART controls and decoded panel frames.
// No electrical/optical model of the real panel.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const avr=require(process.argv[4]?path.resolve(process.argv[4]):'avr8js');
const machine=require('./avr-panel.cjs')(avr);
const checks=machine(process.argv[3]);checks.run(500);
assert(checks.output.includes('ENGINE PASS')&&!checks.output.includes('FAIL'),checks.output);
const m=machine(process.argv[2],true);m.run(1300);
assert(m.output.includes('READY.'),m.output);
assert.equal(m.cpu.data[0x2A],254); // Existing D1..D7 outputs; D0 serial input.
assert.equal(m.cpu.data[0x27]&3,0); // A0/A1 inputs.
m.send('j');
for(const [x,y,label] of [[3.190,2.709,'LEFT'],[2.240,2.709,'RIGHT'],[2.716,3.612,'UP'],[2.716,2.259,'DOWN']]){
 m.adc.channelValues[0]=x;m.adc.channelValues[1]=y;m.run(260);
 assert(m.output.trim().endsWith(label),m.output.slice(-300));
 m.adc.channelValues[0]=2.716;m.adc.channelValues[1]=2.709;m.run(220);
}
assert(!m.output.includes('PLAY.'),'joystick must not start a round');
m.send('j');m.send('c',1200);assert(m.output.includes('Press START or send s'),m.output.slice(-200));
m.send('m');const columns=[0,1,7,2,3,4,5];
for(let cell=0;cell<49;cell++){
 const expected=Array(14).fill(0);expected[(cell%7)*2]=1<<columns[Math.floor(cell/7)];
 assert.deepEqual(m.frames.at(-1).data,expected,'isolated bar '+cell);
 m.send('n',60);
}
assert(m.output.includes('BAR digit=7 from left, segment=g raw column=5'));
// Short contact bounce is ignored; held button produces only one start.
m.portC.setPin(2,false);m.run(10);m.portC.setPin(2,true);m.run(50);
assert(!m.output.includes('PLAY.'));
m.portC.setPin(2,false);m.run(50);
assert.equal((m.output.match(/PLAY\./g)||[]).length,1);
m.run(500);
m.portC.setPin(2,true);m.run(40);m.portC.setPin(2,false);m.run(40);
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
assert.equal((m.output.match(/PLAY\./g)||[]).length,1,'held button must not restart after death');
m.portC.setPin(2,true);m.run(50);m.portC.setPin(2,false);m.run(50);
assert.equal((m.output.match(/PLAY\./g)||[]).length,2,'fresh press restarts');
const held=machine(process.argv[2],true,true);held.run(1500);
assert(!held.output.includes('PLAY.'),'held at boot must not start');
held.portC.setPin(2,true);held.run(50);held.portC.setPin(2,false);held.run(50);
assert(held.output.includes('PLAY.'),'release then press starts after boot');
m.send('s');assert.equal((m.output.match(/PLAY\./g)||[]).length,3,'serial start retained');
for(const f of m.frames)for(let row=0;row<7;row++){
 assert.equal(f.data[row*2+1],0,'no symbol columns 8/9');
 assert.equal(f.data[row*2]&64,0,'symbol column6 remains clear');
}
console.log('PASS: engine food/growth/walls/self/tail/win/turn queue; compiled ADC directions, calibration, all49 mapped bars, frozen death, game over, debounced START/held-at-boot/live-round protection, no joystick auto-start, serial restart, numeric-only frames.');


