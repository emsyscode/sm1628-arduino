// Compare actual compiled output of the supplied sketch and the refactor.
// No replacement driver model: both .hex files execute their own AVR firmware.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const avr = require(process.argv[4] ? path.resolve(process.argv[4]) : 'avr8js');
const referenceFile = path.join(__dirname,
  '../reference/WorkingPanel/SM1628B_Panel_DVD_Reader_7digits_sanake_v0/SM1628B_Panel_DVD_Reader_7digits_sanake_v0.ino');
const raw = fs.readFileSync(referenceFile);
assert.equal(crypto.createHash('sha256').update(raw).digest('hex'),
  '4e9521dc761201b3c6feb347db51230398487458f13151ba866038d02293b9b4',
  'The user-supplied reference must remain byte-for-byte unchanged');
const source = raw.toString('utf8');
const patterns = Array.from({length:10}, (_,n) => {
  const block = source.match(new RegExp('unsigned int segNum'+n+'\\[\\]\\s*=\\s*\\{([\\s\\S]*?)\\};'))[1];
  const bytes = [...block.matchAll(/\(0b([01]+)\)/g)].map(m=>parseInt(m[1],2));
  assert.equal(bytes.length,14);
  return bytes;
});
const expected = [[3],[0x40],[0xC0],[0x8F]];
const addClear = () => { for(let a=0;a<14;a++)expected.push([0x40],[0xC0|a,0]); };
addClear();
for(let slot=0;slot<8;slot++)for(let n=0;n<10;n++)
  expected.push([3],[0x80],[0xC0,...patterns[n].map(b=>(b<<slot)&255)],[0x8F]);
addClear();
assert.equal(expected.length,380);

function simulate(hex) {
  const flash = new Uint8Array(32768); let base=0;
  for(const line of fs.readFileSync(hex,'utf8').trim().split(/\r?\n/)) {
    const r=Buffer.from(line.slice(1),'hex');
    assert.equal([...r].reduce((a,b)=>a+b,0)%256,0,'HEX checksum');
    if(r[3]===0)flash.set(r.subarray(4,4+r[0]),base+r.readUInt16BE(1));
    if(r[3]===4)base=r.readUInt16BE(4)*65536;
  }
  const cpu = new avr.CPU(new Uint16Array(flash.buffer));
  new avr.AVRTimer(cpu,avr.timer0Config);
  new avr.AVRTimer(cpu,avr.timer1Config);
  new avr.AVRTimer(cpu,avr.timer2Config);
  const b = new avr.AVRIOPort(cpu,avr.portBConfig);
  new avr.AVRIOPort(cpu,avr.portCConfig);
  new avr.AVRIOPort(cpu,avr.portDConfig);
  new avr.AVRUSART(cpu,avr.usart0Config,16e6);
  const packets=[]; let bits=[],active=false,start=0,firstRise=null,lastRise=null;
  b.addListener((value,old)=>{
    const stb=(value&2)!==0,clk=(value&1)!==0;
    if((old&2)&&!stb){bits=[];active=true;start=cpu.cycles;firstRise=null;}
    if(!stb && !(old&1) && clk){
      // The reference starts with STB already LOW, so its first transaction
      // has no falling edge. Capture it from its first rising clock edge.
      if(!active){active=true;bits=[];start=cpu.cycles;firstRise=null;}
      if(firstRise===null)firstRise=cpu.cycles;
      lastRise=cpu.cycles;
      bits.push((cpu.data[0x2B]&128)?1:0);
    }
    if(!(old&2)&&stb&&active){
      assert(bits.length>0 && bits.length%8===0,'incomplete STB transaction');
      const bytes=[];
      for(let i=0;i<bits.length;i+=8)
        bytes.push(bits.slice(i,i+8).reduce((v,bit,k)=>v|(bit<<k),0));
      packets.push({bytes,start,end:cpu.cycles,firstRise,lastRise,
        ddrB:cpu.data[0x24],ddrD:cpu.data[0x2A]});
      active=false;bits=[];
    }
  });
  while(packets.length<expected.length && cpu.cycles<16e6*12) {
    avr.avrInstruction(cpu);cpu.tick();
  }
  assert.equal(packets.length,expected.length,'reference/refactor did not complete one cycle');
  assert.deepEqual(packets.map(p=>p.bytes),expected);
  for(const p of packets){
    assert.equal(p.ddrB,255,'retained PORTB direction setup');
    assert.equal(p.ddrD,255,'retained PORTD direction setup');
  }
  // Every numeral is followed by the original 100 ms application hold.
  for(let numeral=0;numeral<80;numeral++){
    const enabled=packets[32+numeral*4+3];
    const following=packets[32+numeral*4+4];
    const holdMs=(following.start-enabled.end)/16000;
    assert(holdMs>=99 && holdMs<=102,'changed numeral hold: '+holdMs);
  }
  return {packets,ms:cpu.cycles/16000};
}
const original=simulate(process.argv[2]),clean=simulate(process.argv[3]);
assert.deepEqual(clean.packets.map(p=>p.bytes),original.packets.map(p=>p.bytes));
const packetDifferences = original.packets.map((p,i)=>
  Math.abs((p.end-p.firstRise)-(clean.packets[i].end-clean.packets[i].firstRise))/16);
console.log(JSON.stringify({
  result:'PASS',transactions_per_sketch:expected.length,numeral_slot_combinations:80,
  reference_ms:original.ms,refactor_ms:clean.ms,
  max_packet_clock_to_strobe_duration_difference_us:Math.max(...packetDifferences),
  checks:['original SHA-256','literal payloads including second bytes',
    'startup command sequence','all 80 numeral/slot combinations',
    'per-frame blanking and brightness','individual-address clear writes',
    'STB packet boundaries and LSB-first order','100 ms holds','whole-port direction state'],
  limit:'Compiled Uno GPIO comparison only. CPU timing may differ; no electrical/optical panel model or hardware retest.'
},null,2));
