// Execute compiled Uno firmware; inject UART commands and decode actual GPIO writes.
const fs=require('node:fs'),path=require('node:path'),assert=require('node:assert/strict');
const avr=require(process.argv[3]?path.resolve(process.argv[3]):'avr8js');
const flash=new Uint8Array(32768);
for(const line of fs.readFileSync(process.argv[2],'utf8').trim().split(/\r?\n/)){
 const r=Buffer.from(line.slice(1),'hex'); if(r[3]===0)flash.set(r.subarray(4,4+r[0]),r.readUInt16BE(1));
}
const cpu=new avr.CPU(new Uint16Array(flash.buffer));
for(const c of [avr.timer0Config,avr.timer1Config,avr.timer2Config])new avr.AVRTimer(cpu,c);
const b=new avr.AVRIOPort(cpu,avr.portBConfig);
new avr.AVRIOPort(cpu,avr.portCConfig);new avr.AVRIOPort(cpu,avr.portDConfig);
const uart=new avr.AVRUSART(cpu,avr.usart0Config,16e6);
let output='',bits=[],active=false;const packets=[];
uart.onByteTransmit=v=>output+=String.fromCharCode(v);
b.addListener((v,o)=>{
 if((o&2)&&!(v&2)){bits=[];active=true;}
 if(!(v&2)&&!(o&1)&&(v&1)){if(!active){active=true;bits=[];}bits.push((cpu.data[0x2B]>>7)&1);}
 if(!(o&2)&&(v&2)&&active){
  assert.equal(bits.length%8,0);const bytes=[];
  for(let i=0;i<bits.length;i+=8)bytes.push(bits.slice(i,i+8).reduce((a,x,k)=>a|(x<<k),0));
  packets.push(bytes);active=false;
 }
});
function run(ms){const end=cpu.cycles+ms*16000;while(cpu.cycles<end){avr.avrInstruction(cpu);cpu.tick();}}
function command(text){for(const ch of text+'\n'){assert(uart.writeByte(ch.charCodeAt(0)));run(1.3);}run(200);}
run(2200);assert(output.includes('READY.'));assert.equal(packets.length,32);
assert.equal(cpu.data[0x2A],254,'only D0 restored to input');assert.equal(cpu.data[0x24],255);
const expectFrame=(cmd,frame)=>{
 const before=packets.length;command(cmd);
 assert.deepEqual(packets.slice(before),[[3],[128],[192,...frame],[143]],cmd);
};
let f=Array(14).fill(0);
expectFrame('column 0 8',f.map((_,i)=>i%2===0?1:0));
expectFrame('column 8 8',f.map((_,i)=>i%2===1?1:0));
f[13]=128;expectFrame('bit 13 7',f);
f=Array(14).fill(0);f[0]=1;expectFrame('next',f);
f=Array(14).fill(0);f[13]=128;expectFrame('prev',f);
f[0]=1;expectFrame('set 0 0 1',f);
f[13]=0;expectFrame('set 13 7 0',f);
expectFrame('show',f);
const before=packets.length;
for(const cmd of ['slot 8','column 16','bit 14 0','bit 0 8','set 0 0 2','slot -1','slot 0 8 extra','x'.repeat(70),'digit 2','next'])
 command(cmd);
assert.equal(packets.length,before,'invalid commands must not mutate display');
assert.equal((output.match(/ERR:/g)||[]).length,10);
expectFrame('clear',Array(14).fill(0));
command('frame');assert(output.includes('RAM 00..0D: '+Array(14).fill('00').join(' ')));
const held=packets.length;run(1000);assert.equal(packets.length,held,'no automatic stepping');
console.log('PASS: compiled Uno UART commands, full frames, bit stepping, invalid inputs, RX direction and indefinite hold.');
