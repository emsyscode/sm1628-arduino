// Compiled Uno gameplay test: ADC input, UART controls and decoded panel frames.
// No electrical/optical model of the real panel.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const avr=require(process.argv[4]?path.resolve(process.argv[4]):'avr8js');
function machine(file, panel=false) {
 const flash=new Uint8Array(32768);
 for(const line of fs.readFileSync(file,'utf8').trim().split(/\r?\n/)){
  const r=Buffer.from(line.slice(1),'hex');
  assert.equal([...r].reduce((a,b)=>a+b,0)%256,0);
  if(r[3]===0)flash.set(r.subarray(4,4+r[0]),r.readUInt16BE(1));
 }
 const cpu=new avr.CPU(new Uint16Array(flash.buffer));
 for(const c of [avr.timer0Config,avr.timer1Config,avr.timer2Config])new avr.AVRTimer(cpu,c);
 const b=new avr.AVRIOPort(cpu,avr.portBConfig);
 new avr.AVRIOPort(cpu,avr.portCConfig);new avr.AVRIOPort(cpu,avr.portDConfig);
 const uart=new avr.AVRUSART(cpu,avr.usart0Config,16e6);
 const adc=new avr.AVRADC(cpu,avr.adcConfig);adc.channelValues[0]=2.716;adc.channelValues[1]=2.709;
 const result={output:'',frames:[],cpu,adc,run,send};let bits=[],active=false;
 uart.onByteTransmit=v=>result.output+=String.fromCharCode(v);
 if(panel)b.addListener((v,o)=>{
  if((o&2)&&!(v&2)){bits=[];active=true;}
  if(!(v&2)&&!(o&1)&&(v&1)){if(!active){active=true;bits=[];}bits.push((cpu.data[0x2B]>>7)&1);}
  if(!(o&2)&&(v&2)&&active){
   assert.equal(bits.length%8,0);const bytes=[];
   for(let i=0;i<bits.length;i+=8)bytes.push(bits.slice(i,i+8).reduce((a,x,k)=>a|(x<<k),0));
   if(bytes[0]===192 && bytes.length===15)result.frames.push({ms:cpu.cycles/16000,data:bytes.slice(1)});
   active=false;
  }
 });
 function run(ms){const end=cpu.cycles+ms*16000;while(cpu.cycles<end){avr.avrInstruction(cpu);cpu.tick();}}
 function send(text,wait=100){for(const c of text+'\n'){assert(uart.writeByte(c.charCodeAt(0)));run(.15);}run(wait);}
 return result;
}
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
m.send('j');m.send('c',1200);assert(m.output.trim().endsWith('starts.'),m.output.slice(-200));
m.send('m');const columns=[0,1,7,2,3,4,5];
for(let cell=0;cell<49;cell++){
 const expected=Array(14).fill(0);expected[(cell%7)*2]=1<<columns[Math.floor(cell/7)];
 assert.deepEqual(m.frames.at(-1).data,expected,'isolated bar '+cell);
 m.send('n',60);
}
assert(m.output.includes('BAR digit=7 from left, segment=g raw column=5'));
m.send('s',100);
m.run(2300);
assert(m.output.includes('DEAD score='),m.output);
const deathEnd=m.cpu.cycles/16000;
m.run(900);
const blink=m.frames.filter(f=>f.ms>deathEnd+40 && f.ms<deathEnd+850).map(f=>f.data);
const nonblank=blink.filter(f=>f.some(Boolean));assert(nonblank.length>0);
for(const f of nonblank)assert.deepEqual(f,nonblank[0],'body must stay frozen through blink');
assert(blink.some(f=>f.every(v=>v===0)),'death includes blank blink frames');
m.run(1000);assert(m.output.includes('GAME OVER score='));
m.send('s');assert(m.output.includes('PLAY.'));
for(const f of m.frames)for(let row=0;row<7;row++){
 assert.equal(f.data[row*2+1],0,'no symbol columns 8/9');
 assert.equal(f.data[row*2]&64,0,'symbol column6 remains clear');
}
console.log('PASS: engine food/growth/walls/self/tail/win/turn queue; compiled ADC directions, calibration, all49 mapped bars, frozen death, game over, restart, numeric-only frames.');
